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
import com.alibaba.fastjson.JSONObject;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;
import java.util.TreeMap;
import java.util.TreeSet;
import java.text.SimpleDateFormat;

/**
 * Inter-app so deduplication solver: filters modules by eligibility, checks the version / driver
 * and device type gates and matches sha256 to produce the removal plan.
 */
final class InterAppDedupSolver {
    private static final String MODULE = "module";
    private static final String APP = "app";
    private static final String NAME = "name";
    private static final String DEVICE_TYPES = "deviceTypes";
    private static final String REQUIRED_DEVICE_FEATURES = "requiredDeviceFeatures";
    private static final String COMPRESS_NATIVE_LIBS = "compressNativeLibs";
    private static final String EXTRACT_NATIVE_LIBS = "extractNativeLibs";
    private static final String LIB_ISOLATION = "libIsolation";
    private static final String COMPILE_SDK_TYPE = "compileSdkType";
    private static final String EXTENSION_ABILITIES = "extensionAbilities";
    private static final String TYPE = "type";
    private static final String DRIVER = "driver";
    private static final String MIN_API_VERSION = "minAPIVersion";
    private static final String OPEN_HARMONY = "OpenHarmony";

    private static final String RESULT_FILE_VERSION = "1.0";
    private static final String RESULT_FILE_DESCRIPTION = "SO files removed during packaging";
    private static final String KEY_VERSION = "version";
    private static final String KEY_GENERATED_TIME = "generatedTime";
    private static final String KEY_DESCRIPTION = "description";
    private static final String KEY_REMOVED_SO_FILES = "removedSoFiles";
    private static final String KEY_ORIGINAL_PATH_IN_HAP = "originalPathInHap";
    private static final String KEY_ORIGINAL_SHA256 = "originalSha256";

    private static final Log LOG = new Log(InterAppDedupSolver.class.toString());

    InterAppDedupSolver() {
    }

    // Module level configuration used only for the inter-app eligibility check
    static final class ModuleContext {
        private boolean stageModel = false;
        private String moduleName = "";
        private final Set<String> deviceTypes = new HashSet<>();
        private boolean deviceTypesConfigured = false;
        private boolean compressNativeLibs = false;
        private boolean extractNativeLibs = false;
        private boolean libIsolation = false;
        private String compileSdkType = "";

        private boolean isValidForDedup() {
            if (!stageModel) {
                LOG.debug("So deduplication for sdk skip: module " + moduleName + " is not a Stage model");
                return false;
            }
            if (moduleName.isEmpty() || !deviceTypesConfigured) {
                LOG.debug("So deduplication for sdk skip: module " + moduleName
                        + " is not valid for dedup (moduleName/deviceTypes)");
                return false;
            }
            return true;
        }

        boolean isEligible() {
            if (!isValidForDedup()) {
                return false;
            }
            if (!compressNativeLibs && !extractNativeLibs) {
                LOG.debug("So deduplication for sdk skip: module " + moduleName + " does not extract native libs");
                return false;
            }
            if (libIsolation) {
                LOG.debug("So deduplication for sdk skip: module " + moduleName + " enables libIsolation");
                return false;
            }
            if (OPEN_HARMONY.equals(compileSdkType)) {
                LOG.debug("So deduplication for sdk skip: module " + moduleName + " compileSdkType is OpenHarmony");
                return false;
            }
            return true;
        }

        String getModuleName() {
            return moduleName;
        }
    }

    // Application level gate input (minAPIVersion, driver flag): aggregated over every module; the device
    // type gate is module level and uses ModuleContext.deviceTypes. The raw integer is normalised to three
    // segments on demand, so the gate and the compatibility groups compare it against the same value as
    // dedupMinApiVersion and the manifest keys.
    static final class GateContext {
        private int minApiVersion = 0;
        private boolean hasDriverExtensionAbility = false;

        boolean hasValidVersion() {
            return minApiVersion > 0;
        }

        // Normalised api version compared by the gate and the compatibility groups
        DedupListSdkParser.DedupApiVersion getApiVersion() {
            return DedupListSdkParser.DedupApiVersion.fromApiVersion(minApiVersion);
        }
    }

    // Single package packaging context parsed from module.json
    static final class PackContext {
        private boolean applicable = false;
        private final GateContext gateContext = new GateContext();
        private final ModuleContext moduleContext = new ModuleContext();

        boolean isApplicable() {
            return applicable;
        }

        GateContext getGateContext() {
            return gateContext;
        }

        ModuleContext getModuleContext() {
            return moduleContext;
        }
    }

    // A single candidate so (extracted from the module with sha256 computed)
    static final class SoCandidate {
        private final String soName;
        private final String abi;
        private final String relativePath;
        private final String sha256;

        SoCandidate(String soName, String abi, String relativePath, String sha256) {
            this.soName = soName;
            this.abi = abi;
            this.relativePath = relativePath;
            this.sha256 = sha256;
        }
    }

    // Solve input of a single module (with the configuration needed for module level eligibility)
    static final class ModuleCandidate {
        private final String moduleName;
        private final ModuleContext moduleContext;
        private DedupListSdkParser.DedupApiVersion matchApiVersion = null;
        private final List<SoCandidate> sos = new ArrayList<>();

