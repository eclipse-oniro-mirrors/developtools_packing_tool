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
import com.alibaba.fastjson.serializer.SerializerFeature;
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry;
import org.apache.commons.compress.archivers.zip.ZipArchiveOutputStream;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.attribute.BasicFileAttributes;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;
import java.util.regex.Pattern;
import java.util.stream.Stream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

/**
 * SO deduplicator for Java packing tool.
 */
final class SODeduplicator {
    private static final String LIBS_DIR = "libs";
    private static final String MODULE_JSON = "module.json";
    private static final String CONFIG_JSON = "config.json";
    private static final String MODULE = "module";
    private static final String NAME = "name";
    private static final String TYPE = "type";
    private static final String DEVICE_TYPES = "deviceTypes";
    private static final String DELIVERY_WITH_INSTALL = "deliveryWithInstall";
    private static final String REQUIRED_DEVICE_FEATURES = "requiredDeviceFeatures";
    private static final String COMPRESS_NATIVE_LIBS = "compressNativeLibs";
    private static final String EXTRACT_NATIVE_LIBS = "extractNativeLibs";
    private static final String LIB_ISOLATION = "libIsolation";
    private static final String COMPILE_SDK_TYPE = "compileSdkType";
    private static final String BUNDLE_TYPE = "bundleType";
    private static final String ENTRY = "entry";
    private static final String OPEN_HARMONY = "OpenHarmony";
    private static final String APP = "app";
    private static final String ATOMIC_SERVICE = "atomicService";
    private static final String REPORT_NAME = "so_dedup_report.json";
    private static final String DEDUP_RESULT_FILE = "dedupfiles.json";
    private static final String LIBS_PREFIX = "libs/";
    private static final int EXACT_THRESHOLD = 20;
    private static final int BUFFER_SIZE = 8192;
    private static final Pattern SO_PATTERN = Pattern.compile(".*\\.so(\\.\\d+)*$", Pattern.CASE_INSENSITIVE);

    private static final Log LOG = new Log(SODeduplicator.class.toString());

    private SODeduplicator() {
    }

    static final class DedupSwitch {
        private final boolean deduplicateSo;
        private final String interAppDedupListPath;

        DedupSwitch(boolean deduplicateSo, String interAppDedupListPath) {
            this.deduplicateSo = deduplicateSo;
            this.interAppDedupListPath = interAppDedupListPath == null ? "" : interAppDedupListPath;
        }

        private boolean isInterAppEnabled() {
            return !interAppDedupListPath.isEmpty();
        }
    }

    private static void logDedupError(String cause) {
        LOG.error(PackingToolErrMsg.SO_DEDUPLICATION_FAILED.toString(cause));
    }

    private static String formatInterAppDedupError(String cause) {
        String message = PackingToolErrMsg.SO_DEDUPLICATION_FOR_SDK_FAILED.toString(cause);
        LOG.error(message);
        return message;
    }

    /**
     * Two stage intra-app and inter-app deduplication (unified repack).
     *
     * Stage one runs intra-app deduplication when the switch is on, stage two runs inter-app
     * deduplication per module when the deduplist path is set; both exclusion sets are merged at
     * repack so a given .so is removed only once. Either switch enables deduplication.
     *
     * @param modulePaths module package paths, replaced by the repacked packages
     * @param originalModulePaths module package paths as given by the caller
     * @param dedupSwitch deduplication switches and the inter-app deduplist path
     * @param workDir temporary working directory
     * @param reportDir directory receiving the deduplication report
     * @return the module package paths after deduplication
     * @throws BundleException when the module path count does not match or repacking fails
     */
    static List<String> deduplicateModules(List<String> modulePaths, List<String> originalModulePaths,
                                           DedupSwitch dedupSwitch, Path workDir, Path reportDir)
            throws BundleException {
        if (modulePaths == null) {
            return new ArrayList<>();
        }
        if (originalModulePaths == null || modulePaths.size() != originalModulePaths.size()) {
            throw new BundleException("Module path count does not match original module path count");
        }
        if (modulePaths.isEmpty()) {
            return modulePaths;
        }
        if (!dedupSwitch.deduplicateSo) {
            LOG.info("SO deduplication skipped: disabled.");
        }
        if (!dedupSwitch.isInterAppEnabled()) {
            LOG.info("SO deduplication for sdk skipped: disabled.");
        }
        if (!dedupSwitch.deduplicateSo && !dedupSwitch.isInterAppEnabled()) {
            return modulePaths;
        }
        try {
            Path modulesRoot = Files.createTempDirectory(workDir, "so_dedup_modules_");
            Path repackedRoot = Files.createTempDirectory(workDir, "so_dedup_repacked_");
            List<ModuleRecord> modules = extractModules(modulePaths, originalModulePaths, modulesRoot, repackedRoot);
            // The intra-app stage caches the sha256 computed together with the md5 in a single read
            Map<String, Map<String, String>> soSha256Cache = new HashMap<>();
            DedupResult result = new DedupResult();
            if (dedupSwitch.deduplicateSo) {
                result = deduplicateExtractedModules(modules, soSha256Cache, dedupSwitch.isInterAppEnabled());
                writeReport(result, reportDir);
            }
            Map<String, Set<String>> intraAppRemoved = collectRemovedPaths(result);
            InterAppMergeResult interApp = new InterAppMergeResult();
            if (dedupSwitch.isInterAppEnabled()) {
                interApp = mergeInterAppDedup(modules, dedupSwitch.interAppDedupListPath, soSha256Cache,
                        intraAppRemoved);
            }
            List<String> repacked = repackModules(modules, intraAppRemoved, interApp.excluded, interApp.resultJson);
            if (dedupSwitch.deduplicateSo) {
                String reportPath = reportDir == null ? "" : reportDir.resolve(REPORT_NAME).toString();
                LOG.warning("SO deduplication completed: removed=" + result.removedRecords.size()
                        + ", savedBytes=" + result.totalSavedSize + ", report=" + reportPath);
            }
            return repacked;
        } catch (IOException exception) {
            String errMsg = "SO deduplication error: " + exception.getMessage();
            logDedupError(errMsg);
            throw new BundleException(errMsg);
        }
    }

