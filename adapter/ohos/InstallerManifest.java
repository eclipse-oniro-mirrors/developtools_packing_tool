/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
import java.nio.charset.StandardCharsets;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Parses, validates, and generates installer package manifests.
 *
 * @since 2026-10-07
 */
final class InstallerManifest {
    private InstallerManifest() {}

    static final class Failure extends BundleException {
        private static final long serialVersionUID = 1L;

        final ErrorMsg error;

        Failure(ErrorMsg error, String message) {
            this(error, message, null);
        }

        Failure(ErrorMsg error, String message, Throwable cause) {
            super(message, cause);
            this.error = error;
        }
    }

    static JSONObject parse(String source) throws BundleException {
        Object result;
        try {
            result = JSON.parse(source, Feature.DisableSpecialKeyDetect);
        } catch (JSONException exception) {
            throw new Failure(PackingToolErrMsg.INSTALLER_MANIFEST_INVALID,
                    "Cannot parse manifest JSON", exception);
        }
        require(result instanceof JSONObject, "JSON root must be an object");
        return (JSONObject) result;
    }

    static void require(boolean condition, String message) throws BundleException {
        require(condition, PackingToolErrMsg.INSTALLER_MANIFEST_INVALID, message);
    }

    static void require(boolean condition, ErrorMsg error, String message) throws BundleException {
        if (!condition) {
            throw new Failure(error, message);
        }
    }

    static JSONObject requireObject(Object value, String field) throws BundleException {
        require(value instanceof JSONObject, field + " must be an object");
        return (JSONObject) value;
    }

    static String readRequiredString(JSONObject object, String field) throws BundleException {
        Object value = object.get(field);
        require(value instanceof String && !((String) value).isEmpty(), field + " must be a nonempty string");
        return (String) value;
    }

    static int readNonNegativeInt32(JSONObject object, String field) throws BundleException {
        Object value = object.get(field);
        require(value instanceof Number, field + " must be a nonnegative int32");
        double number = ((Number) value).doubleValue();
        require(Double.isFinite(number) && number >= 0 && number <= Integer.MAX_VALUE
                && Math.rint(number) == number, field + " must be a nonnegative int32");
        return (int) number;
    }

    static byte[] generate(List<JSONObject> apps) {
        JSONObject manifest = new JSONObject(true);
        manifest.put("version", 1);
        manifest.put("distributionType", "developer_id");
        JSONArray entries = new JSONArray();
        for (int index = 0; index < apps.size(); ++index) {
            JSONObject entry = new JSONObject(true);
            entry.putAll(apps.get(index));
            if (index == 0) {
                entry.put("coreApp", true);
            }
            entries.add(entry);
        }
        manifest.put("apps", entries);
        return JSON.toJSONString(manifest).getBytes(StandardCharsets.UTF_8);
    }

    static void validate(byte[] bytes, List<JSONObject> expected) throws BundleException {
        JSONObject manifest = parse(new String(bytes, StandardCharsets.UTF_8));
        Object value = manifest.get("apps");
        require(value instanceof JSONArray && ((JSONArray) value).size() == expected.size(),
                "apps must correspond one-to-one with application directories");
        Set<String> seen = new HashSet<>();
        int coreCount = 0;
        for (Object item : (JSONArray) value) {
            JSONObject app = requireObject(item, "apps item");
            String path = readRequiredString(app, "path");
            require(seen.add(path), "Duplicate application path");
            JSONObject match = findExpectedApp(expected, path);
            validateIdentity(app, match, path);
            require(readNonNegativeInt32(app, "minAPIVersion") >=
                    readNonNegativeInt32(match, "minAPIVersion"),
                    "Manifest minAPIVersion is below a module: " + path);
            if (hasCoreApp(app)) {
                ++coreCount;
            }
        }
        require(coreCount == 1, "Exactly one coreApp=true is required");
    }

    private static JSONObject findExpectedApp(List<JSONObject> expected, String path) throws BundleException {
        for (JSONObject candidate : expected) {
            if (path.equals(candidate.get("path"))) {
                return candidate;
            }
        }
        throw new Failure(PackingToolErrMsg.INSTALLER_MANIFEST_INVALID, "Unknown application path: " + path);
    }

    private static void validateIdentity(JSONObject app, JSONObject match, String path) throws BundleException {
        require(readRequiredString(app, "bundleName").equals(match.get("bundleName"))
                && readRequiredString(app, "versionName").equals(match.get("versionName"))
                && readNonNegativeInt32(app, "versionCode") == readNonNegativeInt32(match, "versionCode"),
                "Manifest application identity differs: " + path);
    }

    private static boolean hasCoreApp(JSONObject app) throws BundleException {
        if (!app.containsKey("coreApp")) {
            return false;
        }
        require(app.get("coreApp") instanceof Boolean, "coreApp must be boolean");
        return Boolean.TRUE.equals(app.get("coreApp"));
    }
}