        ModuleCandidate(String moduleName, ModuleContext moduleContext) {
            this.moduleName = moduleName;
            this.moduleContext = moduleContext;
        }

        void setMatchApiVersion(DedupListSdkParser.DedupApiVersion matchApiVersion) {
            this.matchApiVersion = matchApiVersion;
        }

        void addSo(SoCandidate candidate) {
            sos.add(candidate);
        }
    }

    // An so file to be removed by inter-app dedup (recorded in the result file)
    static final class RemovedSo {
        private final String soName;
        private final String relativePath;
        private final String sha256;

        RemovedSo(String soName, String relativePath, String sha256) {
            this.soName = soName;
            this.relativePath = relativePath;
            this.sha256 = sha256;
        }

        String getSoName() {
            return soName;
        }

        String getRelativePath() {
            return relativePath;
        }

        String getSha256() {
            return sha256;
        }
    }

    // Inter-app dedup plan
    static final class Plan {
        private boolean applied = false;
        private String skipReason = "";
        private final List<RemovedSo> removedSos = new ArrayList<>();

        boolean isApplied() {
            return applied;
        }

        String getSkipReason() {
            return skipReason;
        }

        List<RemovedSo> getRemovedSos() {
            return removedSos;
        }
    }

    /**
     * Builds the packaging context from an already parsed Stage module.json.
     *
     * @param root parsed module.json root object
     * @return the context, or null when a parse failure must abort packaging
     */
    static PackContext buildPackContext(JSONObject root) {
        PackContext context = new PackContext();
        context.applicable = true;
        JSONObject app = root == null ? null : root.getJSONObject(APP);
        JSONObject module = root == null ? null : root.getJSONObject(MODULE);
        buildGateContext(app, module, context.gateContext);
        buildModuleContext(app, module, context.moduleContext);
        return context;
    }

    private static void buildGateContext(JSONObject app, JSONObject module, GateContext gateContext) {
        if (app != null && app.containsKey(MIN_API_VERSION)) {
            int minApiVersion = app.getIntValue(MIN_API_VERSION);
            if (minApiVersion > 0) {
                gateContext.minApiVersion = minApiVersion;
            }
        }
        if (module != null) {
            gateContext.hasDriverExtensionAbility = hasDriverExtensionAbility(module);
        }
    }

    private static void buildModuleContext(JSONObject app, JSONObject module, ModuleContext context) {
        if (module == null) {
            return;
        }
        context.stageModel = true;
        context.moduleName = getString(module, NAME, "");
        context.compressNativeLibs = module.getBooleanValue(COMPRESS_NATIVE_LIBS);
        context.extractNativeLibs = !module.containsKey(EXTRACT_NATIVE_LIBS)
                || module.getBooleanValue(EXTRACT_NATIVE_LIBS);
        context.libIsolation = module.getBooleanValue(LIB_ISOLATION);
        context.compileSdkType = app == null ? "" : getString(app, COMPILE_SDK_TYPE, "");
        Set<String> declaredDeviceTypes = getDeviceTypes(module);
        Set<String> requiredDeviceFeatureTypes = getRequiredDeviceFeatureTypes(module);
        context.deviceTypes.addAll(declaredDeviceTypes);
        context.deviceTypes.addAll(requiredDeviceFeatureTypes);
        context.deviceTypesConfigured = !declaredDeviceTypes.isEmpty()
                || !requiredDeviceFeatureTypes.isEmpty();
    }

    private static Set<String> getDeviceTypes(JSONObject module) {
        Set<String> deviceTypes = new TreeSet<>();
        JSONArray deviceTypeArray = module.getJSONArray(DEVICE_TYPES);
        if (deviceTypeArray != null) {
            for (int index = 0; index < deviceTypeArray.size(); index++) {
                Object value = deviceTypeArray.get(index);
                if (value instanceof String) {
                    deviceTypes.add((String) value);
                }
            }
        }
        return deviceTypes;
    }

    private static Set<String> getRequiredDeviceFeatureTypes(JSONObject module) {
        Set<String> deviceTypes = new TreeSet<>();
        JSONObject features = module.getJSONObject(REQUIRED_DEVICE_FEATURES);
        if (features == null) {
            return deviceTypes;
        }
        for (Map.Entry<String, Object> entry : features.entrySet()) {
            if (entry.getValue() instanceof JSONArray && !((JSONArray) entry.getValue()).isEmpty()) {
                deviceTypes.add(entry.getKey());
            }
        }
        return deviceTypes;
    }

    private static boolean hasDriverExtensionAbility(JSONObject module) {
        JSONArray extensionAbilities = module.getJSONArray(EXTENSION_ABILITIES);
        if (extensionAbilities == null) {
            return false;
        }
        for (int index = 0; index < extensionAbilities.size(); index++) {
            Object item = extensionAbilities.get(index);
            if (!(item instanceof JSONObject)) {
                continue;
            }
            String extensionAbilityType = getString((JSONObject) item, TYPE, null);
            if (DRIVER.equals(extensionAbilityType)) {
                return true;
            }
        }
        return false;
    }

