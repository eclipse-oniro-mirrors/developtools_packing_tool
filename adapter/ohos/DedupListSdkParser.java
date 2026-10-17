/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ohos;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.parser.Feature;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

/**
 * Parser for the deduplist_sdk.json manifest: exposes dedupMinApiVersion, deviceType and the sha256
 * of every API version by name / abi / api, and matches an app so sha256.
 */
final class DedupListSdkParser {
    private static final String JSON_DEDUP_MIN_API_VERSION = "dedupMinApiVersion";
    private static final String JSON_DEVICE_TYPE = "deviceType";
    private static final String JSON_LIBRARIES = "libraries";
    private static final String JSON_NAME = "name";
    private static final String JSON_ABI = "abi";

    private static final int API_VERSION_PARTS = 3;
    private static final int COMPOSITE_API_VERSION_MIN = 10000000;

    private static final Log LOG = new Log(DedupListSdkParser.class.toString());

    private DedupApiVersion dedupMinApiVersion;
    private final Set<String> deviceTypes = new TreeSet<>();
    private final Map<String, List<DedupLibraryInfo>> libraryIndex = new TreeMap<>();
    private String errorMessage = "";

    /**
     * The app minAPIVersion packs two versions in one integer: the high part carries the SDK version and
     * the last three digits carry the api version number. Every api version is normalised to three
     * segments before comparing, where missing components are treated as 0.
     */
    static final class DedupApiVersion {
        private final List<Integer> parts;

        private DedupApiVersion(List<Integer> parts) {
            this.parts = parts;
        }

        boolean isValid() {
            return !parts.isEmpty();
        }

        private int compareTo(DedupApiVersion other) {
            int size = Math.max(parts.size(), other.parts.size());
            for (int index = 0; index < size; index++) {
                int left = index < parts.size() ? parts.get(index) : 0;
                int right = index < other.parts.size() ? other.parts.get(index) : 0;
                if (left != right) {
                    return left < right ? -1 : 1;
                }
            }
            return 0;
        }

        boolean isNotGreaterThan(DedupApiVersion other) {
            return compareTo(other) <= 0;
        }

        boolean isLowerThan(DedupApiVersion other) {
            return compareTo(other) < 0;
        }

        @Override
        public boolean equals(Object other) {
            if (this == other) {
                return true;
            }
            if (!(other instanceof DedupApiVersion)) {
                return false;
            }
            return compareTo((DedupApiVersion) other) == 0;
        }

        @Override
        public int hashCode() {
            return parts.hashCode();
        }

        // Parses a manifest version string; only the dotted three-component form is accepted
        static DedupApiVersion parse(String text) {
            if (text == null || text.isEmpty() || text.endsWith(".")) {
                return null;
            }
            String[] rawParts = text.split("\\.", -1);
            if (rawParts.length != API_VERSION_PARTS) {
                return null;
            }
            List<Integer> parts = new ArrayList<>();
            for (String rawPart : rawParts) {
                Integer value = parsePositiveInteger(rawPart);
                if (value == null) {
                    return null;
                }
                parts.add(value);
            }
            return new DedupApiVersion(parts);
        }

        // Normalises an integer api version to three segments: a composite app value keeps the SDK version
        // from its high part and a plain value becomes the major segment, so minAPIVersion and
        // dedupMinApiVersion compare the same value
        static DedupApiVersion fromApiVersion(int apiVersion) {
            List<Integer> parts = new ArrayList<>();
            if (apiVersion > 0) {
                if (apiVersion >= COMPOSITE_API_VERSION_MIN) {
                    // A composite app api version packs major / minor / patch in its high part
                    int sdkVersion = apiVersion / 1000;
                    parts.add(sdkVersion / 10000);
                    parts.add((sdkVersion / 100) % 100);
                    parts.add(sdkVersion % 100);
                } else {
                    // A plain api version is the major segment
                    parts.add(apiVersion);
                    parts.add(0);
                    parts.add(0);
                }
            }
            return new DedupApiVersion(parts);
        }

        private static Integer parsePositiveInteger(String text) {
            if (text == null || text.isEmpty()) {
                return null;
            }
            for (int index = 0; index < text.length(); index++) {
                if (!Character.isDigit(text.charAt(index))) {
                    return null;
                }
            }
            try {
                return Integer.parseInt(text);
            } catch (NumberFormatException exception) {
                return null;
            }
        }
    }