    private static List<ModuleRecord> extractModules(List<String> modulePaths, List<String> originalModulePaths,
            Path modulesRoot, Path repackedRoot)
            throws IOException, BundleException {
        List<ModuleRecord> modules = new ArrayList<>();
        int index = 0;
        for (String modulePath : modulePaths) {
            Path source = Paths.get(modulePath);
            String fileName = source.getFileName().toString();
            String moduleId = String.valueOf(index);
            Path extractDir = modulesRoot.resolve(moduleId);
            Path repackedPath = repackedRoot.resolve(moduleId).resolve(fileName);
            Files.createDirectories(extractDir);
            unzip(source, extractDir);
            ModuleConfig config = readModuleConfig(extractDir, source);
            modules.add(new ModuleRecord(
                    moduleId, modulePath, originalModulePaths.get(index), extractDir, repackedPath, config));
            index++;
        }
        return modules;
    }

    private static DedupResult deduplicateExtractedModules(List<ModuleRecord> modules,
            Map<String, Map<String, String>> soSha256Cache, boolean collectSha256) throws IOException {
        LOG.info("SO deduplication started.");
        List<ModuleRecord> eligibleModules = new ArrayList<>();
        List<ModuleRecord> validModules = new ArrayList<>();
        for (ModuleRecord module : modules) {
            if (module.config.isValidForDedup()) {
                validModules.add(module);
            }
            if (module.config.isEligible()) {
                eligibleModules.add(module);
            }
        }
        DedupResult result = new DedupResult();
        if (eligibleModules.isEmpty()) {
            return result;
        }
        Set<DeviceInstance> devices = calculateDevices(validModules);
        if (devices.isEmpty()) {
            LOG.info("SO deduplication skipped: no valid entry devices.");
            return result;
        }
        Map<DeviceInstance, Set<String>> mandatoryModules = calculateMandatoryModules(eligibleModules, devices);
        Map<String, ModuleConfig> moduleConfigs = new HashMap<>();
        for (ModuleRecord module : eligibleModules) {
            moduleConfigs.put(module.moduleId, module.config);
        }
        Map<String, List<SoRecord>> groups = collectDuplicateGroups(eligibleModules, soSha256Cache, collectSha256);
        for (List<SoRecord> group : groups.values()) {
            if (group.size() <= 1) {
                continue;
            }
            applyDedupForGroup(group, mandatoryModules, moduleConfigs, devices, result);
        }
        // The intra-app stage only solves; removal happens at the repack stage so that the
        // inter-app stage still sees every so file of the module.
        return result;
    }

    private static void applyDedupForGroup(List<SoRecord> group,
                                           Map<DeviceInstance, Set<String>> mandatoryModules,
                                           Map<String, ModuleConfig> moduleConfigs,
                                           Set<DeviceInstance> devices, DedupResult result) {
        String strategy = group.size() <= EXACT_THRESHOLD ? "exact" : "greedy";
        LOG.info("SO deduplication strategy: " + strategy
                + ", duplicateCopies=" + group.size() + ".");

        List<SoRecord> remaining;
        if ("exact".equals(strategy)) {
            remaining = findExactSolution(group, mandatoryModules, moduleConfigs, devices);
        } else {
            remaining = findGreedySolution(group, mandatoryModules, moduleConfigs, devices);
        }
        Set<SoRecord> kept = new HashSet<>(remaining);
        for (SoRecord so : group) {
            if (!kept.contains(so)) {
                result.addRemoved(so);
            }
        }
        for (SoRecord so : remaining) {
            result.addKept(so);
        }
    }

    private static List<SoRecord> findExactSolution(List<SoRecord> group,
                                                     Map<DeviceInstance, Set<String>> mandatoryModules,
                                                     Map<String, ModuleConfig> moduleConfigs,
                                                     Set<DeviceInstance> devices) {
        List<SoRecord> best = new ArrayList<>(group);
        long subsetCount = 1L << group.size();
        for (long mask = 1; mask < subsetCount; mask++) {
            int keptCount = Long.bitCount(mask);
            if (keptCount > best.size()) {
                continue;
            }
            List<SoRecord> candidate = new ArrayList<>();
            for (int i = 0; i < group.size(); i++) {
                if ((mask & (1L << i)) != 0) {
                    candidate.add(group.get(i));
                }
            }
            if (!satisfiesAllDevices(group, candidate, mandatoryModules, moduleConfigs, devices)) {
                continue;
            }
            if (candidate.size() < best.size()) {
                best = candidate;
            }
        }
        return best;
    }