    private static String getString(JSONObject object, String key, String defaultValue) {
        Object value = object.get(key);
        return value instanceof String ? (String) value : defaultValue;
    }

    // Module level dedup eligibility check
    static boolean isModuleEligible(ModuleContext moduleContext) {
        return moduleContext != null && moduleContext.isEligible();
    }

    // Whether the declared module device types are a subset of the manifest device types
    static boolean isDeviceTypeSubset(Set<String> deviceTypes, Set<String> supportedDeviceTypes) {
        for (String deviceType : deviceTypes) {
            if (!supportedDeviceTypes.contains(deviceType)) {
                return false;
            }
        }
        return true;
    }

    // Version / driver gates; evaluated once on the aggregated context, a failure skips the package.
    // skipReason receives the gate reason when the check fails.
    boolean checkGates(GateContext context, DedupListSdkParser dedupList, StringBuilder skipReason) {
        if (context.hasDriverExtensionAbility) {
            skipReason.append("driver extensionAbility detected");
            return false;
        }
        if (!context.hasValidVersion()) {
            skipReason.append("compatibleSdkVersion is invalid");
            return false;
        }
        // Both sides are normalised to the same three segments before comparing; equal versions run dedup
        if (context.getApiVersion().isLowerThan(dedupList.getDedupMinApiVersion())) {
            skipReason.append("compatibleSdkVersion is lower than dedupMinApiVersion");
            return false;
        }
        return true;
    }

    // OR-es the driver flag and keeps the highest version; an unevaluable module version contributes nothing
    static void mergeGateContext(GateContext appGate, GateContext moduleGate) {
        appGate.hasDriverExtensionAbility = appGate.hasDriverExtensionAbility
                || moduleGate.hasDriverExtensionAbility;
        if (!appGate.hasValidVersion()
                || appGate.getApiVersion().isLowerThan(moduleGate.getApiVersion())) {
            appGate.minApiVersion = moduleGate.minApiVersion;
        }
    }

    // Checks the gates first, then evaluates eligibility and the device type gate per module; each
    // module matches compatibility groups with its own matchApiVersion
    Plan solve(GateContext context, List<ModuleCandidate> modules, DedupListSdkParser dedupList) {
        Plan plan = new Plan();
        StringBuilder skipReason = new StringBuilder();
        if (!checkGates(context, dedupList, skipReason)) {
            LOG.info("So deduplication for sdk skipped: " + skipReason);
            plan.skipReason = skipReason.toString();
            return plan;
        }
        plan.applied = true;
        for (ModuleCandidate module : modules) {
            if (!isModuleEligible(module.moduleContext)) {
                LOG.debug("Module " + module.moduleName + " is not eligible for inter-app deduplication");
                continue;
            }
            if (!isDeviceTypeSubset(module.moduleContext.deviceTypes, dedupList.getDeviceTypes())) {
                LOG.debug("Module " + module.moduleName
                        + " deviceTypes is not a subset of deduplist sdk deviceType");
                continue;
            }
            collectModuleRemovals(module, dedupList, plan);
        }
        return plan;
    }

    // Result file json content: removedSoFiles is an array ordered by package relative path, each
    // entry keeping the original path and sha256 of the removed so; empty when nothing is recorded
    static String generateResultJson(List<RemovedSo> removedSos) {
        if (removedSos == null || removedSos.isEmpty()) {
            return "";
        }
        Map<String, RemovedSo> orderedRemovals = new TreeMap<>();
        for (RemovedSo removed : removedSos) {
            orderedRemovals.put(removed.relativePath, removed);
        }
        JSONObject root = new JSONObject(true);
        root.put(KEY_VERSION, RESULT_FILE_VERSION);
        root.put(KEY_GENERATED_TIME, currentTimestamp());
        root.put(KEY_DESCRIPTION, RESULT_FILE_DESCRIPTION);
        JSONArray removedSoFiles = new JSONArray();
        for (RemovedSo removed : orderedRemovals.values()) {
            JSONObject item = new JSONObject(true);
            item.put(KEY_ORIGINAL_PATH_IN_HAP, removed.relativePath);
            item.put(KEY_ORIGINAL_SHA256, removed.sha256);
            removedSoFiles.add(item);
        }
        root.put(KEY_REMOVED_SO_FILES, removedSoFiles);
        return JSON.toJSONString(root);
    }

    private static String currentTimestamp() {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.ENGLISH);
        format.setTimeZone(TimeZone.getTimeZone("UTC"));
        return format.format(new Date());
    }

    private void collectModuleRemovals(ModuleCandidate module, DedupListSdkParser dedupList, Plan plan) {
        for (SoCandidate candidate : module.sos) {
            DedupListSdkParser.DedupSoMatch match = dedupList.findMatch(candidate.soName, candidate.abi,
                    candidate.sha256, module.matchApiVersion);
            if (match == null) {
                continue;
            }
            plan.removedSos.add(new RemovedSo(candidate.soName, candidate.relativePath,
                    match.getSha256()));
        }
    }
}