    // Compatibility group of one manifest API version: the system-preset so sha256 of that version
    private static final class DedupCompatGroup {
        private final DedupApiVersion apiVersion;
        private final String sha256;

        private DedupCompatGroup(DedupApiVersion apiVersion, String sha256) {
            this.apiVersion = apiVersion;
            this.sha256 = sha256;
        }
    }

    // A single SO library record, matching one element of the manifest libraries array
    private static final class DedupLibraryInfo {
        private final String name;
        private final String abi;
        private final List<DedupCompatGroup> compatGroups = new ArrayList<>();

        private DedupLibraryInfo(String name, String abi) {
            this.name = name;
            this.abi = abi;
        }
    }

    // Inter-app dedup match result: the compatibility group matched by the app so sha256
    static final class DedupSoMatch {
        private final String abi;
        private final String sha256;

        private DedupSoMatch(String abi, String sha256) {
            this.abi = abi;
            this.sha256 = sha256;
        }

        String getAbi() {
            return abi;
        }

        String getSha256() {
            return sha256;
        }
    }

    boolean parseFile(String path) {
        reset();
        if (path == null || path.isEmpty()) {
            return setError("deduplist sdk path is empty");
        }
        File file = new File(path);
        if (!file.isFile()) {
            return setError("deduplist sdk file does not exist: " + path);
        }
        String content;
        try {
            content = new String(Files.readAllBytes(file.toPath()), StandardCharsets.UTF_8);
        } catch (IOException exception) {
            return setError("deduplist sdk file is empty or unreadable: " + path);
        }
        if (content.isEmpty()) {
            return setError("deduplist sdk file is empty or unreadable: " + path);
        }
        return parseContent(content);
    }

    boolean parseContent(String content) {
        reset();
        if (content == null || content.isEmpty()) {
            return setError("deduplist sdk content is empty");
        }
        JSONObject root;
        try {
            root = JSON.parseObject(content, Feature.OrderedField);
        } catch (JSONException exception) {
            return setError("deduplist sdk json must be an object");
        }
        if (root == null) {
            return setError("deduplist sdk json must be an object");
        }
        Object minApiVersionValue = root.get(JSON_DEDUP_MIN_API_VERSION);
        if (!(minApiVersionValue instanceof Number)) {
            return setError("deduplist sdk json missing or invalid field dedupMinApiVersion");
        }
        long rawMinApiVersion = ((Number) minApiVersionValue).longValue();
        if (rawMinApiVersion <= 0 || rawMinApiVersion > Integer.MAX_VALUE) {
            return setError("deduplist sdk json missing or invalid field dedupMinApiVersion");
        }
        JSONArray deviceTypeArray = getJsonArray(root, JSON_DEVICE_TYPE);
        if (deviceTypeArray == null) {
            return setError("deduplist sdk field deviceType must be an array");
        }
        if (deviceTypeArray.isEmpty()) {
            return setError("deduplist sdk field deviceType must not be empty");
        }
        if (!parseDeviceTypes(deviceTypeArray)) {
            return false;
        }
        JSONArray librariesArray = getJsonArray(root, JSON_LIBRARIES);
        if (librariesArray == null) {
            return setError("deduplist sdk field libraries must be an array");
        }
        if (librariesArray.isEmpty()) {
            return setError("deduplist sdk field libraries must not be empty");
        }
        if (!parseLibraries(librariesArray)) {
            return false;
        }
        dedupMinApiVersion = DedupApiVersion.fromApiVersion((int) rawMinApiVersion);
        return true;
    }

    /**
     * Looks up the group matching an app side so: only groups with apiVersion <= compatibleSdkVersion
     * are considered (a lower one is inherited by higher ones) and the sha256 must equal appSha256.
     *
     * @param name so name
     * @param abi so abi
     * @param appSha256 sha256 of the app side so
     * @param compatibleSdkVersion compatible sdk version of the app
     * @return the matched compatibility group, or null when no match is found
     */
    DedupSoMatch findMatch(String name, String abi, String appSha256, DedupApiVersion compatibleSdkVersion) {
        if (compatibleSdkVersion == null || !compatibleSdkVersion.isValid()) {
            LOG.debug("So deduplication for sdk skip: compatibleSdkVersion is invalid, so=" + name);
            return null;
        }
        List<DedupLibraryInfo> libraries = libraryIndex.get(name);
        if (libraries == null) {
            LOG.debug("So deduplication for sdk skip: so is not listed in deduplist sdk, so=" + name);
            return null;
        }
        DedupLibraryInfo library = findLibrary(libraries, abi);
        if (library == null) {
            LOG.debug("So deduplication for sdk skip: so abi is not listed in deduplist sdk, so=" + name
                    + ", abi=" + abi);
            return null;
        }
        for (DedupCompatGroup group : library.compatGroups) {
            if (group.apiVersion.isNotGreaterThan(compatibleSdkVersion)
                    && group.sha256.equals(appSha256)) {
                return new DedupSoMatch(abi, group.sha256);
            }
        }
        LOG.debug("So deduplication for sdk skip: no compatibility group matched for so=" + name + ", abi=" + abi);
        return null;
    }