    private static List<SoRecord> findGreedySolution(List<SoRecord> group,
                                                      Map<DeviceInstance, Set<String>> mandatoryModules,
                                                      Map<String, ModuleConfig> moduleConfigs,
                                                      Set<DeviceInstance> devices) {
        List<SoRecord> remaining = new ArrayList<>(group);
        for (SoRecord so : group) {
            List<SoRecord> trial = new ArrayList<>(remaining);
            trial.remove(so);
            if (satisfiesAllDevices(group, trial, mandatoryModules, moduleConfigs, devices)) {
                remaining = trial;
            }
        }
        return remaining;
    }

    private static boolean satisfiesAllDevices(List<SoRecord> original, List<SoRecord> remaining,
                                                Map<DeviceInstance, Set<String>> mandatoryModules,
                                                Map<String, ModuleConfig> moduleConfigs,
                                                Set<DeviceInstance> devices) {
        if (remaining.isEmpty()) {
            return false;
        }
        Set<String> mandatoryUnion = new HashSet<>();
        for (Map.Entry<DeviceInstance, Set<String>> entry : mandatoryModules.entrySet()) {
            Set<String> modules = entry.getValue();
            mandatoryUnion.addAll(modules);
            if (containsSoFromModules(original, modules) && !containsSoFromModules(remaining, modules)) {
                return false;
            }
        }
        for (SoRecord so : original) {
            if (remaining.contains(so) || mandatoryUnion.contains(so.moduleId)) {
                continue;
            }
            ModuleConfig config = moduleConfigs.get(so.moduleId);
            if (config == null) {
                return false;
            }
            for (DeviceInstance device : devices) {
                if (supportsDevice(config, device)
                        && !containsSoFromModules(remaining, mandatoryModules.get(device))) {
                    return false;
                }
            }
        }
        return true;
    }

    private static boolean containsSoFromModules(List<SoRecord> records, Set<String> modules) {
        if (modules == null || modules.isEmpty()) {
            return false;
        }
        for (SoRecord so : records) {
            if (modules.contains(so.moduleId)) {
                return true;
            }
        }
        return false;
    }

    private static Set<DeviceInstance> calculateDevices(List<ModuleRecord> modules) {
        Set<DeviceInstance> devices = new HashSet<>();
        for (ModuleRecord module : modules) {
            if (ENTRY.equals(module.config.moduleType)) {
                for (String deviceType : module.config.deviceTypes) {
                    devices.add(new DeviceInstance(deviceType, module.config.distributionFilter));
                }
            }
        }
        return devices;
    }

    private static Map<DeviceInstance, Set<String>> calculateMandatoryModules(
            List<ModuleRecord> modules, Set<DeviceInstance> devices) {
        Map<DeviceInstance, Set<String>> mandatoryModules = new HashMap<>();
        for (DeviceInstance device : devices) {
            Set<String> moduleIds = new HashSet<>();
            for (ModuleRecord module : modules) {
                ModuleConfig config = module.config;
                boolean isEntry = ENTRY.equals(config.moduleType);
                boolean isMandatory = isEntry ? supportsDevice(config, device)
                        : supportsDevice(config, device) && config.deliveryWithInstall
                        && config.requiredDeviceFeatures.isEmpty();
                if (isMandatory) {
                    moduleIds.add(module.moduleId);
                }
            }
            mandatoryModules.put(device, moduleIds);
        }
        return mandatoryModules;
    }

    private static boolean supportsDevice(ModuleConfig config, DeviceInstance device) {
        if (!config.deviceTypes.contains(device.type)) {
            return false;
        }
        return config.distributionFilter.isEmpty() || config.distributionFilter.equals(device.distributionFilter);
    }

    private static Map<String, List<SoRecord>> collectDuplicateGroups(List<ModuleRecord> modules,
            Map<String, Map<String, String>> soSha256Cache, boolean collectSha256) throws IOException {
        Map<String, List<SoRecord>> groups = new HashMap<>();
        for (ModuleRecord module : modules) {
            Path libsPath = module.extractDir.resolve(LIBS_DIR);
            if (!Files.exists(libsPath)) {
                continue;
            }
            collectSoFiles(groups, module, libsPath, soSha256Cache, collectSha256);
        }
        return groups;
    }