    DedupApiVersion getDedupMinApiVersion() {
        return dedupMinApiVersion;
    }

    Set<String> getDeviceTypes() {
        return deviceTypes;
    }

    String getErrorMessage() {
        return errorMessage;
    }

    private static JSONArray getJsonArray(JSONObject root, String key) {
        Object value = root.get(key);
        return value instanceof JSONArray ? (JSONArray) value : null;
    }

    private boolean parseDeviceTypes(JSONArray deviceTypeArray) {
        for (int index = 0; index < deviceTypeArray.size(); index++) {
            Object item = deviceTypeArray.get(index);
            if (!(item instanceof String) || ((String) item).isEmpty()) {
                return setError("deduplist sdk field deviceType must contain non-empty strings");
            }
            deviceTypes.add((String) item);
        }
        return true;
    }

    private boolean parseLibraries(JSONArray librariesArray) {
        for (int index = 0; index < librariesArray.size(); index++) {
            Object item = librariesArray.get(index);
            if (!(item instanceof JSONObject)) {
                return setError("deduplist sdk libraries element must be an object");
            }
            if (!parseLibrary((JSONObject) item)) {
                return false;
            }
        }
        return true;
    }

    private boolean parseLibrary(JSONObject libraryJson) {
        String name = getStringValue(libraryJson, JSON_NAME);
        if (name == null) {
            return setError("deduplist sdk library field name invalid or missing");
        }
        if (name.isEmpty()) {
            return setError("deduplist sdk library field name must not be empty");
        }
        String abi = getStringValue(libraryJson, JSON_ABI);
        if (abi == null) {
            return setError("deduplist sdk library field abi invalid or missing");
        }
        if (abi.isEmpty()) {
            return setError("deduplist sdk library field abi must not be empty");
        }
        DedupLibraryInfo library = new DedupLibraryInfo(name, abi);
        for (Map.Entry<String, Object> entry : libraryJson.entrySet()) {
            String key = entry.getKey();
            if (JSON_NAME.equals(key) || JSON_ABI.equals(key)) {
                continue;
            }
            DedupApiVersion apiVersion = DedupApiVersion.parse(key);
            if (apiVersion == null) {
                return setError("deduplist sdk library has invalid api version key: " + key);
            }
            String sha256 = parseCompatGroup(key, entry.getValue());
            if (sha256 == null) {
                return false;
            }
            library.compatGroups.add(new DedupCompatGroup(apiVersion, sha256));
        }
        if (library.compatGroups.isEmpty()) {
            return setError("deduplist sdk library " + name + " must define at least one compatibility group");
        }
        libraryIndex.computeIfAbsent(name, key -> new ArrayList<>()).add(library);
        return true;
    }

    private String parseCompatGroup(String apiKey, Object groupValue) {
        if (!(groupValue instanceof String) || ((String) groupValue).isEmpty()) {
            setError("deduplist sdk compatibility group " + apiKey + " must map a non-empty sha256 string");
            return null;
        }
        return (String) groupValue;
    }

    private static DedupLibraryInfo findLibrary(List<DedupLibraryInfo> libraries, String abi) {
        for (DedupLibraryInfo library : libraries) {
            if (library.abi.equals(abi)) {
                return library;
            }
        }
        return null;
    }

    private static String getStringValue(JSONObject object, String key) {
        Object value = object.get(key);
        return value instanceof String ? (String) value : null;
    }

    private void reset() {
        dedupMinApiVersion = null;
        deviceTypes.clear();
        libraryIndex.clear();
        errorMessage = "";
    }

    private boolean setError(String error) {
        errorMessage = error;
        return false;
    }
}