    private static void collectSoFiles(final Map<String, List<SoRecord>> groups, final ModuleRecord module,
                                       Path libsPath, Map<String, Map<String, String>> soSha256Cache,
                                       boolean collectSha256) throws IOException {
        Files.walkFileTree(libsPath, new SimpleFileVisitor<Path>() {
            @Override
            public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                if (isSoFile(file.getFileName().toString())) {
                    addSoRecord(groups, module, file, soSha256Cache, collectSha256);
                }
                return FileVisitResult.CONTINUE;
            }
        });
    }

    private static void addSoRecord(Map<String, List<SoRecord>> groups, ModuleRecord module, Path path,
                                    Map<String, Map<String, String>> soSha256Cache, boolean collectSha256)
            throws IOException {
        // Both digests are computed from a single read so the inter-app stage does not read the file again
        SoDigests digests = calculateDigests(path, collectSha256);
        Path relative = module.extractDir.relativize(path);
        String relativePath = relative.toString().replace('\\', '/');
        SoRecord so = new SoRecord(module.moduleId, module.config.moduleName, module.reportPath,
                relativePath, path, Files.size(path));
        groups.computeIfAbsent(so.relativePath + '\n' + digests.md5, key -> new ArrayList<>()).add(so);
        if (digests.sha256 != null) {
            soSha256Cache.computeIfAbsent(module.moduleId, key -> new HashMap<>()).put(relativePath, digests.sha256);
        }
    }

    private static boolean isSoFile(String fileName) {
        return SO_PATTERN.matcher(fileName).matches();
    }

    private static ModuleConfig readModuleConfig(Path moduleDir, Path source) throws IOException, BundleException {
        Path moduleJson = moduleDir.resolve(MODULE_JSON);
        if (Files.exists(moduleJson)) {
            return readStageModuleConfig(moduleJson, source);
        }
        Path configJson = moduleDir.resolve(CONFIG_JSON);
        if (Files.exists(configJson)) {
            return new ModuleConfig();
        }
        LOG.warning("SO deduplication skipped for module: failed to parse configuration, module=" + moduleDir);
        return new ModuleConfig();
    }

    private static ModuleConfig readStageModuleConfig(Path moduleJson, Path source)
            throws IOException, BundleException {
        JSONObject root;
        String profile;
        try {
            profile = readUtf8(moduleJson);
            root = JSON.parseObject(
                    new ByteArrayInputStream(profile.getBytes(StandardCharsets.UTF_8)), JSONObject.class);
        } catch (IOException | JSONException exception) {
            LOG.warning("SO deduplication skipped for module: failed to parse configuration, module="
                    + moduleJson.getParent());
            return new ModuleConfig();
        }
        JSONObject app = root == null ? null : root.getJSONObject("app");
        JSONObject module = root == null ? null : root.getJSONObject(MODULE);
        ModuleConfig config = new ModuleConfig();
        config.packContext = root == null ? null : InterAppDedupSolver.buildPackContext(root);
        if (module == null) {
            return config;
        }
        config.stageModel = true;
        config.moduleName = getString(module, NAME, "");
        config.moduleType = getString(module, TYPE, "");
        config.deviceTypes = getDeviceTypes(module.getJSONArray(DEVICE_TYPES));
        config.deliveryWithInstallPresent = module.containsKey(DELIVERY_WITH_INSTALL);
        config.deliveryWithInstall = module.getBooleanValue(DELIVERY_WITH_INSTALL);
        try (ZipFile zipFile = new ZipFile(source.toFile())) {
            HashMap<String, String> resourceMap = FileUtils.getProfileJson(zipFile);
            config.distributionFilter = parseDistributionFilter(profile, resourceMap);
        }
        parseRequiredDeviceFeatures(module.getJSONObject(REQUIRED_DEVICE_FEATURES), config);
        config.deviceTypesConfigured = !config.deviceTypes.isEmpty();
        config.compressNativeLibs = module.getBooleanValue(COMPRESS_NATIVE_LIBS);
        config.extractNativeLibs = !module.containsKey(EXTRACT_NATIVE_LIBS)
                || module.getBooleanValue(EXTRACT_NATIVE_LIBS);
        config.libIsolation = module.getBooleanValue(LIB_ISOLATION);
        config.compileSdkType = app == null ? "" : getString(app, COMPILE_SDK_TYPE, "");
        config.bundleType = app == null ? "" : getString(app, BUNDLE_TYPE, APP);
        return config;
    }

    private static String parseDistributionFilter(String profile, HashMap<String, String> resourceMap)
            throws BundleException {
        List<ModuleMetadataInfo> metadata = ModuleJsonUtil.parseModuleAllMetadata(profile, resourceMap);
        return ModuleJsonUtil.parseStageDistroFilter(metadata);
    }

    private static void parseRequiredDeviceFeatures(JSONObject features, ModuleConfig config) {
        if (features == null) {
            return;
        }
        for (String deviceType : features.keySet()) {
            JSONArray featureValues = features.getJSONArray(deviceType);
            if (featureValues == null || featureValues.isEmpty()) {
                continue;
            }
            String normalized = normalizeDeviceType(deviceType);
            config.requiredDeviceFeatures.add(normalized);
            if (!config.deviceTypes.contains(normalized)) {
                config.deviceTypes.add(normalized);
            }
        }
    }

    private static List<String> repackModules(List<ModuleRecord> modules, Map<String, Set<String>> intraAppRemoved,
            Map<String, Set<String>> interAppExcluded, Map<String, String> interAppResultJson)
            throws IOException {
        List<String> repacked = new ArrayList<>();
        for (ModuleRecord module : modules) {
            // Intra-app and inter-app removals are merged, so each .so is removed only once
            Set<String> removedPaths = new HashSet<>();
            Set<String> intraAppPaths = intraAppRemoved.get(module.moduleId);
            if (intraAppPaths != null) {
                removedPaths.addAll(intraAppPaths);
            }
            Set<String> interAppPaths = interAppExcluded.get(module.moduleId);
            if (interAppPaths != null) {
                removedPaths.addAll(interAppPaths);
            }
            copyEntriesWithExclusions(module, removedPaths, interAppResultJson.get(module.moduleId));
            repacked.add(module.repackedPath.toString());
        }
        return repacked;
    }

    private static void copyEntriesWithExclusions(ModuleRecord module, Set<String> removedPaths, String resultJson)
            throws IOException {
        Files.createDirectories(module.repackedPath.getParent());
        try (org.apache.commons.compress.archivers.zip.ZipFile source =
                new org.apache.commons.compress.archivers.zip.ZipFile(new File(module.sourcePath));
             ZipArchiveOutputStream target = new ZipArchiveOutputStream(module.repackedPath.toFile())) {
            Enumeration<ZipArchiveEntry> entries = source.getEntries();
            while (entries.hasMoreElements()) {
                ZipArchiveEntry entry = entries.nextElement();
                String entryName = entry.getName().replace('\\', '/');
                if (removedPaths.contains(entryName)) {
                    continue;
                }
                try (InputStream rawInput = source.getRawInputStream(entry)) {
                    target.addRawArchiveEntry(new ZipArchiveEntry(entry), rawInput);
                }
            }
            if (resultJson != null && !resultJson.isEmpty()) {
                writeStringEntry(target, DEDUP_RESULT_FILE, resultJson);
            }
        }
    }

    private static void writeStringEntry(ZipArchiveOutputStream target, String entryName, String content)
            throws IOException {
        ZipArchiveEntry entry = new ZipArchiveEntry(entryName);
        target.putArchiveEntry(entry);
        target.write(content.getBytes(StandardCharsets.UTF_8));
        target.closeArchiveEntry();
    }

    private static DedupListSdkParser parseDedupList(String interAppDedupListPath) throws BundleException {
        DedupListSdkParser dedupList = new DedupListSdkParser();
        if (!dedupList.parseFile(interAppDedupListPath)) {
            String errMsg = "failed to parse deduplist sdk: " + dedupList.getErrorMessage();
            throw new BundleException(formatInterAppDedupError(errMsg));
        }
        return dedupList;
    }

    /**
     * Runs inter-app deduplication for the whole package and merges the results into the
     * exclusion set.
     *
     * Application level gates (driver / version) are aggregated over all modules and evaluated
     * once; when a gate fails the whole package skips inter-app deduplication. The device type gate
     * is evaluated per module while solving, and each module matches compatibility groups with its
     * own compatibleSdkVersion.
     *
     * @param modules extracted module records; only modules with a parseable
     *                configuration (non-FA models) participate in deduplication
     * @param interAppDedupListPath path of the inter-app deduplication list (deduplist sdk)
     * @param soSha256Cache map from module ID to the "SO relative path -> sha256" cache,
     *                      shared with the intra-app deduplication stage
     * @param intraAppRemoved map from module ID to the set of SO relative paths already
     *                        removed by intra-app deduplication
     * @return the merged deduplication result, containing the excluded SO paths and the per-module
     *         deduplication result JSON; an empty result is returned when the gate check fails
     * @throws BundleException if the inter-app deduplication list cannot be parsed
     */
    private static InterAppMergeResult mergeInterAppDedup(List<ModuleRecord> modules,
            String interAppDedupListPath, Map<String, Map<String, String>> soSha256Cache,
            Map<String, Set<String>> intraAppRemoved) throws BundleException {
        LOG.info("SO deduplication for sdk started.");
        InterAppDedupSolver.GateContext appGateContext = new InterAppDedupSolver.GateContext();
        List<ModuleRecord> participatingModules = new ArrayList<>();
        for (ModuleRecord module : modules) {
            InterAppDedupSolver.PackContext packContext = module.config.packContext;
            // FA models or modules whose configuration failed to parse do not participate
            if (packContext == null || !packContext.isApplicable()) {
                continue;
            }
            InterAppDedupSolver.mergeGateContext(appGateContext, packContext.getGateContext());
            participatingModules.add(module);
        }
        InterAppDedupSolver solver = new InterAppDedupSolver();
        StringBuilder skipReason = new StringBuilder();
        DedupListSdkParser dedupList = parseDedupList(interAppDedupListPath);
        if (!solver.checkGates(appGateContext, dedupList, skipReason)) {
            LOG.info("So deduplication for sdk skipped: " + skipReason);
            return new InterAppMergeResult();
        }
        InterAppMergeResult mergeResult = new InterAppMergeResult();
        InterAppMergeContext mergeContext = new InterAppMergeContext(solver, dedupList, soSha256Cache,
                intraAppRemoved, mergeResult);
        int matchedModules = 0;
        for (ModuleRecord module : participatingModules) {
            if (mergeModulePlan(module, mergeContext)) {
                matchedModules++;
            }
        }
        LOG.info("Inter-app deduplication merged for " + matchedModules + " module(s).");
        return mergeResult;
    }

    private static boolean mergeModulePlan(ModuleRecord module, InterAppMergeContext mergeContext)
            throws BundleException {
        // A hap/hsp that already carries the dedup result file was deduplicated before: keep it as is
        if (hasDedupResultFile(module.sourcePath)) {
            LOG.info("So deduplication for sdk skipped for module " + module.sourcePath
                    + ": " + DEDUP_RESULT_FILE + " already exists");
            return false;
        }
        InterAppDedupSolver.PackContext packContext = module.config.packContext;
        InterAppDedupSolver.ModuleCandidate candidate = collectModuleCandidate(
                module.extractDir.resolve(LIBS_DIR).toString(), packContext,
                mergeContext.soSha256Cache.get(module.moduleId), mergeContext.intraAppRemoved.get(module.moduleId));
        InterAppDedupSolver.Plan plan = mergeContext.solver.solve(packContext.getGateContext(),
                Collections.singletonList(candidate), mergeContext.dedupList);
        if (!plan.isApplied() || plan.getRemovedSos().isEmpty()) {
            return false;
        }
        Set<String> excluded = mergeContext.mergeResult.excluded.computeIfAbsent(module.moduleId,
                key -> new LinkedHashSet<>());
        for (InterAppDedupSolver.RemovedSo removed : plan.getRemovedSos()) {
            excluded.add(removed.getRelativePath());
        }
        String resultJson = InterAppDedupSolver.generateResultJson(plan.getRemovedSos());
        if (resultJson == null || resultJson.isEmpty()) {
            throw new BundleException(formatInterAppDedupError(
                "Inter-app deduplication failed for module: " + module.reportPath
                + ", failed to generate inter-app dedup result json"));
        }
        mergeContext.mergeResult.resultJson.put(module.moduleId, resultJson);
        return true;
    }

    /**
     * Module instance id -> package relative paths already removed by the intra-app stage; the inter-app
     * stage skips them so nothing is processed or recorded twice.
     *
     * @param result intra-app deduplication result carrying the removed so records
     * @return removed package relative paths grouped by module instance id
     */
    private static Map<String, Set<String>> collectRemovedPaths(DedupResult result) {
        Map<String, Set<String>> removedByIntraApp = new HashMap<>();
        for (SoRecord removed : result.removedRecords) {
            removedByIntraApp.computeIfAbsent(removed.moduleId, key -> new HashSet<>()).add(removed.relativePath);
        }
        return removedByIntraApp;
    }

    private static boolean hasDedupResultFile(String packagePath) {
        if (packagePath == null || packagePath.isEmpty() || !new File(packagePath).isFile()) {
            return false;
        }
        try (org.apache.commons.compress.archivers.zip.ZipFile source =
                new org.apache.commons.compress.archivers.zip.ZipFile(new File(packagePath))) {
            return source.getEntry(DEDUP_RESULT_FILE) != null;
        } catch (IOException exception) {
            LOG.warning("Failed to inspect " + packagePath + " for " + DEDUP_RESULT_FILE + ": "
                    + exception.getMessage());
            return false;
        }
    }

    private static InterAppDedupSolver.ModuleCandidate collectModuleCandidate(String libDir,
            InterAppDedupSolver.PackContext packContext, Map<String, String> cachedSha256,
            Set<String> removedByIntraApp) {
        InterAppDedupSolver.ModuleContext moduleContext = packContext.getModuleContext();
        InterAppDedupSolver.GateContext gateContext = packContext.getGateContext();
        String moduleName = moduleContext.getModuleName();
        InterAppDedupSolver.ModuleCandidate candidate = new InterAppDedupSolver.ModuleCandidate(
                moduleName.isEmpty() ? libDir : moduleName, moduleContext);
        candidate.setMatchApiVersion(gateContext.getApiVersion());
        Path libPath = Paths.get(libDir);
        if (!Files.isDirectory(libPath)) {
            return candidate;
        }
        File[] abiDirs = libPath.toFile().listFiles(File::isDirectory);
        if (abiDirs == null) {
            return candidate;
        }
        for (File abiDir : abiDirs) {
            collectSoCandidates(abiDir, abiDir.getName(), cachedSha256, removedByIntraApp, candidate);
        }
        return candidate;
    }

    /**
     * Collects the so candidates of one abi directory. The so may sit under any sub directory of
     * libs/&lt;abi&gt;; the complete package relative path is kept so the result file records where the
     * removed so originally was.
     *
     * @param abiDir directory of one abi under the libs directory
     * @param abi name of the abi, used to build the package relative path of each so
     * @param cachedSha256 map from so relative path to its sha256; may be null, in which case
     *                     the digest is computed from the file
     * @param removedByIntraApp set of so relative paths already removed by intra-app deduplication;
     *                          may be null, such entries are skipped and never hashed or recorded
     * @param candidate module candidate the collected so entries are added to
     */
    private static void collectSoCandidates(File abiDir, String abi, Map<String, String> cachedSha256,
            Set<String> removedByIntraApp, InterAppDedupSolver.ModuleCandidate candidate) {
        Path abiPath = abiDir.toPath();
        List<Path> soPaths = new ArrayList<>();
        try (Stream<Path> stream = Files.walk(abiPath)) {
            stream.filter(Files::isRegularFile).forEach(soPaths::add);
        } catch (IOException exception) {
            LOG.warning("Failed to walk lib directory " + abiPath + ": " + exception.getMessage());
            return;
        }
        for (Path soPath : soPaths) {
            String soName = soPath.getFileName().toString();
            if (!isSoFile(soName)) {
                continue;
            }
            String subPath = abiPath.relativize(soPath).toString().replace('\\', '/');
            String relativePath = LIBS_PREFIX + abi + "/" + subPath;
            // Already removed by the intra-app stage: skip it so it is not hashed, matched or recorded again
            if (removedByIntraApp != null && removedByIntraApp.contains(relativePath)) {
                continue;
            }
            String sha256 = cachedSha256 == null ? null : cachedSha256.get(relativePath);
            if (sha256 == null || sha256.isEmpty()) {
                // Read the file only when the intra-app stage did not cache its sha256
                sha256 = FileUtils.getSha256(soPath.toString());
            }
            if (sha256.isEmpty()) {
                LOG.warning("Failed to calculate sha256 for " + soPath);
                continue;
            }
            candidate.addSo(new InterAppDedupSolver.SoCandidate(soName, abi, relativePath, sha256));
        }
    }

    private static void writeReport(DedupResult result, Path reportDir) throws IOException {
        if (reportDir == null) {
            throw new IOException("SO deduplication report directory is empty");
        }
        Files.createDirectories(reportDir);
        JSONObject report = new JSONObject(true);
        report.put("timestamp", timestamp());
        report.put("totalSavedSize", result.totalSavedSize);
        JSONObject modules = new JSONObject(true);
        List<Map.Entry<String, ModuleReport>> moduleEntries = new ArrayList<>(result.modules.entrySet());
        moduleEntries.sort((left, right) -> Integer.compare(
                Integer.parseInt(left.getKey()), Integer.parseInt(right.getKey())));
        for (Map.Entry<String, ModuleReport> entry : moduleEntries) {
            JSONObject module = new JSONObject(true);
            module.put("moduleName", entry.getValue().moduleName);
            module.put("filePath", entry.getValue().filePath);
            module.put("kept", entry.getValue().kept);
            module.put("removed", entry.getValue().removed);
            modules.put(entry.getKey(), module);
        }
        report.put("modules", modules);
        Files.write(reportDir.resolve(REPORT_NAME),
                JSON.toJSONString(report, SerializerFeature.PrettyFormat).getBytes(StandardCharsets.UTF_8));
    }

    private static void unzip(Path source, Path dest) throws IOException {
        try (ZipFile zipFile = new ZipFile(source.toFile())) {
            Enumeration<? extends ZipEntry> entries = zipFile.entries();
            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();
                Path out = dest.resolve(entry.getName()).normalize();
                if (!out.startsWith(dest)) {
                    throw new IOException("Invalid zip entry path: " + entry.getName());
                }
                if (entry.isDirectory()) {
                    Files.createDirectories(out);
                    continue;
                }
                Files.createDirectories(out.getParent());
                try (InputStream input = new BufferedInputStream(zipFile.getInputStream(entry));
                     OutputStream output = new BufferedOutputStream(Files.newOutputStream(out))) {
                    copy(input, output);
                }
            }
        }
    }

    private static SoDigests calculateDigests(Path path, boolean withSha256) throws IOException {
        MessageDigest md5Digest;
        try {
            md5Digest = MessageDigest.getInstance("MD5");
        } catch (NoSuchAlgorithmException exception) {
            throw new IOException("Calculate SO MD5 failed: " + path + ", " + exception.getMessage(), exception);
        }
        MessageDigest sha256Digest = null;
        if (withSha256) {
            try {
                sha256Digest = MessageDigest.getInstance("SHA-256");
            } catch (NoSuchAlgorithmException exception) {
                // Leave the cache empty for this file so the inter-app stage falls back to reading it
                LOG.warning("Failed to calculate sha256 for " + path + ", " + exception.getMessage());
            }
        }
        try (InputStream input = Files.newInputStream(path)) {
            byte[] buffer = new byte[BUFFER_SIZE];
            int len;
            while ((len = input.read(buffer)) != -1) {
                md5Digest.update(buffer, 0, len);
                if (sha256Digest != null) {
                    sha256Digest.update(buffer, 0, len);
                }
            }
        }
        return new SoDigests(toHex(md5Digest.digest()),
                sha256Digest == null ? null : toHex(sha256Digest.digest()));
    }

    private static String toHex(byte[] data) {
        StringBuilder builder = new StringBuilder();
        for (byte value : data) {
            builder.append(String.format("%02x", value & 0xff));
        }
        return builder.toString();
    }

    private static void copy(InputStream input, OutputStream output) throws IOException {
        byte[] buffer = new byte[BUFFER_SIZE];
        int len;
        while ((len = input.read(buffer)) != -1) {
            output.write(buffer, 0, len);
        }
    }

    private static String readUtf8(Path path) throws IOException {
        return new String(Files.readAllBytes(path), StandardCharsets.UTF_8);
    }

    private static String timestamp() {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.ENGLISH);
        format.setTimeZone(TimeZone.getTimeZone("UTC"));
        return format.format(new Date());
    }

    private static String getString(JSONObject object, String key, String defaultValue) {
        String value = object.getString(key);
        return value == null ? defaultValue : value;
    }

    private static List<String> getStringList(JSONArray array) {
        List<String> values = new ArrayList<>();
        if (array == null) {
            return values;
        }
        for (Object item : array) {
            if (item != null) {
                values.add(String.valueOf(item));
            }
        }
        return values;
    }

    private static List<String> getDeviceTypes(JSONArray array) {
        List<String> rawValues = getStringList(array);
        List<String> values = new ArrayList<>();
        for (String value : rawValues) {
            String normalized = normalizeDeviceType(value);
            if (!values.contains(normalized)) {
                values.add(normalized);
            }
        }
        return values;
    }

    private static String normalizeDeviceType(String deviceType) {
        return "default".equals(deviceType) ? "phone" : deviceType;
    }

    private static final class ModuleConfig {
        private String moduleName = "";
        private String moduleType = "";
        private String bundleType = "";
        private boolean stageModel = false;
        private List<String> deviceTypes = new ArrayList<>();
        private boolean deviceTypesConfigured = false;
        private boolean deliveryWithInstall = false;
        private boolean deliveryWithInstallPresent = false;
        private String distributionFilter = "";
        private List<String> requiredDeviceFeatures = new ArrayList<>();
        private boolean compressNativeLibs = false;
        private boolean extractNativeLibs = true;
        private boolean libIsolation = false;
        private String compileSdkType = ""; // "HarmonyOS" 或 "OpenHarmony"
        private InterAppDedupSolver.PackContext packContext = null;

        private boolean isEligible() {
            return isValidForDedup() && (compressNativeLibs || extractNativeLibs) && !libIsolation
                    && !OPEN_HARMONY.equals(compileSdkType);
        }

        private boolean isValidForDedup() {
            return stageModel && !moduleName.isEmpty() && isSupportedType()
                    && deviceTypesConfigured && deliveryWithInstallPresent;
        }

        private boolean isSupportedType() {
            return APP.equals(bundleType) || ATOMIC_SERVICE.equals(bundleType);
        }

    }

    private static final class ModuleRecord {
        private final String moduleId;
        private final String sourcePath;
        private final String reportPath;
        private final Path extractDir;
        private final Path repackedPath;
        private final ModuleConfig config;

        private ModuleRecord(String moduleId, String sourcePath, String reportPath, Path extractDir,
                             Path repackedPath, ModuleConfig config) {
            this.moduleId = moduleId;
            this.sourcePath = sourcePath;
            this.reportPath = reportPath;
            this.extractDir = extractDir;
            this.repackedPath = repackedPath;
            this.config = config;
        }
    }

    private static final class DeviceInstance {
        private final String type;
        private final String distributionFilter;

        private DeviceInstance(String type, String distributionFilter) {
            this.type = type;
            this.distributionFilter = distributionFilter;
        }

        @Override
        public boolean equals(Object other) {
            if (this == other) {
                return true;
            }
            if (!(other instanceof DeviceInstance)) {
                return false;
            }
            DeviceInstance device = (DeviceInstance) other;
            return type.equals(device.type) && distributionFilter.equals(device.distributionFilter);
        }

        @Override
        public int hashCode() {
            return 31 * type.hashCode() + distributionFilter.hashCode();
        }
    }

    private static final class SoRecord {
        private final String moduleId;
        private final String moduleName;
        private final String moduleFilePath;
        private final String relativePath;
        private final Path path;
        private final long size;

        private SoRecord(String moduleId, String moduleName, String moduleFilePath,
                         String relativePath, Path path, long size) {
            this.moduleId = moduleId;
            this.moduleName = moduleName;
            this.moduleFilePath = moduleFilePath;
            this.relativePath = relativePath;
            this.path = path;
            this.size = size;
        }
    }

    // Digests of a single .so file; the sha256 is null when it was not requested or is unavailable

    private static final class SoDigests {
        private final String md5;
        private final String sha256;

        private SoDigests(String md5, String sha256) {
            this.md5 = md5;
            this.sha256 = sha256;
        }
    }

    // Inter-app stage merge result: module instance id -> exclusion paths / dedupfiles.json content
    private static final class InterAppMergeResult {
        private final Map<String, Set<String>> excluded = new HashMap<>();
        private final Map<String, String> resultJson = new HashMap<>();
    }

    // Immutable inputs of one inter-app merge run, grouped so the per module merge keeps a short
    // parameter list
    private static final class InterAppMergeContext {
        private final InterAppDedupSolver solver;
        private final DedupListSdkParser dedupList;
        private final Map<String, Map<String, String>> soSha256Cache;
        private final Map<String, Set<String>> intraAppRemoved;
        private final InterAppMergeResult mergeResult;

        private InterAppMergeContext(InterAppDedupSolver solver, DedupListSdkParser dedupList,
                Map<String, Map<String, String>> soSha256Cache, Map<String, Set<String>> intraAppRemoved,
                InterAppMergeResult mergeResult) {
            this.solver = solver;
            this.dedupList = dedupList;
            this.soSha256Cache = soSha256Cache;
            this.intraAppRemoved = intraAppRemoved;
            this.mergeResult = mergeResult;
        }
    }

    private static final class ModuleReport {
        private final String moduleName;
        private final String filePath;
        private final List<String> kept = new ArrayList<>();
        private final List<String> removed = new ArrayList<>();

        private ModuleReport(String moduleName, String filePath) {
            this.moduleName = moduleName;
            this.filePath = filePath;
        }
    }

    private static final class DedupResult {
        private final Map<String, ModuleReport> modules = new LinkedHashMap<>();
        private final List<SoRecord> removedRecords = new ArrayList<>();
        private long totalSavedSize = 0L;

        private DedupResult() {
        }

        private void addKept(SoRecord so) {
            modules.computeIfAbsent(so.moduleId,
                    key -> new ModuleReport(so.moduleName, so.moduleFilePath)).kept.add(so.relativePath);
        }

        private void addRemoved(SoRecord so) {
            modules.computeIfAbsent(so.moduleId,
                    key -> new ModuleReport(so.moduleName, so.moduleFilePath)).removed.add(so.relativePath);
            removedRecords.add(so);
            totalSavedSize += so.size;
        }
    }
}
