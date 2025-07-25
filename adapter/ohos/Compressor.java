/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.ExecutionException;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.Locale;
import java.util.Optional;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.UUID;
import java.util.stream.Stream;
import java.util.zip.CRC32;
import java.util.zip.CheckedOutputStream;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;
import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.serializer.SerializerFeature;
import org.apache.commons.compress.archivers.zip.DefaultBackingStoreSupplier;
import org.apache.commons.compress.archivers.zip.ParallelScatterZipCreator;
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry;
import org.apache.commons.compress.archivers.zip.ZipArchiveOutputStream;
import org.apache.commons.compress.parallel.InputStreamSupplier;

/**
 * bundle compressor class, compress file and directory.
 *
 */
public class Compressor {
    private static final String HAP_SUFFIX = ".hap";
    private static final String HSP_SUFFIX = ".hsp";
    private static final String PNG_SUFFIX = ".png";
    private static final String UPPERCASE_PNG_SUFFIX = ".PNG";
    private static final String CONFIG_JSON = "config.json";
    private static final String MODULE_JSON = "module.json";
    private static final String PATCH_JSON = "patch.json";
    private static final String ADDITION_JSON = "addition.json";
    private static final String PKG_CONTEXT_INFO = "pkgContextInfo.json";
    private static final String NAME = "name";
    private static final String NULL_DIR_NAME = "";
    private static final String RES_DIR_NAME = "res/";
    private static final String RESOURCES_DIR_NAME = "resources/";
    private static final String LIBS_DIR_NAME = "libs/";
    private static final String AN_DIR_NAME = "an/";
    private static final String AP_PATH_NAME = "ap/";
    private static final String ASSETS_DIR_NAME = "assets/";
    private static final String SO_DIR_NAME = "maple/";
    private static final String SO_ARM64_DIR_NAME = "maple/arm64/";
    private static final String LINUX_FILE_SEPARATOR = "/";
    private static final String DISTRO = "distro";
    private static final String FORMS = "forms";
    private static final String MODULE_NAME = "module-name";
    private static final String MODULE_NAME_NEW = "moduleName";
    private static final String JSON_END = "}";
    private static final String SEMICOLON = "\"";
    private static final String COMPRESS_NATIVE_LIBS = "compressNativeLibs";
    private static final String SHARED_LIBS_DIR_NAME = "shared_libs/";
    private static final String DEVICE_TYPE = "deviceType";
    private static final String DEVICE_TYPE_FITNESSWATCH = "fitnessWatch";
    private static final String DEVICE_TYPE_FITNESSWATCH_NEW = "liteWearable";
    private static final String ENTRYCARD_NAME = "EntryCard/";
    private static final String PACKINFO_NAME = "pack.info";
    private static final String ENTRYCARD_BASE_NAME = "base";
    private static final String ENTRYCARD_SNAPSHOT_NAME = "snapshot";
    private static final String PIC_1X2 = "1x2";
    private static final String PIC_2X2 = "2x2";
    private static final String PIC_2X4 = "2x4";
    private static final String PIC_4X4 = "4x4";
    private static final String PIC_1X1 = "1x1";
    private static final String PIC_6X4 = "6x4";
    private static final String REGEX_LANGUAGE = "^[a-z]{2}$";
    private static final String REGEX_SCRIPT = "^[A-Z][a-z]{3}$";
    private static final String REGEX_COUNTRY = "^[A-Z]{2,3}|[0-9]{3}$";
    private static final String REGEX_ORIENTATION = "^vertical|horizontal$";
    private static final String REGEX_DEVICE_TYPE = "^phone|tablet|car|tv|wearable|liteWearable|2in1$";
    private static final String REGEX_SCREEN_DENSITY = "^sdpi|mdpi|ldpi|xldpi|xxldpi$";
    private static final String REGEX_COLOR_MODE = "^light|dark$";
    private static final String REGEX_SHAPE = "^circle$";
    private static final String JS_PATH = "js/";
    private static final String ETS_PATH = "ets/";
    private static final String HNP_PATH = "hnp/";
    private static final String TEMP_HAP_DIR = "tempHapDir";
    private static final String TEMP_HSP_DIR = "tempHspDir";
    private static final String TEMP_SELECTED_HAP_DIR = "tempSelectedHapDir";
    private static final String EMPTY_STRING = "";
    private static final String RELEASE = "Release";
    private static final String TYPE_SHARED = "shared";
    private static final String APP = "app";
    private static final String TYPE_APP_PLUGIN = "appPlugin";
    private static final String REQUEST_PERMISSIONS = "requestPermissions";
    private static final String PERMISSION_SUPPORT_PLUGIN = "ohos.permission.kernel.SUPPORT_PLUGIN";
    private static final String EXTENSION_ABILITIES = "extensionAbilities";
    private static final String MODULE = "module";
    private static final String MODULES = "modules";
    private static final String GENERATE_BUILD_HASH = "generateBuildHash";
    private static final String BUILD_HASH = "buildHash";
    private static final String TEMP_DIR = "temp";
    private static final String SHA_256 = "SHA-256";
    private static final String JSON_SUFFIX = ".json";
    private static final String ATOMIC_SERVICE = "atomicService";
    private static final String RAW_FILE_PATH = "resources/rawfile";
    private static final String RES_FILE_PATH = "resources/resfile";
    private static final String SUMMARY = "summary";
    private static final String VERSION_CODE = "versionCode";
    private static final String VERSION_NAME = "versionName";
    private static final String DEVICE_TYPES = "deviceTypes";
    private static final String BUNDLE_NAME = "bundleName";
    private static final String MIN_COMPATIBLE_VERSION_CODE = "minCompatibleVersionCode";
    private static final String MIN_API_VERSION = "minAPIVersion";
    private static final String TARGET_API_VERSION = "targetAPIVersion";
    private static final String API_RELEASE_TYPE = "apiReleaseType";
    private static final String BUNDLE_TYPE = "bundleType";
    private static final String INSTALLATION_FREE = "installationFree";
    private static final String DELIVERY_WITH_INSTALL = "deliveryWithInstall";
    private static final String API_VERSION = "apiVersion";
    private static final String RELEASE_TYPE = "releaseType";
    private static final String TARGET = "target";
    private static final String COMPATIBLE = "compatible";
    private static final String PACKAGES = "packages";
    private static final String VERSION = "version";
    private static final String CODE = "code";
    private static final String VERSION_RECORD = "version_record.json";
    private static final String GENERAL_RECORD = "general_record.json";
    private static final String RES_INDEX = "resources.index";
    private static final String ETS_FILE_NAME = "ets";
    private static final String HNP_FILE_NAME = "hnp";
    private static final String DIR_FILE_NAME = "dir";
    private static final String AN_FILE_NAME = "an";
    private static final String AP_FILE_NAME = "ap";
    private static final String RESOURCE_FILE_NAME = "resources";
    private static final String JS_FILE_NAME = "js";
    private static final String ASSETS_FILE_NAME = "assets";
    private static final String MAPLE_FILE_NAME = "maple";
    private static final String SHARED_LIBS_FILE_NAME = "shared_libs";
    private static final String LIBS_DIR = "libs";
    private static final String RPCID = "rpcid.sc";
    private static final String HAPADDITION_FOLDER_NAME = "hapAddition";
    private static final String TARGET_FILE_PATH = HAPADDITION_FOLDER_NAME + LINUX_FILE_SEPARATOR + "resources"
            + LINUX_FILE_SEPARATOR + "base" + LINUX_FILE_SEPARATOR + "profile";
    private static final String BACKUP_PREFIX = "backup";

    // set timestamp to get fixed MD5
    private static final int ATOMIC_SERVICE_ENTRY_SIZE_LIMIT_DEFAULT =  2048; // 2MB;unit is KB
    private static final int ATOMIC_SERVICE_NON_ENTRY_SIZE_LIMIT_DEFAULT = 2048; // 2MB;unit is KB
    private static final int ATOMIC_SERVICE_TOTAL_SIZE_LIMIT_DEFAULT = 4194304; // 4GB;unit is KB
    private static final int ATOMIC_SERVICE_TOTAL_SIZE_LIMIT_MAX = 4194304; // 4GB;unit is KB
    private static final int SHA256_BASE = 0xff;
    private static final int SHA256_OFFSET = 0x100;
    private static final int RADIX = 16;
    private static final int BEGIN_INDEX = 1;
    private static final int BUFFER_BYTE_SIZE = 1024;
    private static final int BUFFER_WRITE_SIZE = 1444;

    // set buffer size of each read
    private static final int BUFFER_SIZE = 40 * 1024;
    private static final Log LOG = new Log(Compressor.class.toString());
    private static final int SHARED_APP_HSP_LIMIT = 1;

    private static int entryModuleSizeLimit = 2048;
    private static int notEntryModuleSizeLimit = 2048;
    private static int sumModuleSizeLimit = 10240;
    private static final int INVALID_VERSION = -1;
    private static boolean isOverlay = false;

    private ZipArchiveOutputStream zipOut = null;
    private boolean mIsContain2x2EntryCard = true;
    private boolean isEntryOpen = false;
    private List<String> list = new ArrayList<String>();
    private List<String> formNamesList = new ArrayList<String>();
    private List<String> fileNameList = new ArrayList<String>();
    private List<String> supportDimensionsList = Arrays.asList(PIC_1X2, PIC_2X2, PIC_2X4, PIC_4X4, PIC_1X1, PIC_6X4);
    private HashMap<String, HapVerifyInfo> hapVerifyInfoMap = new HashMap<>();

    public static int getEntryModuleSizeLimit() {
        return entryModuleSizeLimit;
    }

    public static void setEntryModuleSizeLimit(int entry) {
        entryModuleSizeLimit = entry;
    }

    public static int getNotEntryModuleSizeLimit() {
        return notEntryModuleSizeLimit;
    }

    public static void setNotEntryModuleSizeLimit(int notEntry) {
        notEntryModuleSizeLimit = notEntry;
    }

    public static int getSumModuleSizeLimit() {
        return sumModuleSizeLimit;
    }

    public static void setSumModuleSizeLimit(int sumModule) {
        sumModuleSizeLimit = sumModule;
    }

    private static class VersionNormalizeUtil {
        private int originVersionCode = INVALID_VERSION;
        private String originVersionName = "";
        private String moduleName = "";

        public int getOriginVersionCode() {
            return originVersionCode;
        }

        public void setOriginVersionCode(int originVersionCode) {
            this.originVersionCode = originVersionCode;
        }

        public String getModuleName() {
            return moduleName;
        }

        public void setModuleName(String name) {
            this.moduleName = name;
        }

        public String getOriginVersionName() {
            return originVersionName;
        }

        public void setOriginVersionName(String originVersionName) {
            this.originVersionName = originVersionName;
        }
    }

    private static class GeneralNormalizeUtil {
        private int originVersionCode = INVALID_VERSION;
        private String originVersionName = "";
        private String moduleName = "";
        private String originBundleName = "";
        private int originMinCompatibleVersionCode = INVALID_VERSION;
        private int originMinAPIVersion = INVALID_VERSION;
        private int originTargetAPIVersion = INVALID_VERSION;
        private String originApiReleaseType = "";
        private String originBundleType = "";
        private boolean originInstallationFree = false;
        private boolean originDeliveryWithInstall = false;
        private String originDeviceTypes = "";
        private boolean isInstallationFree = false;
        private boolean isDeliveryWithInstall = false;

        public void setOriginVersionCode(int originVersionCode) {
            this.originVersionCode = originVersionCode;
        }

        public void setModuleName(String name) {
            this.moduleName = name;
        }

        public void setOriginVersionName(String originVersionName) {
            this.originVersionName = originVersionName;
        }

        public void setOriginBundleName(String originBundleName ) {
            this.originBundleName = originBundleName;
        }

        public void setOriginMinCompatibleVersionCode(int originMinCompatibleVersionCode) {
            this.originMinCompatibleVersionCode = originMinCompatibleVersionCode;
        }

        public void setOriginMinAPIVersion(int originMinAPIVersion) {
            this.originMinAPIVersion = originMinAPIVersion;
        }

        public void setOriginTargetAPIVersion(int originTargetAPIVersion) {
            this.originTargetAPIVersion = originTargetAPIVersion;
        }

        public void setOriginApiReleaseType(String originApiReleaseType) {
            this.originApiReleaseType = originApiReleaseType;
        }

        public void setOriginBundleType(String originBundleType) {
            this.originBundleType = originBundleType;
        }

        public void setIsInstallationFree(boolean isInstallationFree) {
            this.isInstallationFree = isInstallationFree;
        }

        public void setOriginInstallationFree(boolean originInstallationFree) {
            this.originInstallationFree = originInstallationFree;
        }

        public void setOriginDeliveryWithInstall(boolean originDeliveryWithInstall) {
            this.originDeliveryWithInstall = originDeliveryWithInstall;
        }

        public void setIsDeliveryWithInstall(boolean isDeliveryWithInstall) {
            this.isDeliveryWithInstall = isDeliveryWithInstall;
        }

        public void setOriginDeviceTypes(String originDeviceTypes) {
            this.originDeviceTypes = originDeviceTypes;
        }
    }

    /**
     * Parse atomicService size limit parameter from utility.
     *
     * @param utility Indicates the utility.
     */
    public void parseAtomicServiceSizeLimit(Utility utility) throws BundleException {
        parseAtomicServiceEntrySizeLimitParameter(utility);
        parseAtomicServiceNonEntrySizeLimitParameter(utility);
    }

    private int parseAtomicServiceSumSizeLimitParameter(Utility utility) throws BundleException{
        int sumLimit = ATOMIC_SERVICE_TOTAL_SIZE_LIMIT_DEFAULT;
        String totalLimit = utility.getAtomicServiceTotalSizeLimit();
        if (!totalLimit.isEmpty()) {
            try {
                sumLimit = Integer.parseInt(totalLimit);
            } catch (NumberFormatException e) {
                String errMsg = "parseAtomicServiceSumSizeLimitParameter failed, input --atomic-service-total-size-limit invalid.";
                String solution = "Check the --atomic-service-total-size-limit parameter.";
                LOG.error(PackingToolErrMsg.PARSE_ATOMIC_SERVICE_SIZE_LIMIT_FAILED.toString(errMsg, solution));
                throw new BundleException("parseAtomicServiceSumSizeLimitParameter failed, " +
                        "input --atomic-service-total-size-limit invalid.");
            }
            if (sumLimit < 0 || sumLimit > ATOMIC_SERVICE_TOTAL_SIZE_LIMIT_MAX) {
                String errMsg = "parseAtomicServiceSumSizeLimitParameter failed, " +
                        "input --atomic-service-total-size-limit value out of range [0,4194304].";
                String solution = "Check the --atomic-service-total-size-limit parameter is " +
                        "within the range of [0,4194304].";
                LOG.error(PackingToolErrMsg.PARSE_ATOMIC_SERVICE_SIZE_LIMIT_FAILED.toString(errMsg, solution));
                throw new BundleException("parseAtomicServiceSumSizeLimitParameter failed, " +
                        "input --atomic-service-total-size-limit value out of range [0,4194304].");
            }
        }
        setSumModuleSizeLimit(sumLimit);
        return sumLimit;
    }

    private void parseAtomicServiceEntrySizeLimitParameter(Utility utility) throws BundleException{
        String entrySizeLimitParamValue = utility.getAtomicServiceEntrySizeLimit();
        int entryLimit = ATOMIC_SERVICE_ENTRY_SIZE_LIMIT_DEFAULT;
        if (!entrySizeLimitParamValue.isEmpty()) {
            try {
                entryLimit = Integer.parseInt(entrySizeLimitParamValue);
            } catch (NumberFormatException e) {
                String errMsg = "parseAtomicServiceEntrySizeLimitParameter failed, " +
                        "input --atomic-service-entry-size-limit invalid.";
                String solution = "Check the --atomic-service-entry-size-limit parameter is invalid";
                LOG.error(PackingToolErrMsg.PARSE_ATOMIC_SERVICE_SIZE_LIMIT_FAILED.toString(errMsg, solution));
                throw new BundleException("parseAtomicServiceEntrySizeLimitParameter failed, " +
                        "input --atomic-service-entry-size-limit invalid.");
            }
            if (entryLimit < 0 || entryLimit > ATOMIC_SERVICE_TOTAL_SIZE_LIMIT_MAX) {
                String errMsg = "parseAtomicServiceEntrySizeLimitParameter failed, " +
                        "input --atomic-service-entry-size-limit value out of range [0,4194304].";
                String solution = "Check the --atomic-service-entry-size-limit parameter is " +
                        "within the valid range [0,4194304].";
                LOG.error(PackingToolErrMsg.PARSE_ATOMIC_SERVICE_SIZE_LIMIT_FAILED.toString(errMsg, solution));
                throw new BundleException("parseAtomicServiceEntrySizeLimitParameter failed, " +
                        "input --atomic-service-entry-size-limit value out of range [0,4194304].");
            }
        }
        setEntryModuleSizeLimit(entryLimit);
    }

    private void parseAtomicServiceNonEntrySizeLimitParameter(Utility utility) throws BundleException{
        String nonEntryLimitParamValue = utility.getAtomicServiceNonEntrySizeLimit();
        int notEntryLimit = ATOMIC_SERVICE_NON_ENTRY_SIZE_LIMIT_DEFAULT;
        if (!nonEntryLimitParamValue.isEmpty()) {
            try {
                notEntryLimit = Integer.parseInt(nonEntryLimitParamValue);
            } catch (NumberFormatException e) {
                String errMsg = "parseAtomicServiceSizeLimit failed, " +
                        "input --atomic-service-non-entry-size-limit invalid.";
                String solution = "Check the --atomic-service-non-entry-size-limit parameter";
                LOG.error(PackingToolErrMsg.PARSE_ATOMIC_SERVICE_SIZE_LIMIT_FAILED.toString(errMsg, solution));
                throw new BundleException("parseAtomicServiceSizeLimit failed, " +
                        "input --atomic-service-non-entry-size-limit invalid.");
            }
            if (notEntryLimit < 0 || notEntryLimit > ATOMIC_SERVICE_TOTAL_SIZE_LIMIT_MAX) {
                String errMsg = "parseAtomicServiceSizeLimit failed, " +
                        "input --atomic-service-non-entry-size-limit value out of range [0,4194304].";
                String solution = "Check the --atomic-service-non-entry-size-limit parameter is " +
                        "within the valid range [0,4194304].";
                LOG.error(PackingToolErrMsg.PARSE_ATOMIC_SERVICE_SIZE_LIMIT_FAILED.toString(errMsg, solution));
                throw new BundleException("parseAtomicServiceSizeLimit failed, " +
                        "input --atomic-service-non-entry-size-limit value out of range [0,4194304].");
            }
        }
        setNotEntryModuleSizeLimit(notEntryLimit);
    }

    /**
     * check path if is a module.json file
     *
     * @param path   path input
     * @return true if path is a module file
     */
    private static boolean isModuleJSON(String path)
    {
        File file = new File(path);
        if ((file.isFile()) && MODULE_JSON.equals(file.getName())) {
            return true;
        }
        return false;
    }

    /**
     * start compress.
     * file orders as follows:
     * for hap: 1.config.json 2.lib 3.res 4.assets 5.*.so 6.*.dex 7.*.apk 8.resources.index
     * for app: 1.certificate 2.signature 3.pack.info 4.hap (1 and 2 may not be used)
     *
     * @param utility common data
     * @return compressProcess if compress succeed
     */
    public boolean compressProcess(Utility utility) {
        switch (utility.getMode()) {
            case Utility.VERSION_NORMALIZE:
                versionNormalize(utility);
                return true;
            case Utility.MODE_HAPADDITION:
                hapAddition(utility);
                return true;
            case Utility.PACKAGE_NORMALIZE:
                return PackageNormalize.normalize(utility);
            case Utility.GENERAL_NORMALIZE:
                generalNormalize(utility);
                return true;
            default:
                return defaultProcess(utility);
        }
    }

    private boolean defaultProcess(Utility utility) {
        File destFile = new File(utility.getOutPath());
        // if out file directory not exist, mkdirs.
        File outParentFile = destFile.getParentFile();
        if ((outParentFile != null) && (!outParentFile.exists())) {
            if (!outParentFile.mkdirs()) {
                String errMsg = "Create output file's parent directory failed.";
                String solution = "Check the --out-path parameter.";
                LOG.error(PackingToolErrMsg.COMPRESS_PROCESS_FAILED.toString(errMsg, solution));
                return false;
            }
        }
        boolean compressResult = true;
        FileOutputStream fileOut = null;
        CheckedOutputStream checkedOut = null;
        try {
            parseAtomicServiceSizeLimit(utility);

            fileOut = new FileOutputStream(destFile);
            checkedOut = new CheckedOutputStream(fileOut, new CRC32());
            zipOut = new ZipArchiveOutputStream(checkedOut);
            zipOut.setLevel(utility.getCompressLevel());
            compressExcute(utility);
        } catch (FileNotFoundException exception) {
            compressResult = false;
            LOG.error(PackingToolErrMsg.FILE_NOT_FOUND.toString(
                    "Compress exist FileNotFoundException: " + exception.getMessage()));
        } catch (BundleException ex) {
            compressResult = false;
            LOG.error(PackingToolErrMsg.COMPRESS_PROCESS_EXCEPTION.toString(
                    "Compress exist BundleException: " + ex.getMessage()));
        } finally {
            closeZipOutputStream();
            Utility.closeStream(zipOut);
            Utility.closeStream(checkedOut);
            Utility.closeStream(fileOut);
        }

        if (compressResult && !checkAppAtomicServiceCompressedSizeValid(utility)) {
            compressResult = false;
            String errMsg = "The size of a single module, or the size of a module plus its dependencies, " +
                    "exceeds the maximum.";
            LOG.error(PackingToolErrMsg.CHECK_ATOMIC_SERVICE_SIZE_FAILED.toString(errMsg));
        }

        // if compress failed, delete out file.
        if (!compressResult) {
            String errMsg = "Compress process failed.";
            String solution = "Please check the first error message for more details and modify accordingly.";
            LOG.error(PackingToolErrMsg.COMPRESS_PROCESS_FAILED.toString(errMsg, solution));
            if (!destFile.delete()) {
                errMsg = "Delete the output file " + utility.getOutPath() + " failed.";
                solution = "Try to close the output file using programme.";
                LOG.error(PackingToolErrMsg.FILE_DELETE_FAILED.toString(errMsg, solution));
            }
        }
        return compressResult;
    }

    private void compressExcute(Utility utility) throws BundleException {
        switch (utility.getMode()) {
            case Utility.MODE_HAP:
                compressHap(utility);
                break;
            case Utility.MODE_HAR:
                compressHarMode(utility);
                break;
            case Utility.MODE_APP:
                compressAppMode(utility);
                break;
            case Utility.MODE_FAST_APP:
                compressFastAppMode(utility);
                break;
            case Utility.MODE_MULTI_APP:
                compressAppModeForMultiProject(utility);
                break;
            case Utility.MODE_HQF:
                compressHQFMode(utility);
                break;
            case Utility.MODE_APPQF:
                compressAPPQFMode(utility);
                break;
            case Utility.MODE_HSP:
                compressHsp(utility);
                break;
            default:
                compressPackResMode(utility);
        }
    }

    private void compressHsp(Utility utility) throws BundleException {
        setGenerateBuildHash(utility);
        if (isModuleJSON(utility.getJsonPath())) {
            Optional<String> optional = FileUtils.getFileContent(utility.getJsonPath());
            String jsonString = optional.get();
            if (!checkStageAsanTsanEnabledValid(jsonString)) {
                LOG.error(PackingToolErrMsg.COMPRESS_HSP_FAILED.toString(
                        "Check the asanTsanEnabled parameter in the Stage module failed."));
                throw new BundleException("Compress hsp failed.");
            }
            if (!checkStageHwasanEnabledValid(jsonString)) {
                LOG.error(PackingToolErrMsg.COMPRESS_HSP_FAILED.toString(
                        "Check the hwasanEnabled parameter in the Stage module failed."));
                throw new BundleException("Compress hsp failed.");
            }
            if (!checkStageUbsanEnabledValid(jsonString)) {
                LOG.error(PackingToolErrMsg.COMPRESS_HSP_FAILED.toString(
                        "Check the ubsanEnabled parameter in the Stage module failed."));
                throw new BundleException("Compress hsp failed.");
            }
            if (!checkStageAtomicService(jsonString)) {
                LOG.error(PackingToolErrMsg.COMPRESS_HSP_FAILED.toString(
                        "Check the atomicService parameter in the Stage module failed."));
                throw new BundleException("Check stage AtomicService failed.");
            }
            // check continueBundleName in module.json
            if (!checkContinueBundleNameIsValid(jsonString)) {
                LOG.error(PackingToolErrMsg.COMPRESS_HSP_FAILED.toString(
                        "Check the continueBundleName parameter in the Stage module failed."));
                throw new BundleException("Compress hsp failed.");
            }
            // check whether is an overlay hsp or not
            if (!checkStageOverlayCfg(jsonString)) {
                LOG.error(PackingToolErrMsg.COMPRESS_HSP_FAILED.toString(
                        "Check the overlay config in the Stage module failed."));
                throw new BundleException("Compress hsp failed.");
            }
            String moduleType = ModuleJsonUtil.parseModuleType(jsonString);
            if (!TYPE_SHARED.equals(moduleType)) {
                LOG.error(PackingToolErrMsg.COMPRESS_HSP_FAILED.toString("Module type must be shared."));
                throw new BundleException("Compress hsp failed.");
            }
        }
        if (!checkAppPlugin(utility)) {
            LOG.error(PackingToolErrMsg.COMPRESS_HSP_FAILED.toString("plugin package packaging failed."));
            throw new BundleException("Compress hsp failed.");
        }
        compressHSPMode(utility);
        buildHash(utility);
    }

    private void compressHap(Utility utility) throws BundleException {
        if (utility.getJsonPath().isEmpty() && !utility.getBinPath().isEmpty()) {
            // only for slim device
            compressHapMode(utility);
            return;
        }
        setGenerateBuildHash(utility);
        if (isModuleJSON(utility.getJsonPath())) {
            if (!checkStageHap(utility)) {
                LOG.error(PackingToolErrMsg.COMPRESS_HAP_FAILED.toString(
                        "Verify the module.json file of the Stage HAP package failed."));
                throw new BundleException("Verify the module.json file of the Stage HAP package failed.");
            }
            Optional<String> optional = FileUtils.getFileContent(utility.getJsonPath());
            String jsonString = optional.get();
            String moduleType = ModuleJsonUtil.parseModuleType(jsonString);
            if (TYPE_SHARED.equals(moduleType)) {
                LOG.warning("Compress mode is hap, but module type is shared.");
            }
            String bundleType = ModuleJsonUtil.parseStageBundleType(jsonString);
            if (TYPE_SHARED.equals(bundleType)) {
                LOG.warning("Compress mode is hap, but app type is shared.");
            }
            if (!TYPE_APP_PLUGIN.equals(bundleType)) {
                if (!isPluginHost(utility)) {
                    if (!checkPkgContext(utility)) {
                        LOG.error(PackingToolErrMsg.COMPRESS_HAP_FAILED.toString(
                                "plugin package packing failed, the pkgContextInfo.json not exist."));
                        throw new BundleException("Compress hap failed.");
                    }
                }
            } else {
                LOG.error(PackingToolErrMsg.COMPRESS_HAP_FAILED.toString("hap can not plugin."));
                throw new BundleException("Compress hap failed.");
            }
            compressHapModeForModule(utility);
            buildHash(utility);
        } else {
            compressHapMode(utility);
            buildHash(utility);
        }
    }

    private static boolean isPluginHost(Utility utility) throws BundleException {
        File file = new File(utility.getJsonPath());
        if (!file.exists()) {
            String errMsg = "The --json-path file does not exist.";
            LOG.error(PackingToolErrMsg.FILE_NOT_EXIST.toString(errMsg));
            throw new BundleException("Verify has generate build hash failed for --json-path file does not exist.");
        }
        try (InputStream json = new FileInputStream(file)) {
            JSONObject jsonObject = JSON.parseObject(json, JSONObject.class);
            if (jsonObject == null || !jsonObject.containsKey(MODULE)) {
                LOG.error(PackingToolErrMsg.IS_PLUGIN_HOST_FAILED.toString("The --json-path file is invalid."));
                throw new BundleException("Parse --json-path file is invalid.");
            }
            JSONObject moduleJson = jsonObject.getJSONObject(MODULE);
            if (moduleJson.containsKey(REQUEST_PERMISSIONS)) {
                JSONArray requestPermissions = moduleJson.getJSONArray(REQUEST_PERMISSIONS);
                for (int i = 0; i < requestPermissions.size(); ++i) {
                    JSONObject requestPermission = requestPermissions.getJSONObject(i);
                    if (isPermissionSupportPlugin(requestPermission)) {
                        return false;
                    }
                }
            }
            return true;
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(
                            "check is plugin host exist IOException: " + e.getMessage()));
        }
        return true;
    }

    private static boolean isPermissionSupportPlugin(JSONObject requestPermission) throws BundleException {
        String reqPermissionName = getJsonString(requestPermission, NAME);
        if (reqPermissionName.equals(PERMISSION_SUPPORT_PLUGIN)) {
            return true;
        }
        return false;
    }

    private static boolean checkPkgContext(Utility utility) throws BundleException {
        if (!utility.getPkgContextPath().isEmpty()) {
            File file = new File(utility.getPkgContextPath());
            if (!file.isFile() || !PKG_CONTEXT_INFO.equals(file.getName())) {
                String errMsg = "--pkg-context-path file must be the pkgContextInfo.json file.";
                LOG.error(PackingToolErrMsg.CHECK_PKG_CONTEXT_FAILED.toString(errMsg));
                return false;
            }
            return true;
        }
        LOG.error(PackingToolErrMsg.CHECK_PKG_CONTEXT_FAILED.toString("no have pkgContextInfo.json file."));
        return false;
    }

    private static boolean checkAppPlugin(Utility utility) throws BundleException {
        File file = new File(utility.getJsonPath());
        if (!file.exists()) {
            String errMsg = "The --json-path file does not exist.";
            LOG.error(PackingToolErrMsg.CHECK_APP_PLUGIN_FAILED.toString(errMsg));
            throw new BundleException("Verify has generate build hash failed for --json-path file does not exist.");
        }
        try (InputStream json = new FileInputStream(file)) {
            JSONObject jsonObject = JSON.parseObject(json, JSONObject.class);
            if (jsonObject == null || !jsonObject.containsKey(MODULE)) {
                LOG.error(PackingToolErrMsg.CHECK_APP_PLUGIN_FAILED.toString("The --json-path file is invalid."));
                throw new BundleException("Parse --json-path file is invalid.");
            }
            Optional<String> optional = FileUtils.getFileContent(utility.getJsonPath());
            String jsonString = optional.get();
            String bundleType = ModuleJsonUtil.parseStageBundleType(jsonString);
            if (!TYPE_APP_PLUGIN.equals(bundleType)) {
                if (isPluginHost(utility)) {
                    return true;
                }
                if (!checkPkgContext(utility)) {
                    LOG.error(PackingToolErrMsg.CHECK_APP_PLUGIN_FAILED.toString("checkPkgContext failed."));
                    return false;
                }
                return true;
            }
            JSONObject moduleJson = jsonObject.getJSONObject(MODULE);
            JSONArray extensionAbilityJsonList = moduleJson.getJSONArray(EXTENSION_ABILITIES);
            if (extensionAbilityJsonList != null) {
                for (int j = 0; j < extensionAbilityJsonList.size(); j++) {
                    JSONObject extensionAbilityJson = extensionAbilityJsonList.getJSONObject(j);
                    if (extensionAbilityJson != null) {
                        LOG.error(PackingToolErrMsg.CHECK_APP_PLUGIN_FAILED.toString("extensionAbilities is not null."));
                        return false;
                    }
                }
            }
            if (!checkPkgContext(utility)) {
                LOG.error(PackingToolErrMsg.CHECK_APP_PLUGIN_FAILED.toString("checkPkgContext failed."));
                return false;
            }
            if (!isPluginHost(utility)) {
                LOG.error(PackingToolErrMsg.CHECK_APP_PLUGIN_FAILED.toString("plugin package cannot be the host."));
                return false;
            }
            return true;
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(
                            "check app plugin exist IOException: " + e.getMessage()));
        }
        return true;
    }

    /**
     * get the String from JSONObject by the key.
     *
     * @param jsonObject uncompress json object
     * @param key value key
     * @return the result
     */
    private static String getJsonString(JSONObject jsonObject, String key) {
        String value = "";
        if (jsonObject != null && jsonObject.containsKey(key) && jsonObject.get(key) != null) {
            value = jsonObject.get(key).toString();
        }
        return value;
    }

    private static boolean hasGenerateBuildHash(Utility utility) throws BundleException {
        File file = new File(utility.getJsonPath());
        if (!file.exists()) {
            String errMsg = "The --json-path file does not exist.";
            LOG.error(PackingToolErrMsg.HAS_GENERATE_BUILD_HASH.toString(errMsg));
            throw new BundleException("Verify has generate build hash failed for --json-path file does not exist.");
        }
        InputStream json = null;
        boolean res = false;
        try {
            json = new FileInputStream(file);
            JSONObject jsonObject = JSON.parseObject(json, JSONObject.class);
            if (jsonObject == null || !jsonObject.containsKey(APP) || !jsonObject.containsKey(MODULE)) {
                LOG.error(PackingToolErrMsg.HAS_GENERATE_BUILD_HASH.toString("The --json-path file is invalid."));
                throw new BundleException("Parse --json-path file is invalid.");
            }
            JSONObject appJson = jsonObject.getJSONObject(APP);
            JSONObject moduleJson = jsonObject.getJSONObject(MODULE);
            if (appJson.containsKey(GENERATE_BUILD_HASH) || moduleJson.containsKey(GENERATE_BUILD_HASH)) {
                res = true;
            }
        } catch (BundleException exception) {
            LOG.error(PackingToolErrMsg.HAS_GENERATE_BUILD_HASH.toString(
                    "Verify has generate build hash exist BundleException: " + exception.getMessage()));
            throw new BundleException("Verify has generate build hash failed.");
        } catch (JSONException | IOException e) {
            LOG.error(PackingToolErrMsg.HAS_GENERATE_BUILD_HASH.toString(
                    "Verify has generate build hash exist Exception (JSONException | IOException): " +
                    e.getMessage()));
            throw new BundleException("Verify has generate build hash failed.");
        } finally {
            FileUtils.closeStream(json);
        }

        return res;
    }

    private static void setGenerateBuildHash(Utility utility) throws BundleException {
        if (utility.isBuildHashFinish() || !hasGenerateBuildHash(utility)) {
            return;
        }
        copyFileToTempDir(utility);
        File file = new File(utility.getJsonPath());
        if (!file.exists()) {
            String errMsg = "The --json-path file does not exist.";
            LOG.error(PackingToolErrMsg.SET_GENERATE_BUILD_HASH.toString(errMsg));
            throw new BundleException("Set generate build hash failed for --json-path file does not exist.");
        }
        // 1. 解析 JSON
        JSONObject jsonObject;
        try (InputStream json = Files.newInputStream(file.toPath())) {
            jsonObject = JSON.parseObject(json, JSONObject.class);
        } catch (IOException | JSONException e) {
            LOG.error(PackingToolErrMsg.SET_GENERATE_BUILD_HASH.toString(
                    "Failed to read JSON file: " + e.getMessage()));
            throw new BundleException("Failed to read JSON file");
        }
        //2. 检查必要字段
        if (jsonObject == null || !jsonObject.containsKey(APP) || !jsonObject.containsKey(MODULE)) {
            LOG.error(PackingToolErrMsg.SET_GENERATE_BUILD_HASH.toString("Parse --json-path file is invalid."));
            throw new BundleException("The --json-path file is invalid.");
        }
        //3. 处理 JSON 数据
        processBuildHashFlags(utility, jsonObject);
        //4. 写入修改后的 JSON
        writeJsonFile(utility.getJsonPath(), jsonObject);
    }

    private static void processBuildHashFlags(Utility utility, JSONObject jsonObject) {
        JSONObject appJson = jsonObject.getJSONObject(APP);
        JSONObject moduleJson = jsonObject.getJSONObject(MODULE);
        if (appJson.containsKey(GENERATE_BUILD_HASH) && appJson.getBoolean(GENERATE_BUILD_HASH)) {
            utility.setGenerateBuildHash(true);
        } else if (moduleJson.containsKey(GENERATE_BUILD_HASH) && moduleJson.getBoolean(GENERATE_BUILD_HASH)) {
            utility.setGenerateBuildHash(true);
        }
        appJson.remove(GENERATE_BUILD_HASH);
        moduleJson.remove(GENERATE_BUILD_HASH);
    }

    private static void writeJsonFile(String filePath, JSONObject jsonObject) throws BundleException {
        try (BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(
                Files.newOutputStream(Paths.get(filePath)), StandardCharsets.UTF_8))) {
            String pretty = JSON.toJSONString(jsonObject, new SerializerFeature[]{
                    SerializerFeature.PrettyFormat,
                    SerializerFeature.WriteMapNullValue,
                    SerializerFeature.WriteDateUseDateFormat});
            bw.write(pretty);
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.SET_GENERATE_BUILD_HASH.toString(
                    "Failed to write JSON file: " + e.getMessage()));
            throw new BundleException("Failed to write JSON file");
        }
    }

    private static void copyFileToTempDir(Utility utility) throws BundleException {
        String jsonPath = utility.getJsonPath();
        File oldfile = new File(jsonPath);
        if (!oldfile.exists()) {
            String errMsg = "Parse --json-path file does not found, parse json path is " + jsonPath + ".";
            LOG.error(PackingToolErrMsg.FILE_NOT_EXIST.toString(errMsg));
            throw new BundleException("Copy file to temp dir failed for --json-path file not found.");
        }
        String oldFileParent = oldfile.getParent();
        String tempDir = TEMP_DIR + File.separator + UUID.randomUUID();
        mkdir(new File(oldFileParent + File.separator + tempDir));
        String fileName;
        if (isModuleJSON(utility.getJsonPath())) {
            fileName = MODULE_JSON;
        } else {
            fileName = CONFIG_JSON;
        }
        String tempPath = oldFileParent + File.separator + tempDir + File.separator + fileName;

        try (InputStream inStream = new FileInputStream(jsonPath);
             FileOutputStream fs = new FileOutputStream(tempPath)) {
            byte[] buffer = new byte[BUFFER_WRITE_SIZE];
            int byteread;
            while ((byteread = inStream.read(buffer)) != -1) {
                fs.write(buffer, 0, byteread);
            }
            utility.setJsonPath(tempPath);
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(
                    "Copy file to temp dir exist IOException:" + e.getMessage()));
            throw new BundleException("Copy file to temp dir failed.");
        }
    }

    private static void mkdir(File file) {
        if (null != file && !file.exists()) {
            mkdir(file.getParentFile());
            file.mkdir();
        }
    }

    private static void buildHash(Utility utility) throws BundleException {
        if (utility.isBuildHashFinish() || (!utility.getGenerateBuildHash())) {
            return;
        }
        String filePath = utility.getOutPath();
        String hash = getSHA256(filePath);
        try {
            putBuildHash(utility, hash);
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Build hash exist IOException: " + e.getMessage()));
            throw new BundleException("Put build hash failed.");
        }
    }

    private static byte[] checkSum(String filename) throws BundleException {
        try (InputStream fis = new FileInputStream(filename)) {
            byte[] buffer = new byte[BUFFER_BYTE_SIZE];
            MessageDigest complete = MessageDigest.getInstance(SHA_256);
            int numRead;
            do {
                numRead = fis.read(buffer);
                if (numRead > 0) {
                    complete.update(buffer, 0, numRead);
                }
            } while (numRead != -1);
            return complete.digest();
        } catch (IOException | NoSuchAlgorithmException e) {
            LOG.error(PackingToolErrMsg.SHA256_CALCULATION_FAILED.toString("SHA-256 checksum calculation exist " +
                    "Exception (IOException | NoSuchAlgorithmException): " + e.getMessage()));
            throw new BundleException("Compressor::checkSum failed.");
        }
    }

    /**
     * get SHA256 of hap or hsp
     *
     * @param filePath the path of hap or hsp.
     */
    public static String getSHA256(String filePath) throws BundleException {
        byte[] byteSum = checkSum(filePath);
        StringBuilder temp = new StringBuilder();
        for (int i = 0; i < byteSum.length; i++) {
            temp.append(
                    Integer.toString((byteSum[i] & SHA256_BASE) + SHA256_OFFSET, RADIX).substring(BEGIN_INDEX));
        }
        return temp.toString();
    }

    private static void putBuildHash(Utility utility, String hash) throws BundleException, IOException {
        if (utility.isBuildHashFinish()) {
            return;
        }
        String jsonPath = utility.getJsonPath();
        File file = new File(jsonPath);
        if (!file.exists()) {
            String errMsg = "The --json-path file does not exist.";
            LOG.error(PackingToolErrMsg.FILE_NOT_EXIST.toString(errMsg));
            throw new BundleException("Put build hash failed for json file not exist.");
        }
        InputStream json = null;
        BufferedWriter bw = null;
        try {
            json = new FileInputStream(file);
            JSONObject jsonObject = JSON.parseObject(json, JSONObject.class);
            JSONObject moduleJson = jsonObject.getJSONObject(MODULE);
            moduleJson.put(BUILD_HASH, hash);
            String pretty = JSON.toJSONString(jsonObject, SerializerFeature.PrettyFormat,
                    SerializerFeature.WriteMapNullValue, SerializerFeature.WriteDateUseDateFormat);
            bw = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(jsonPath), StandardCharsets.UTF_8));
            bw.write(pretty);
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Put build hash exist IOException: " + e.getMessage()));
            throw new BundleException("Put build hash failed.");
        } catch (NullPointerException e) {
            LOG.error(PackingToolErrMsg.NULL_POINTER_EXCEPTION.toString(
                    "The json data err, exist NullPointerException: " + e.getMessage()));
            throw new BundleException("Put build hash failed, json data err.");
        } finally {
            FileUtils.closeStream(json);
            if (bw != null) {
                bw.flush();
                bw.close();
            }
        }
        utility.setBuildHashFinish(true);
    }

    private boolean checkAppAtomicServiceCompressedSizeValid(Utility utility) {
        if (!utility.getMode().equals(Utility.MODE_APP) &&
                !utility.getMode().equals(Utility.MODE_FAST_APP) &&
                !utility.getMode().equals(Utility.MODE_MULTI_APP)) {
            return true;
        }

        File destFile = new File(utility.getOutPath());
        List<HapVerifyInfo> hapVerifyInfos = new ArrayList<>();
        try (ZipFile zipApp = new ZipFile(destFile)) {
            Enumeration<? extends ZipEntry> entries = zipApp.entries();
            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();
                if (entry != null && hapVerifyInfoMap.containsKey(entry.getName())) {
                    hapVerifyInfoMap.get(entry.getName()).setFileLength(entry.getCompressedSize());
                    hapVerifyInfos.add(hapVerifyInfoMap.get(entry.getName()));
                }
            }

            if (hapVerifyInfos.isEmpty()) {
                LOG.error(PackingToolErrMsg.CHECK_HAP_VERIFY_INFO_LIST_EMPTY.toString("Hap verify infos is empty"));
                return false;
            }

            String bundleType = hapVerifyInfos.get(0).getBundleType();
            if (!bundleType.equals(ATOMIC_SERVICE)) {
                return true;
            }
            boolean isStage = hapVerifyInfos.get(0).isStageModule();
            if (!isStage) {
                return true;
            }

            return HapVerify.checkFileSizeIsValid(hapVerifyInfos);
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.APP_ATOMICSERVICE_COMPRESSED_SIZE_INVALID.toString(
                    "IOException: " + exception.getMessage()));
            return false;
        } catch (BundleException ignored) {
            LOG.error(PackingToolErrMsg.APP_ATOMICSERVICE_COMPRESSED_SIZE_INVALID.toString(
                    "BundleException: " + ignored.getMessage()));
            return false;
        }
    }

    private static boolean checkStageHap(Utility utility) throws BundleException {
        Optional<String> optional = FileUtils.getFileContent(utility.getJsonPath());
        String jsonString = optional.get();
        if (!checkStageAsanTsanEnabledValid(jsonString)) {
            LOG.error(PackingToolErrMsg.CHECK_STAGE_HAP_FAILED.toString(
                    "Check the asanTsanEnabled parameter in the Stage module failed."));
            return false;
        }
        if (!checkStageHwasanEnabledValid(jsonString)) {
            LOG.error(PackingToolErrMsg.CHECK_STAGE_HAP_FAILED.toString(
                    "Check the hwasanEnabled parameter in the Stage module failed."));
            return false;
        }
        if (!checkStageUbsanEnabledValid(jsonString)) {
            LOG.error(PackingToolErrMsg.CHECK_STAGE_HAP_FAILED.toString(
                    "Check the ubsanEnabled parameter in the Stage module failed."));
            return false;
        }
        // check atomicService in module.json
        if (!checkStageAtomicService(jsonString)) {
            LOG.error(PackingToolErrMsg.CHECK_STAGE_HAP_FAILED.toString(
                    "Check the atomicService parameter in the Stage module failed."));
            return false;
        }
        // check continueBundleName in module.json
        if (!checkContinueBundleNameIsValid(jsonString)) {
            LOG.error(PackingToolErrMsg.CHECK_STAGE_HAP_FAILED.toString(
                    "Check the continueBundleName parameter in the Stage module failed."));
            return false;
        }
        return true;
    }

    private static boolean checkStageAsanTsanEnabledValid(String jsonString) throws BundleException {
        boolean asanEnabled = ModuleJsonUtil.getStageAsanEnabled(jsonString);
        boolean tsanEnabled = ModuleJsonUtil.getStageTsanEnabled(jsonString);
        if (asanEnabled && tsanEnabled) {
            LOG.error(PackingToolErrMsg.CHECK_AS_TSAN_ENABLED.toString(
                    "asanEnabled and tsanEnabled cannot be true at the same time."));
            return false;
        }
        return true;
    }

    private static boolean checkStageHwasanEnabledValid(String jsonString) throws BundleException {
        boolean asanEnabled = ModuleJsonUtil.getStageAsanEnabled(jsonString);
        boolean tsanEnabled = ModuleJsonUtil.getStageTsanEnabled(jsonString);
        boolean gwpAsanEnabled = ModuleJsonUtil.getStageGwpAsanEnabled(jsonString);
        boolean hwasanEnabled = ModuleJsonUtil.getStageHwasanEnabled(jsonString);
        if (hwasanEnabled && asanEnabled) {
            LOG.error(PackingToolErrMsg.CHECK_HWASAN_ENABLED_INVALID.toString(
                    "hwasanEnabled and asanEnabled cannot be true at the same time."));
            return false;
        }
        if (hwasanEnabled && tsanEnabled) {
            LOG.error(PackingToolErrMsg.CHECK_HWASAN_ENABLED_INVALID.toString(
                    "hwasanEnabled and tsanEnabled cannot be true at the same time."));
            return false;
        }
        if (hwasanEnabled && gwpAsanEnabled) {
            LOG.error(PackingToolErrMsg.CHECK_HWASAN_ENABLED_INVALID.toString(
                    "hwasanEnabled and GWPAsanEnabled cannot be true at the same time."));
            return false;
        }
        return true;
    }

    private static boolean checkContinueBundleNameIsValid(String jsonString) throws BundleException {
        Map<String, List<String>> continueBundleNameMap = ModuleJsonUtil.getAbilityContinueBundleNameMap(jsonString);
        String bundleName = ModuleJsonUtil.parseBundleName(jsonString);
        String moduleName = ModuleJsonUtil.parseStageModuleName(jsonString);
        for (Map.Entry<String, List<String>> entry : continueBundleNameMap.entrySet()) {
            List<String> continueBundleNameList = entry.getValue();
            if (continueBundleNameList == null) {
                continue;
            }
            for (int i = 0; i < continueBundleNameList.size(); i++) {
                if (bundleName.equals(continueBundleNameList.get(i))) {
                    LOG.error(PackingToolErrMsg.CHECK_CONTINUE_BUNDLENAME_INVALID.toString(
                            "Module(" + moduleName + ") continueBundleName include self."));
                    return false;
                }
            }
        }
        return true;
    }

    private static boolean checkStageUbsanEnabledValid(String jsonString) throws BundleException {
        boolean asanEnabled = ModuleJsonUtil.getStageAsanEnabled(jsonString);
        boolean tsanEnabled = ModuleJsonUtil.getStageTsanEnabled(jsonString);
        boolean hwasanEnabled = ModuleJsonUtil.getStageHwasanEnabled(jsonString);
        boolean ubsanEnabled = ModuleJsonUtil.getStageUbsanEnabled(jsonString);
        if (ubsanEnabled && asanEnabled) {
            LOG.error(PackingToolErrMsg.CHECK_UBASAN_ENABLED_INVALID.toString(
                    "ubsanEnabled and asanEnabled cannot be true at the same time."));
            return false;
        }
        if (ubsanEnabled && tsanEnabled) {
            LOG.error(PackingToolErrMsg.CHECK_UBASAN_ENABLED_INVALID.toString(
                    "ubsanEnabled and tsanEnabled cannot be true at the same time."));
            return false;
        }
        if (ubsanEnabled && hwasanEnabled) {
            LOG.error(PackingToolErrMsg.CHECK_UBASAN_ENABLED_INVALID.toString(
                    "ubsanEnabled and hwasanEnabled cannot be true at the same time."));
            return false;
        }
        return true;
    }

    private static boolean checkStageAtomicService(String jsonString) throws BundleException {
        // check consistency of atomicService
        if (!ModuleJsonUtil.isModuleAtomicServiceValid(jsonString)) {
            LOG.error(PackingToolErrMsg.CHECK_ATOMIC_SERVICE_FAILED.toString(
                    "Check consistency of atomicService failed."));
            return false;
        }
        // check entry module must have ability
        if (!ModuleJsonUtil.checkEntryInAtomicService(jsonString)) {
            LOG.error(PackingToolErrMsg.CHECK_ATOMIC_SERVICE_FAILED.toString(
                    "Check the atomicService entry module failed."));
            return false;
        }
        // check installationFree
        if (!ModuleJsonUtil.checkAtomicServiceInstallationFree(jsonString)) {
            LOG.error(PackingToolErrMsg.CHECK_ATOMIC_SERVICE_FAILED.toString(
                    "Check the installationFree parameter failed."));
            return false;
        }

        return true;
    }

    private static boolean checkStageOverlayCfg(String jsonString) throws BundleException {
        // check module
        String targetModuleName = ModuleJsonUtil.getStageTargetModuleName(jsonString);
        String moduleName = ModuleJsonUtil.parseStageModuleName(jsonString);
        if (!targetModuleName.isEmpty()) {
            // check targetModuleName and requestPermission
            if (ModuleJsonUtil.isExistedStageRequestPermissions(jsonString)) {
                LOG.error(PackingToolErrMsg.CHECK_OVERLAY_CFG_FAILED.toString(
                        "The module(" + moduleName + ") targetModuleName and requestPermissions " +
                        "cannot be configured at the same time."));
                return false;
            }
            // check targetModuleName and name
            if (targetModuleName.equals(moduleName)) {
                LOG.error(PackingToolErrMsg.CHECK_OVERLAY_CFG_FAILED.toString(
                        "The targetModuleName of module(" + moduleName + ") cannot be itself."));
                return false;
            }
        } else {
            if (ModuleJsonUtil.isExistedStageModuleTargetPriority(jsonString)) {
                LOG.error(PackingToolErrMsg.CHECK_OVERLAY_CFG_FAILED.toString(
                        "The targetPriority cannot be existed without the targetModuleName in module.json. " +
                        "The moduleName is " + moduleName + "."));
                return false;
            }
        }
        // check app
        String targetBundleName = ModuleJsonUtil.getStageTargetBundleName(jsonString);
        if (!targetBundleName.isEmpty()) {
            if (targetModuleName.isEmpty()) {
                LOG.error(PackingToolErrMsg.CHECK_OVERLAY_CFG_FAILED.toString("The module(" + moduleName +
                        ") targetModuleName settings is necessary in the overlay bundle."));
                return false;
            }
            if (targetBundleName.equals(ModuleJsonUtil.parseBundleName(jsonString))) {
                LOG.error(PackingToolErrMsg.CHECK_OVERLAY_CFG_FAILED.toString(
                        "The module(" + moduleName + ") targetBundleName cannot be same with the bundleName."));
                return false;
            }
        } else {
            if (ModuleJsonUtil.isExistedStageAppTargetPriority(jsonString)) {
                LOG.error(PackingToolErrMsg.CHECK_OVERLAY_CFG_FAILED.toString("The targetPriority cannot be existed " +
                        "without the targetBundleName in app.json. The moduleName is " + moduleName + "."));
                return false;
            }
        }
        return true;
    }

    private static boolean checkFAHap(Utility utility) throws BundleException {
        Optional<String> optional = FileUtils.getFileContent(utility.getJsonPath());
        String jsonString = optional.get();
        return checkFAAsanEnabledValid(jsonString);
    }

    private static boolean checkFAAsanEnabledValid(String jsonString) throws BundleException {
        boolean asanEnabled = ModuleJsonUtil.getFAAsanEnabled(jsonString);
        boolean debug = ModuleJsonUtil.getFADebug(jsonString);
        if (asanEnabled && !debug) {
            LOG.error("asanEnabled is not supported for Release.");
            return false;
        }
        return true;
    }

    /**
     * compress in hap mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressHapMode(Utility utility) throws BundleException {
        pathToFile(utility, utility.getJsonPath(), NULL_DIR_NAME, false);

        pathToFile(utility, utility.getProfilePath(), NULL_DIR_NAME, false);

        if (!utility.getIndexPath().isEmpty() && !utility.getModuleName().isEmpty()) {
            String assetsPath = ASSETS_DIR_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR;
            pathToFile(utility, utility.getIndexPath(), assetsPath, false);
        }

        if (!utility.getLibPath().isEmpty()) {
            pathToFile(utility, utility.getLibPath(), LIBS_DIR_NAME, utility.isCompressNativeLibs());
        }

        if (!utility.getFilePath().isEmpty()) {
            pathToFile(utility, utility.getFilePath(), NULL_DIR_NAME, false);
        }

        if (!utility.getResPath().isEmpty() && !utility.getModuleName().isEmpty()) {
            String resPath = ASSETS_DIR_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR
                    + RESOURCES_DIR_NAME;
            String deviceTypes = utility.getDeviceType().replace("\"", "").trim();
            if (DEVICE_TYPE_FITNESSWATCH.equals(deviceTypes) ||
                    DEVICE_TYPE_FITNESSWATCH_NEW.equals(deviceTypes)) {
                resPath = RES_DIR_NAME;
            }
            pathToFile(utility, utility.getResPath(), resPath, false);
        }

        if (!utility.getResourcesPath().isEmpty() && !utility.getModuleName().isEmpty()) {
            String resourcesPath = ASSETS_DIR_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR
                    + RESOURCES_DIR_NAME;
            pathToFile(utility, utility.getResourcesPath(), resourcesPath, false);
        }

        if (!utility.getRpcidPath().isEmpty()) {
            String rpcidPath = NULL_DIR_NAME;
            pathToFile(utility, utility.getRpcidPath(), rpcidPath, false);
        }

        if (!utility.getPackInfoPath().isEmpty()) {
            String packInfoPath = NULL_DIR_NAME;
            pathToFile(utility, utility.getPackInfoPath(), packInfoPath, false);
        }

        if (!utility.getAssetsPath().isEmpty()) {
            pathToFile(utility, utility.getAssetsPath(), ASSETS_DIR_NAME, false);
        }

        if (!utility.getBinPath().isEmpty()) {
            pathToFile(utility, utility.getBinPath(), NULL_DIR_NAME, false);
        }
        // pack --dir-list
        if (!utility.getFormatedDirList().isEmpty()) {
            for (int i = 0; i < utility.getFormatedDirList().size(); ++i) {
                String baseDir = new File(utility.getFormatedDirList().get(i)).getName() + File.separator;
                pathToFile(utility, utility.getFormatedDirList().get(i), baseDir, false);
            }
        }

        compressHapModeMultiple(utility);
    }

    /**
     * compress in hap mode for module.json.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressHapModeForModule(Utility utility) throws BundleException {
        pathToFile(utility, utility.getJsonPath(), NULL_DIR_NAME, false);

        pathToFile(utility, utility.getProfilePath(), NULL_DIR_NAME, false);

        if (!utility.getIndexPath().isEmpty() && isModuleJSON(utility.getJsonPath())) {
            String assetsPath = NULL_DIR_NAME;
            pathToFile(utility, utility.getIndexPath(), assetsPath, false);
        }

        if (!utility.getLibPath().isEmpty()) {
            pathToFile(utility, utility.getLibPath(), LIBS_DIR_NAME, utility.isCompressNativeLibs());
        }

        if (!utility.getANPath().isEmpty()) {
            pathToFile(utility, utility.getANPath(), AN_DIR_NAME, false);
        }

        if (!utility.getAPPath().isEmpty()) {
            pathToFile(utility, utility.getAPPath(), AP_PATH_NAME, false);
        }

        if (!utility.getFilePath().isEmpty()) {
            pathToFile(utility, utility.getFilePath(), NULL_DIR_NAME, false);
        }

        if (!utility.getResPath().isEmpty() && !utility.getModuleName().isEmpty()) {
            String resPath = ASSETS_DIR_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR
                    + RESOURCES_DIR_NAME;
            String deviceTypes = utility.getDeviceType().replace("\"", "").trim();
            if (DEVICE_TYPE_FITNESSWATCH.equals(deviceTypes) ||
                    DEVICE_TYPE_FITNESSWATCH_NEW.equals(deviceTypes)) {
                resPath = RES_DIR_NAME;
            }
            pathToFile(utility, utility.getResPath(), resPath, false);
        }

        if (!utility.getResourcesPath().isEmpty() && isModuleJSON(utility.getJsonPath())) {
            String resourcesPath = RESOURCES_DIR_NAME;
            pathToFile(utility, utility.getResourcesPath(), resourcesPath, false);
        }
        if (!utility.getJsPath().isEmpty() && isModuleJSON(utility.getJsonPath())) {
            String jsPath = JS_PATH;
            pathToFile(utility, utility.getJsPath(), jsPath, false);
        }

        if (!utility.getEtsPath().isEmpty() && isModuleJSON(utility.getJsonPath())) {
            String etsPath = ETS_PATH;
            pathToFile(utility, utility.getEtsPath(), etsPath, false);
        }

        if (!utility.getHnpPath().isEmpty() && isModuleJSON(utility.getJsonPath())) {
            String hnpPath = HNP_PATH;
            pathToFile(utility, utility.getHnpPath(), hnpPath, false);
        }

        if (!utility.getRpcidPath().isEmpty()) {
            String rpcidPath = NULL_DIR_NAME;
            pathToFile(utility, utility.getRpcidPath(), rpcidPath, false);
        }

        if (!utility.getAssetsPath().isEmpty()) {
            pathToFile(utility, utility.getAssetsPath(), ASSETS_DIR_NAME, false);
        }

        if (!utility.getBinPath().isEmpty()) {
            pathToFile(utility, utility.getBinPath(), NULL_DIR_NAME, false);
        }

        if (!utility.getPackInfoPath().isEmpty()) {
            pathToFile(utility, utility.getPackInfoPath(), NULL_DIR_NAME, false);
        }

        // pack --dir-list
        if (!utility.getFormatedDirList().isEmpty()) {
            for (int i = 0; i < utility.getFormatedDirList().size(); ++i) {
                String baseDir = new File(utility.getFormatedDirList().get(i)).getName() + File.separator;
                pathToFile(utility, utility.getFormatedDirList().get(i), baseDir, false);
            }
        }
        if (!utility.getPkgContextPath().isEmpty()) {
            pathToFile(utility, utility.getPkgContextPath(), NULL_DIR_NAME, false);
        }

        compressHapModeMultiple(utility);
    }

    /**
     * compress in hap mode multiple path.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressHapModeMultiple(Utility utility) throws BundleException {
        for (String soPathItem : utility.getFormattedSoPathList()) {
            pathToFile(utility, soPathItem, SO_ARM64_DIR_NAME, false);
        }

        if (utility.getFormattedSoPathList().size() == 0 && !utility.getSoDir().isEmpty()) {
            pathToFile(utility, utility.getSoDir(), SO_DIR_NAME, false);
        }

        for (String soPathItem : utility.getFormattedAbilitySoPathList()) {
            pathToFile(utility, soPathItem, NULL_DIR_NAME, false);
        }

        for (String dexPathItem : utility.getFormattedDexPathList()) {
            pathToFile(utility, dexPathItem, NULL_DIR_NAME, false);
        }

        for (String abcPathItem : utility.getFormattedAbcPathList()) {
            pathToFile(utility, abcPathItem, NULL_DIR_NAME, false);
        }

        for (String apkPathItem : utility.getFormattedApkPathList()) {
            pathToFile(utility, apkPathItem, NULL_DIR_NAME, false);
        }

        for (String jarPathItem : utility.getFormattedJarPathList()) {
            pathToFile(utility, jarPathItem, NULL_DIR_NAME, false);
        }

        for (String txtPathItem : utility.getFormattedTxtPathList()) {
            pathToFile(utility, txtPathItem, NULL_DIR_NAME, false);
        }

        if (!utility.getSharedLibsPath().isEmpty()) {
            pathToFile(utility, utility.getSharedLibsPath(), SHARED_LIBS_DIR_NAME, utility.isCompressNativeLibs());
        }
    }

    /**
     * compress in har mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressHarMode(Utility utility) throws BundleException {
        pathToFile(utility, utility.getJsonPath(), NULL_DIR_NAME, false);

        if (!utility.getLibPath().isEmpty()) {
            pathToFile(utility, utility.getLibPath(), LIBS_DIR_NAME, utility.isCompressNativeLibs());
        }

        if (!utility.getResPath().isEmpty()) {
            pathToFile(utility, utility.getResPath(), RESOURCES_DIR_NAME, false);
        }

        if (!utility.getResourcesPath().isEmpty()) {
            pathToFile(utility, utility.getResourcesPath(), RESOURCES_DIR_NAME, false);
        }

        if (!utility.getAssetsPath().isEmpty()) {
            pathToFile(utility, utility.getAssetsPath(), ASSETS_DIR_NAME, false);
        }

        for (String jarPathItem : utility.getFormattedJarPathList()) {
            pathToFile(utility, jarPathItem, NULL_DIR_NAME, false);
        }

        for (String txtPathItem : utility.getFormattedTxtPathList()) {
            pathToFile(utility, txtPathItem, NULL_DIR_NAME, false);
        }
    }

    /**
     * compress in app mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressAppMode(Utility utility) throws BundleException {
        List<String> fileList = new ArrayList<>();
        File appOutputFile = new File(utility.getOutPath().trim());
        String tempPath = appOutputFile.getParentFile().getParent() + File.separator + TEMP_HAP_DIR;
        String hspTempDirPath = appOutputFile.getParentFile().getParent() + File.separator + TEMP_HSP_DIR;
        try {
            pathToFile(utility, utility.getJsonPath(), NULL_DIR_NAME, false);

            if (!utility.getCertificatePath().isEmpty()) {
                pathToFile(utility, utility.getCertificatePath(), NULL_DIR_NAME, false);
            }

            if (!utility.getSignaturePath().isEmpty()) {
                pathToFile(utility, utility.getSignaturePath(), NULL_DIR_NAME, false);
            }

            File tempDir = new File(tempPath);
            if (!tempDir.exists()) {
                tempDir.mkdirs();
            }

            for (String hapPathItem : utility.getFormattedHapPathList()) {
                File hapFile = new File(hapPathItem.trim());
                String hapTempPath = tempDir + File.separator + hapFile.getName();
                fileList.add(hapTempPath);
                try {
                    compressPackinfoIntoHap(hapPathItem, hapTempPath, utility.getPackInfoPath(),
                            utility.getCompressLevel());
                } catch (IOException e) {
                    LOG.error(PackingToolErrMsg.COMPRESS_APP_IO_EXCEPTION.toString(
                            "Compress pack.info into hap exist IOException: " + e.getMessage()));
                    throw new BundleException("Compress pack.info into hap failed.");
                }
            }

            File hspTempDir = new File(hspTempDirPath);
            if (!hspTempDir.exists()) {
                hspTempDir.mkdirs();
            }
            for (String hspPathItem : utility.getFormattedHspPathList()) {
                File hspFile = new File(hspPathItem.trim());
                String hspTempPath = hspTempDir + File.separator + hspFile.getName();
                fileList.add(hspTempPath);
                try {
                    compressPackinfoIntoHap(hspPathItem, hspTempPath, utility.getPackInfoPath(),
                            utility.getCompressLevel());
                } catch (IOException e) {
                    LOG.error(PackingToolErrMsg.COMPRESS_APP_IO_EXCEPTION.toString(
                            "Compress pack.info into hsp exist IOException: " + e.getMessage()));
                    throw new BundleException("Compress pack.info into hsp failed.");
                }
            }
            // check hap is valid
            if (!checkHapIsValid(fileList, utility.getSharedApp())) {
                throw new BundleException("Verify failed when compress app.");
            }

            for (String hapPath : fileList) {
                HapVerifyInfo hapVerifyInfo = hapVerifyInfoMap.get(getFileNameByPath(hapPath));
                if (hapVerifyInfo != null && !hapVerifyInfo.isDebug()) {
                    pathToFile(utility, hapPath, NULL_DIR_NAME, true);
                } else {
                    pathToFile(utility, hapPath, NULL_DIR_NAME, false);
                }
            }

            if (!utility.getEntryCardPath().isEmpty()) {
                String entryCardPath = ENTRYCARD_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR
                        + ENTRYCARD_BASE_NAME + ENTRYCARD_SNAPSHOT_NAME;
                for (String entryCardPathItem : utility.getformattedEntryCardPathList()) {
                    pathToFile(utility, entryCardPathItem, entryCardPath, true);
                }
            }

            if (!utility.getPackResPath().isEmpty()) {
                pathToFile(utility, utility.getPackResPath(), NULL_DIR_NAME, false);
            }
            File file = new File(utility.getPackInfoPath());
            compressFile(utility, file, NULL_DIR_NAME, false);
            // pack encrypt.json file
            packEncryptJsonFile(utility);
            // pack pac.json file
            packPacJsonFile(utility);
        } catch (BundleException e) {
            LOG.error(PackingToolErrMsg.COMPRESS_APP_FAILED.toString(
          "Compress app file exist BundleException: " + e.getMessage()));
            throw new BundleException("Compress app failed.");
        } finally {
            // delete temp file
            for (String path : fileList) {
                deleteFile(path);
            }
            deleteFile(tempPath);
            deleteFile(hspTempDirPath);
        }
    }

    private void compressFastAppMode(Utility utility) throws BundleException {
        Path appOutPath = Paths.get(utility.getOutPath().trim());
        Path tmpDir = null;
        try {
            tmpDir = Files.createTempDirectory(appOutPath.getParent(), TEMP_DIR);
            Path appPackInfo = Paths.get(utility.getPackInfoPath());
            List<String> fileList = new ArrayList<>();
            for (String hapPath : utility.getFormattedHapPathList()) {
                Path path = Paths.get(hapPath);
                Path hap = PackageUtil.pack(path, appPackInfo, tmpDir, utility.getCompressLevel());
                if (hap != null) {
                    fileList.add(hap.toString());
                }
            }
            for (String hspPath : utility.getFormattedHspPathList()) {
                Path path = Paths.get(hspPath);
                Path hsp = PackageUtil.pack(path, appPackInfo, tmpDir, utility.getCompressLevel());
                if (hsp != null) {
                    fileList.add(hsp.toString());
                }
            }
            // check hap is valid
            if (!checkHapIsValid(fileList, utility.getSharedApp())) {
                throw new BundleException("Verify failed when compress fast app.");
            }
            // packApp
            packFastApp(utility, fileList);
        } catch (IOException ex) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Compress fast app exist IOException: " +
                    ex.getMessage()));
            throw new BundleException("Compressor::compressAppMode compress failed.");
        } finally {
            if (tmpDir != null) {
                PackageUtil.rmdir(tmpDir);
            }
        }
    }

    private void packFastApp(Utility utility, List<String> fileList) throws BundleException {
        // pack.info
        pathToFile(utility, utility.getPackInfoPath(), NULL_DIR_NAME, false);
        // pack encrypt.json file
        packEncryptJsonFile(utility);
        // pack pac.json file
        packPacJsonFile(utility);
        // hap/hsp
        for (String hapPath : fileList) {
            HapVerifyInfo hapVerifyInfo = hapVerifyInfoMap.get(getFileNameByPath(hapPath));
            if (hapVerifyInfo != null && !hapVerifyInfo.isDebug()) {
                pathToFile(utility, hapPath, NULL_DIR_NAME, true);
            } else {
                pathToFile(utility, hapPath, NULL_DIR_NAME, false);
            }
        }
        // form/card
        if (!utility.getEntryCardPath().isEmpty()) {
            String entryCardPath = ENTRYCARD_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR
                    + ENTRYCARD_BASE_NAME + ENTRYCARD_SNAPSHOT_NAME;
            for (String entryCardPathItem : utility.getformattedEntryCardPathList()) {
                pathToFile(utility, entryCardPathItem, entryCardPath, true);
            }
        }
        if (!utility.getPackResPath().isEmpty()) {
            pathToFile(utility, utility.getPackResPath(), NULL_DIR_NAME, false);
        }
        // others
        if (!utility.getCertificatePath().isEmpty()) {
            pathToFile(utility, utility.getCertificatePath(), NULL_DIR_NAME, false);
        }
        if (!utility.getSignaturePath().isEmpty()) {
            pathToFile(utility, utility.getSignaturePath(), NULL_DIR_NAME, false);
        }
    }

    /**
     * compress in app mode for multi project.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressAppModeForMultiProject(Utility utility) throws BundleException {
        List<String> fileList = new ArrayList<>();
        File appOutputFile = new File(utility.getOutPath().trim());
        String tempPath = appOutputFile.getParentFile().getParent() + File.separator + TEMP_HAP_DIR;
        String tempSelectedHapPath = appOutputFile.getParentFile().getParent() +File.separator + TEMP_SELECTED_HAP_DIR;
        try {
            File tempSelectedHapDir = new File(tempSelectedHapPath);
            FileUtils.makeDir(tempSelectedHapDir);
            File tempHapDir = new File(tempPath);
            FileUtils.makeDir(tempHapDir);
            // pack app and dispose conflict
            // save hap name into list
            List<String> seletedHaps = new ArrayList<>();
            String finalPackInfoStr = disposeApp(utility, seletedHaps, tempSelectedHapPath);
            // pack hap and dispose conflict
            finalPackInfoStr = disposeHap(utility, seletedHaps, tempSelectedHapPath, finalPackInfoStr);

            // save final pack.info file
            String finalPackInfoPath = tempSelectedHapDir.getPath() + File.separator + PACKINFO_NAME;
            writePackInfo(finalPackInfoPath, finalPackInfoStr);
            // pack haps
            for (String selectedHapName : seletedHaps) {
                String hapPathItem = tempSelectedHapDir.getPath() + File.separator + selectedHapName;
                File hapFile = new File(hapPathItem.trim());
                String hapTempPath = tempHapDir.getPath() + File.separator + hapFile.getName();
                fileList.add(hapTempPath);
                compressPackinfoIntoHap(hapPathItem, hapTempPath, finalPackInfoPath, utility.getCompressLevel());
            }
            // check hap is valid
            if (!checkHapIsValid(fileList, false)) {
                String errMsg = "Compressor::compressAppModeForMultiProject There are some " +
                        "haps with different version code or duplicated moduleName or packageName.";
                throw new BundleException(errMsg);
            }
            for (String hapPath : fileList) {
                pathToFile(utility, hapPath, NULL_DIR_NAME, false);
            }
            File file = new File(finalPackInfoPath);
            compressFile(utility, file, NULL_DIR_NAME, false);
            //pack encrypt.json file
            packEncryptJsonFile(utility);
            // pack pac.json file
            packPacJsonFile(utility);
        } catch (BundleException | IOException exception) {
            String errMsg = "Compress app mode for multi project file exist Exception (BundleException | IOException): "
                    + exception.getMessage();
            LOG.error(PackingToolErrMsg.COMPRESS_APP_MODE_FORMULTI_PROJECT_FAILED.toString(errMsg));
            throw new BundleException(errMsg);
        } finally {
            deleteFile(tempPath);
            deleteFile(tempSelectedHapPath);
        }
    }

    /**
     * compress in hapAddition mode.
     *
     * @param utility common data
     */
    private void hapAddition(Utility utility) {
        File hapPath = new File(utility.getAbsoluteHapPath());
        String hapFileName = hapPath.getName();

        File destFile = new File(utility.getOutPath() + LINUX_FILE_SEPARATOR + hapFileName);
        File outParentFile = destFile.getParentFile();
        if ((outParentFile != null) && (!outParentFile.exists())) {
            if (!outParentFile.mkdirs()) {
                LOG.error(PackingToolErrMsg.HAP_ADDITION_FAILED.toString("Create out file parent directory failed."));
            }
        }
        FileOutputStream fileOut = null;
        CheckedOutputStream checkedOut = null;
        String currentDir = System.getProperty("user.dir");
        String hapAdditionPath = currentDir + LINUX_FILE_SEPARATOR + HAPADDITION_FOLDER_NAME;
        String backName = BACKUP_PREFIX + hapFileName;
        String hapPathOri = utility.getHapPath();
        try {
            copyHapFile(utility, backName);
            fileOut = new FileOutputStream(destFile);
            checkedOut = new CheckedOutputStream(fileOut, new CRC32());
            zipOut = new ZipArchiveOutputStream(checkedOut);

            compressHapAddition(utility, hapAdditionPath);
        } catch (BundleException | IOException exception) {
            LOG.error(PackingToolErrMsg.HAP_ADDITION_FAILED.toString("Hap addition exist Exception " +
                    "(BundleException | IOException): " + exception.getMessage()));
            copyFileSafely(backName, hapPathOri);
        } finally {
            closeZipOutputStream();
            Utility.closeStream(zipOut);
            Utility.closeStream(checkedOut);
            Utility.closeStream(fileOut);
            // delete packaging and unpacking process files
            deleteFile(backName);
            deleteFile(hapAdditionPath);
        }
    }

    private void copyHapFile(Utility utility, String backName) throws IOException, BundleException {
        File hapFile = new File(utility.getAbsoluteHapPath());
        String currentDir = System.getProperty("user.dir");
        String backupPath = currentDir + LINUX_FILE_SEPARATOR + backName;
        File backupFile = new File(backupPath);
        FileUtils.copyFile(hapFile, backupFile);
        utility.setHapPath(backName);
    }

    private void copyFileSafely(String source, String dest) {
        try {
            File sourceFile = new File(source);
            File destFile = new File(dest);
            FileUtils.copyFile(sourceFile, destFile);
        } catch (IOException | BundleException e) {
            LOG.error(PackingToolErrMsg.COPY_FILE_SAFELY_FAILED.toString("Copy file exist Exception " +
                    "(IOException | BundleException): " + e.getMessage()));
        }
    }

    private static String readerFile(String jsonPath) throws IOException {
        byte[] bytes = Files.readAllBytes(Paths.get(jsonPath));
        return new String(bytes, StandardCharsets.UTF_8);
    }

    private static void writeJsonFile(String dataJson, String targetPath) throws BundleException {
        try (FileWriter fileWriter = new FileWriter(targetPath)) {
            Object object = JSON.parse(dataJson);
            String jsonString = new String();
            if (object instanceof JSONArray) {
                JSONArray jsonArray = JSONArray.parseArray(dataJson);
                jsonString = JSON.toJSONString(
                        jsonArray, SerializerFeature.PrettyFormat, SerializerFeature.WriteMapNullValue);
            } else {
                JSONObject jsonObject = JSONObject.parseObject(dataJson);
                jsonString = JSON.toJSONString(
                        jsonObject, SerializerFeature.PrettyFormat, SerializerFeature.WriteMapNullValue);
            }
            fileWriter.write(jsonString);
            fileWriter.flush();
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Write json File exist IOException: " +
                    exception.getMessage()));
            throw new BundleException(exception.getMessage());
        } catch (JSONException e) {
            LOG.error(PackingToolErrMsg.WRITE_JSON_FILE_EXPECTION.toString("Write json file exist JSONException: " +
                    e.getMessage()));
            throw new BundleException(e.getMessage());
        }
    }

    private static void setUtilityParameter(String hapAdditionPath, Utility utility) throws IOException {
        Path basePath = Paths.get(hapAdditionPath);
        try (Stream<Path> pathStream = Files.walk(basePath, 1)) {
            pathStream.forEach(path -> {
                String fileName = path.getFileName().toString();
                String filePath = path.toString();
                switch (fileName) {
                    case ETS_FILE_NAME:
                        utility.setEtsPath(filePath);
                        break;
                    case DIR_FILE_NAME:
                        utility.setLibPath(filePath);
                        break;
                    case AN_FILE_NAME:
                        utility.setANPath(filePath);
                        break;
                    case AP_FILE_NAME:
                        utility.setAPPath(filePath);
                        break;
                    case RESOURCE_FILE_NAME:
                        utility.setResourcesPath(filePath);
                        break;
                    case JS_FILE_NAME:
                        utility.setJsPath(filePath);
                        break;
                    case ASSETS_FILE_NAME:
                        utility.setAssetsPath(filePath);
                        break;
                    case MAPLE_FILE_NAME:
                        utility.setSoDir(filePath);
                        break;
                    case SHARED_LIBS_FILE_NAME:
                        utility.setSharedLibsPath(filePath);
                        break;
                    case MODULE_JSON:
                        utility.setJsonPath(filePath);
                        break;
                    case RES_INDEX:
                        utility.setIndexPath(filePath);
                        break;
                    case PACKINFO_NAME:
                        utility.setPackInfoPath(filePath);
                        break;
                }
            });
        }
    }

    private void compressHapAddition(Utility utility, String hapAdditionPath) throws BundleException {
        // decompression hap file to hapAddition

        unpackHap(utility.getHapPath(), hapAdditionPath);

        // generate addition.json file
        try {
            String data = readerFile(utility.getJsonPath());
            String currentDir = System.getProperty("user.dir");
            String targetParentPath = currentDir + LINUX_FILE_SEPARATOR + TARGET_FILE_PATH;
            File targetParent = new File(targetParentPath);
            if (!targetParent.exists()) {
                if (!targetParent.mkdirs()) {
                    LOG.error(PackingToolErrMsg.COMPRESS_HAP_ADDITION_FAILED.toString(
                            "Create target file parent directory failed."));
                }
            }
            String targetPath = targetParentPath + LINUX_FILE_SEPARATOR + ADDITION_JSON;
            writeJsonFile(data, targetPath);
        } catch (IOException | JSONException | BundleException e) {
            String errMsg = "Generate addition.json file exist Exception" +
                    " (IOException | JSONException | BundleException): " + e.getMessage();
            LOG.error(PackingToolErrMsg.COMPRESS_HAP_ADDITION_FAILED.toString(errMsg));
            throw new BundleException(errMsg);
        }

        // package a new hap file
        try {
            setUtilityParameter(hapAdditionPath, utility);
        } catch (IOException e) {
            String errMsg = "Set utility parameter exist IOException: " + e.getMessage();
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(errMsg));
            throw new BundleException(errMsg);
        }
        if (utility.getHapPath().endsWith(HAP_SUFFIX)) {
            compressHap(utility);
        } else if (utility.getHapPath().endsWith(HSP_SUFFIX)) {
            compressHsp(utility);
        } else {
            String errMsg = "Compression add failed because file (" + utility.getHapPath() +
                    ") is an unsupported file type.";
            LOG.error(PackingToolErrMsg.COMPRESS_HAP_ADDITION_FAILED.toString(errMsg));
            throw new BundleException(errMsg);
        }
    }

    /**
     * pack hap in app to selectedHaps
     *
     * @param utility is common data
     * @param seletedHaps is seleted haps should be pack into app
     * @return the final pack.info string after dispose app
     * @throws BundleException FileNotFoundException|IOException.
     */
    private static String disposeApp(Utility utility, List<String> seletedHaps,
                                     String tempDir) throws BundleException {
        // dispose app conflict
        if (utility.getFormattedAppList().isEmpty()) {
            return "";
        }
        String finalAppPackInfo = "";
        try {
            for (String appPath : utility.getFormattedAppList()) {
                // select hap in app
                finalAppPackInfo = selectHapInApp(appPath, seletedHaps, tempDir, finalAppPackInfo);
            }
        } catch (BundleException | IOException e) {
            String errMsg = "Dispose app exist Exception (BundleException | IOException): " + e.getMessage();
            LOG.error(PackingToolErrMsg.DISPOSE_APP_FAILED.toString(errMsg));
            throw new BundleException(errMsg);
        }
        return finalAppPackInfo;
    }

    /**
     * select hap from app file list
     *
     * @param appPath is common data
     * @param selectedHaps is list of packInfos
     * @throws BundleException FileNotFoundException|IOException.
     */
    private static String selectHapInApp(String appPath, List<String> selectedHaps, String tempDir,
                                         String finalAppPackInfo) throws BundleException, IOException {
        List<String> selectedHapsInApp = new ArrayList<>();
        // classify hap in app
        copyHapAndHspFromApp(appPath, selectedHapsInApp, selectedHaps, tempDir);
        // rebuild pack.info
        String packInfoStr = FileUtils.getJsonInZips(new File(appPath), PACKINFO_NAME);
        if (packInfoStr.isEmpty()) {
            String errorMsg = "Select hap from app find that app does not have pack.info.";
            LOG.error(PackingToolErrMsg.NO_PACK_INFO.toString(errorMsg));
            throw new BundleException(errorMsg);
        }
        if (finalAppPackInfo.isEmpty()) {
            finalAppPackInfo = packInfoStr;
            return finalAppPackInfo;
        }
        // read selected module in temp hap
        HashMap<String, String> packagePair = new HashMap<>();
        for (String hapName : selectedHapsInApp) {
            packagePair.put(hapName, readModlueNameFromHap(tempDir + File.separator + hapName));
        }
        return ModuleJsonUtil.mergeTwoPackInfoByPackagePair(finalAppPackInfo, packInfoStr, packagePair);
    }

    /**
     * copy hap from app file
     *
     * @param appPath is common data
     * @param selectedHapsInApp is list of haps and hsps selected in app file
     * @param selectedHaps is the list of haps and hsps selected in input
     * @throws BundleException FileNotFoundException|IOException.
     */
    private static void copyHapAndHspFromApp(String appPath, List<String> selectedHapsInApp, List<String> selectedHaps,
                                             String tempDir) throws BundleException {
        ZipInputStream zipInput = null;
        ZipFile zipFile = null;
        OutputStream outputStream = null;
        InputStream inputStream = null;
        ZipEntry zipEntry = null;
        try {
            zipInput = new ZipInputStream(new FileInputStream(appPath));
            zipFile = new ZipFile(appPath);
            while ((zipEntry = zipInput.getNextEntry()) != null) {
                File file = null;
                if (!zipEntry.getName().endsWith(HAP_SUFFIX) && !zipEntry.getName().endsWith(HSP_SUFFIX)) {
                    continue;
                }
                // copy duplicated hap to duplicated dir and get moduleName of duplicated hap
                if (selectedHaps.contains(zipEntry.getName())) {
                    LOG.error(PackingToolErrMsg.COMPRESS_FILE_DUPLICATE.toString("Copy hap from app file exist hap " +
                            "with duplicate file names. file is " + zipEntry.getName() + "."));
                    throw new BundleException("Compressor::copyHapFromApp file duplicated, file is "
                            + zipEntry.getName() + ".");
                } else {
                    // copy selectedHap to tempDir
                    file = new File(tempDir + File.separator + zipEntry.getName());
                    selectedHaps.add(file.getName());
                    selectedHapsInApp.add(file.getName());
                }
                outputStream = new FileOutputStream(file);
                inputStream = zipFile.getInputStream(zipEntry);
                int len;
                byte[] buf = new byte[BUFFER_SIZE];
                while ((len = inputStream.read(buf)) != -1) {
                    outputStream.write(buf, 0, len);
                }
                outputStream.close();
                inputStream.close();
            }
        } catch (IOException e) {
            String errMsg = "Copy hap and hsp from app exist IOException: " + e.getMessage();
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(errMsg));
            throw new BundleException(errMsg);
        } finally {
            Utility.closeStream(zipInput);
            Utility.closeStream(zipFile);
            Utility.closeStream(outputStream);
            Utility.closeStream(inputStream);
        }
    }

    /**
     * read moduleName in hap
     *
     * @param hapPath is path of hap file
     * @throws BundleException FileNotFoundException|IOException.
     */
    private static String readModlueNameFromHap(String hapPath) throws BundleException {
        String moduleName = "";
        File hapFile = new File(hapPath);
        if (isModuleHap(hapPath)) {
            String jsonString = FileUtils.getJsonInZips(hapFile, MODULE_JSON);
            moduleName = ModuleJsonUtil.parseStageModuleName(jsonString);
        } else {
            String jsonString = FileUtils.getJsonInZips(hapFile, CONFIG_JSON);
            moduleName = ModuleJsonUtil.parseFaModuleName(jsonString);
        }
        return moduleName;
    }

    /**
     * dispose input of hap
     *
     * @param utility is common data
     * @param seletedHaps is the selected  haps of all input
     * @param tempDir is the path of temp directory
     * @param finalPackInfoStr is the pack.info of the final app
     * @throws BundleException FileNotFoundException|IOException.
     */
    private static String disposeHap(Utility utility, List<String> seletedHaps, String tempDir,
                                     String finalPackInfoStr) throws BundleException, IOException {
        // dispose hap conflict
        if (utility.getFormattedHapList().isEmpty()) {
            return finalPackInfoStr;
        }
        for (String hapPath : utility.getFormattedHapList()) {
            if (seletedHaps.contains(new File(hapPath).getName())) {
                LOG.error(PackingToolErrMsg.COMPRESS_FILE_DUPLICATE.toString("Dispose hap exist duplicate file " +
                        "names, file is " + new File(hapPath).getName() + "."));
                throw new BundleException("Compressor::disposeHap file duplicated, file is "
                        + new File(hapPath).getName() + ".");
            }
            File hapFile = new File(hapPath);
            seletedHaps.add(hapFile.getName());
            // copy hap to tempDir
            FileUtils.copyFile(hapFile, new File((tempDir +File.separator + hapFile.getName())));
            String packInfo = FileUtils.getJsonInZips(hapFile, PACKINFO_NAME);

            if (packInfo.isEmpty()) {
                String errMsg = "Hap does not have pack.info.";
                LOG.error(PackingToolErrMsg.NO_PACK_INFO.toString(errMsg));
                throw new BundleException(errMsg);
            }
            if (finalPackInfoStr.isEmpty()) {
                finalPackInfoStr = packInfo;
            } else {
                finalPackInfoStr = ModuleJsonUtil.mergeTwoPackInfo(finalPackInfoStr, packInfo);
            }
        }
        return finalPackInfoStr;
    }

    /**
     * write string to pack.info
     *
     * @param filePath pack.info file path
     * @param packInfoStr is the string of pack.info
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void writePackInfo(String filePath, String packInfoStr) throws BundleException, IOException {
        FileWriter fwriter = null;
        try {
            fwriter = new FileWriter(filePath);
            fwriter.write(packInfoStr);
        } catch (IOException e) {
            String errMsg = "Write pack info exist IOException: " + e.getMessage();
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(errMsg));
            throw new BundleException(errMsg);
        } finally {
            if (fwriter != null) {
                fwriter.flush();
                fwriter.close();
            }
        }
    }

    private void copy(InputStream input, OutputStream output) throws IOException {
        int bytesRead;
        byte[] data = new byte[BUFFER_SIZE];
        while ((bytesRead = input.read(data, 0, BUFFER_SIZE)) != -1) {
            output.write(data, 0, bytesRead);
        }
    }

    private void compressPackinfoIntoHap(String hapPathItem, String outPathString, String packInfo, int compressLevel)
            throws IOException, BundleException {
        ZipFile sourceHapFile = new ZipFile(hapPathItem);
        ZipOutputStream append = new ZipOutputStream(new FileOutputStream(outPathString));
        append.setLevel(compressLevel);
        try {
            Enumeration<? extends ZipEntry> entries = sourceHapFile.entries();
            while (entries.hasMoreElements()) {
                ZipEntry zipEntry = new ZipEntry(entries.nextElement());
                if (PACKINFO_NAME.equals(zipEntry.getName())) {
                    continue;
                }
                ZipEntry newEntry = zipEntry.getMethod() ==
                        ZipEntry.STORED ? new ZipEntry(zipEntry) : new ZipEntry(zipEntry.getName());
                append.putNextEntry(newEntry);
                if (!zipEntry.isDirectory()) {
                    copy(sourceHapFile.getInputStream(zipEntry), append);
                }
                append.closeEntry();
            }
            File packInfoFile = new File(packInfo);
            ZipEntry zipEntry = getStoredZipEntry(packInfoFile, PACKINFO_NAME);
            append.putNextEntry(zipEntry);
            FileInputStream in = new FileInputStream(packInfoFile);
            try {
                byte[] buf = new byte[BUFFER_SIZE];
                int len;
                while ((len = in.read(buf)) != -1) {
                    append.write(buf, 0, len);
                }
            } finally {
                in.close();
            }
            append.closeEntry();
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.COMPRESS_FILE_EXCEPTION.toString(
                    "Compress pack.info into hap exist IOException: " + exception.getMessage()));
            throw new BundleException("Compress PackInfo into hap IOException.");
        } finally {
            sourceHapFile.close();
            append.close();
        }
    }

    /**
     * delete file
     *
     * @param path file path which will be deleted
     */
    private static void deleteFile(final String path) {
        File file = new File(path);
        if (file.exists()) {
            if (file.isDirectory()) {
                File[] files = file.listFiles();
                for (int i = 0; i < files.length; i++) {
                    deleteFile(files[i].toString());
                }
            }
            file.delete();
        }
    }

    /**
     * compress in res mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressPackResMode(Utility utility) throws BundleException {
        if (!utility.getPackInfoPath().isEmpty()) {
            parsePackInfoJsonFile(utility, utility.getPackInfoPath());
            File file = new File(utility.getPackInfoPath());
            infoSpecialProcess(utility, file);
        }
        if (!utility.getEntryCardPath().isEmpty()) {
            getFileList(utility.getEntryCardPath());
            List<String> snapshotNameList = new ArrayList<>();
            for (String fileName : fileNameList) {
                if (fileName.endsWith(PNG_SUFFIX) || fileName.endsWith(UPPERCASE_PNG_SUFFIX)) {
                    String fName = fileName.trim();
                    String[] temp = fName.replace("\\", "/").split("/");
                    if (temp.length < 4) {
                        LOG.error(PackingToolErrMsg.COMPRESS_PACK_RES_MODE_FAILED.toString("The hap file path is "
                                + "invalid, length is " + temp.length + ", expected at least 4."));
                        continue;
                    }
                    String fileLanguageCountryName = temp[temp.length - 3];
                    if (!isThirdLevelDirectoryNameValid(fileLanguageCountryName)) {
                        LOG.error(PackingToolErrMsg.COMPRESS_PACK_RES_MODE_FAILED.toString("Third level "
                                + "directory name: " + fileLanguageCountryName + " is invalid, please check it with "
                                + "reference to this example: zh_Hani_CN-vertical-car-mdpi-dark or "
                                + "zh_Hani_CN-vertical-car-mdpi."));
                        throw new BundleException("Compress failed third level directory name Error.");
                    }
                    String filePicturingName = temp[temp.length - 1];
                    if (!isPicturing(filePicturingName, utility)) {
                        LOG.error(PackingToolErrMsg.COMPRESS_PACK_RES_MODE_FAILED.toString("Invalid resource file"
                                + " name: " + filePicturingName + ", correct format example is formName-2x2.png."));
                        throw new BundleException("Compress pack.res failed, Invalid resource file name: "
                                + filePicturingName + ", correct format example is formName-2x2.png.");
                    }

                    String fullSnapshotName = temp[temp.length - 4] + "/" +
                            filePicturingName.substring(0, filePicturingName.lastIndexOf("."));
                    snapshotNameList.add(fullSnapshotName);

                } else {
                    LOG.error(PackingToolErrMsg.COMPRESS_PACK_RES_MODE_FAILED.toString("No PNG format image found."));
                    throw new BundleException("Compress pack.res failed, compress failed No image in"
                            + " PNG format is found.");
                }
            }

            for (String formName : formNamesList) {
                if (!snapshotNameList.contains(formName)) {
                    LOG.error(PackingToolErrMsg.COMPRESS_PACK_RES_MODE_FAILED.toString("EntryCard has no related "
                            + "snapshot. " + formName + " has no related snapshot "
                            + snapshotNameList.toString() + "."));
                    throw new BundleException("Compress pack.res failed, compress failed entryCard has no related snapshot.");
                }
            }
            pathToFileResMode(utility, utility.getEntryCardPath(), ENTRYCARD_NAME, false);
        }
    }

    /**
     * Check whether modelname meets specifications.
     *
     * @param name modelName
     * @return false and true
     */
    private boolean isModelName(String name) {
        for (String listName : list) {
            if (name.equals(listName)) {
                return true;
            }
        }
        return false;
    }

    private boolean isThirdLevelDirectoryNameValid(String thirdLevelDirectoryName) {
        if (thirdLevelDirectoryName == null || thirdLevelDirectoryName.isEmpty()) {
            return false;
        }
        if (ENTRYCARD_BASE_NAME.equals(thirdLevelDirectoryName)) {
            return true;
        }
        // example: zh_Hani_CN-vertical-car-mdpi-dark or zh_Hani_CN-vertical-car-mdpi
        int firstDelimiterIndex = thirdLevelDirectoryName.indexOf("_");
        if (firstDelimiterIndex < 0) {
            return false;
        }
        String language = thirdLevelDirectoryName.substring(0, firstDelimiterIndex);
        int secondDelimiterIndex = thirdLevelDirectoryName.indexOf("_", firstDelimiterIndex + 1);
        if (secondDelimiterIndex < 0) {
            return false;
        }
        String script = thirdLevelDirectoryName.substring(firstDelimiterIndex + 1, secondDelimiterIndex);
        int thirdDelimiterIndex = thirdLevelDirectoryName.indexOf("-", secondDelimiterIndex + 1);
        if (thirdDelimiterIndex < 0) {
            return false;
        }
        String country = thirdLevelDirectoryName.substring(secondDelimiterIndex + 1, thirdDelimiterIndex);
        if (!checkLanguage(language) || !checkScript(script) || !checkCountry(country)) {
            return false;
        }
        int forthDelimiterIndex = thirdLevelDirectoryName.indexOf("-", thirdDelimiterIndex + 1);
        if (forthDelimiterIndex < 0) {
            return false;
        }
        String orientation = thirdLevelDirectoryName.substring(thirdDelimiterIndex + 1, forthDelimiterIndex);
        int fifthDelimiterIndex = thirdLevelDirectoryName.indexOf("-", forthDelimiterIndex + 1);
        if (fifthDelimiterIndex < 0) {
            return false;
        }
        String deviceType = thirdLevelDirectoryName.substring(forthDelimiterIndex + 1, fifthDelimiterIndex);
        if (!checkOrientation(orientation) || !checkDeviceType(deviceType)) {
            return false;
        }
        int sixthDelimiterIndex = thirdLevelDirectoryName.indexOf("-", fifthDelimiterIndex + 1);
        if (sixthDelimiterIndex < 0) {
            String screenDensity = thirdLevelDirectoryName.substring(fifthDelimiterIndex + 1,
                    thirdLevelDirectoryName.length());
            return checkScreenDensity(screenDensity);
        } else {
            String screenDensity = thirdLevelDirectoryName.substring(fifthDelimiterIndex + 1, sixthDelimiterIndex);
            if (!checkScreenDensity(screenDensity)) {
                return false;
            }
        }
        int seventhDelimiterIndex = thirdLevelDirectoryName.indexOf("-", sixthDelimiterIndex + 1);
        if (seventhDelimiterIndex < 0) {
            String tmp = thirdLevelDirectoryName.substring(sixthDelimiterIndex + 1, thirdLevelDirectoryName.length());
            return checkColorModeOrShape(tmp);
        }
        if (!checkColorMode(thirdLevelDirectoryName.substring(sixthDelimiterIndex + 1, seventhDelimiterIndex))) {
            return false;
        }
        String shape = thirdLevelDirectoryName.substring(seventhDelimiterIndex + 1, thirdLevelDirectoryName.length());
        return checkShape(shape);
    }

    private boolean checkLanguage(String language) {
        if (!Pattern.compile(REGEX_LANGUAGE).matcher(language).matches()) {
            LOG.error(PackingToolErrMsg.INVALID_THIRD_LEVEL_DIRECTORY_NAME.toString("The language " + language +
                    " is not in ISO 639-1 list."));
            return false;
        }
        return true;
    }

    private boolean checkScript(String script) {
        if (!Pattern.compile(REGEX_SCRIPT).matcher(script).matches()) {
            LOG.error(PackingToolErrMsg.INVALID_THIRD_LEVEL_DIRECTORY_NAME.toString("The script " + script +
                    " is not in ISO 15924 list."));
            return false;
        }
        return true;
    }

    private boolean checkCountry(String country) {
        if (!Pattern.compile(REGEX_COUNTRY).matcher(country).matches()) {
            LOG.error(PackingToolErrMsg.INVALID_THIRD_LEVEL_DIRECTORY_NAME.toString("The country " + country +
                    " is not in ISO 3166-1 list."));
            return false;
        }
        return true;
    }

    private boolean checkOrientation(String orientation) {
        if (!Pattern.compile(REGEX_ORIENTATION).matcher(orientation).matches()) {
            LOG.error(PackingToolErrMsg.INVALID_THIRD_LEVEL_DIRECTORY_NAME.toString("The orientation " + orientation +
                    " is not in {vertical, horizontal} list."));
            return false;
        }
        return true;
    }

    private boolean checkDeviceType(String deviceType) {
        if (!Pattern.compile(REGEX_DEVICE_TYPE).matcher(deviceType).matches()) {
            LOG.error(PackingToolErrMsg.INVALID_THIRD_LEVEL_DIRECTORY_NAME.toString("The deviceType " + deviceType +
                    " is not in {phone, tablet, car, tv, wearable, liteWearable, 2in1} list."));
            return false;
        }
        return true;
    }

    private boolean checkScreenDensity(String screenDensity) {
        if (!Pattern.compile(REGEX_SCREEN_DENSITY).matcher(screenDensity).matches()) {
            LOG.error(PackingToolErrMsg.INVALID_THIRD_LEVEL_DIRECTORY_NAME.toString("The screenDensity " +
                    screenDensity + " is not in {sdpi, mdpi, ldpi, xldpi, xxldpi} list."));
            return false;
        }
        return true;
    }

    private boolean checkColorMode(String colorMode) {
        if (!Pattern.compile(REGEX_COLOR_MODE).matcher(colorMode).matches()) {
            LOG.error(PackingToolErrMsg.INVALID_THIRD_LEVEL_DIRECTORY_NAME.toString("The colorMode " + colorMode +
                    " is not in {light, dark} list."));
            return false;
        }
        return true;
    }

    private boolean checkColorModeOrShape(String tmp) {
        if (Pattern.compile(REGEX_COLOR_MODE).matcher(tmp).matches() ||
                Pattern.compile(REGEX_SHAPE).matcher(tmp).matches()) {
            return true;
        }
        LOG.error(PackingToolErrMsg.INVALID_THIRD_LEVEL_DIRECTORY_NAME.toString("The colorMode or shape " + tmp +
                " is neither in colorMode list {light, dark} nor in shape list {circle}."));
        return false;
    }

    private boolean checkShape(String shape) {
        if (Pattern.compile(REGEX_SHAPE).matcher(shape).matches()) {
            return true;
        }
        LOG.error(PackingToolErrMsg.INVALID_THIRD_LEVEL_DIRECTORY_NAME.toString("The shape " + shape +
                " is not in {circle} list."));
        return false;
    }

    /**
     * Check whether picturingName meets specifications.
     *
     * @param name picturingName
     * @param utility common data
     * @return false and true
     */
    private boolean isPicturing(String name, Utility utility) {
        boolean isSpecifications = false;
        if (name == null || name.isEmpty()) {
            return isSpecifications;
        }
        if (!name.endsWith(PNG_SUFFIX) && !name.endsWith(UPPERCASE_PNG_SUFFIX)) {
            LOG.error(PackingToolErrMsg.IS_PICTURING_FAILED.toString("The suffix is not .png or .PNG."));
            return false;
        }
        int delimiterIndex = name.lastIndexOf("-");
        if (delimiterIndex < 0) {
            LOG.error(PackingToolErrMsg.IS_PICTURING_FAILED.toString(
                    "The entry card naming format is invalid and should be separated by '-'."));
            return false;
        }
        String formName = name.substring(0, delimiterIndex);
        if (!utility.getFormNameList().contains(formName)) {
            LOG.error(PackingToolErrMsg.IS_PICTURING_FAILED.toString(
                    "The name is not same as formName, name: " + formName + " is not in "
                    + utility.getFormNameList().toString() + "."));
            return false;
        }
        String dimension = name.substring(delimiterIndex + 1, name.lastIndexOf("."));
        if (!supportDimensionsList.contains(dimension)) {
            LOG.error(PackingToolErrMsg.IS_PICTURING_FAILED.toString("The dimension " + dimension +
                    " is invalid, is not in the following list: {1X2, 2X2, 2X4, 4X4, 1X1, 6X4}."));
            return false;
        }

        return true;
    }

    private void getFileList(final String filePath) throws BundleException {
        File file = new File(filePath);
        if (!file.exists()) {
            LOG.error(PackingToolErrMsg.FILE_NOT_EXIST.toString("File does not exist. File path is "
                    + filePath + "."));
            return;
        }
        File[] files = file.listFiles();
        if (files == null) {
            LOG.error(PackingToolErrMsg.GET_FILE_LIST_FAILED.toString("No files found in the specified path: "
                    + filePath + "."));
            return;
        }
        for (File f : files) {
            try {
                if (f.isFile()) {
                    if (f.getName().endsWith(".DS_Store")) {
                        deleteFile(f.getCanonicalPath());
                        continue;
                    }
                    String snapshotDirectoryName = f.getParentFile().getName();
                    if (!ENTRYCARD_SNAPSHOT_NAME.equals(snapshotDirectoryName)) {
                        LOG.error(PackingToolErrMsg.GET_FILE_LIST_FAILED.toString("The level-4 directory of EntryCard "
                                + "must be named as snapshot, but current is: " + snapshotDirectoryName + "."));
                        throw new BundleException("The level-4 directory of EntryCard must be named as snapshot" +
                                ", but current is: " + snapshotDirectoryName + ".");
                    }
                    fileNameList.add(f.getCanonicalPath());
                } else if (f.isDirectory()) {
                    getFileList(f.getCanonicalPath());
                } else {
                    LOG.error(PackingToolErrMsg.GET_FILE_LIST_FAILED.toString("It's not file or directory."));
                }
            } catch (IOException msg) {
                LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Get file list exist IOException: "
                        + msg.getMessage()));
                return;
            }
        }
    }

    private void checkContain2x2EntryCard(final File snapshotDirectory) throws IOException, BundleException {
        if (!snapshotDirectory.exists()) {
            LOG.error("checkContain2x2EntryCard: file is not exist: " + snapshotDirectory.getName() + ".");
            throw new BundleException("checkContain2x2EntryCard: file is not exist.");
        }
        File[] files = snapshotDirectory.listFiles();
        if (files == null) {
            LOG.error("checkContain2x2EntryCard: no file in this file path.");
            throw new BundleException("checkContain2x2EntryCard: no file in this file path.");
        }

        for (File entryCardFile : files) {
            if (entryCardFile.isFile() && entryCardFile.getName().contains(PIC_2X2)) {
                return;
            }
        }
        mIsContain2x2EntryCard = false;
        LOG.error("checkContain2x2EntryCard: must contain 2x2 entryCard, please check it in "
                + snapshotDirectory.getCanonicalPath() + ".");
        throw new BundleException("checkContain2x2EntryCard: must contain 2x2 entryCard, please check it in "
                + snapshotDirectory.getCanonicalPath() + ".");
    }

    /**
     * compress file or directory.
     *
     * @param utility       common data
     * @param path          create new file by path
     * @param baseDir       base path for file
     * @param isCompression if need compression
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void pathToFile(Utility utility, String path, String baseDir, boolean isCompression)
            throws BundleException {
        if (path.isEmpty()) {
            return;
        }
        if (isCompression && LIBS_DIR_NAME.equals(baseDir)) {
            compressNativeLibsParallel(path, baseDir, utility.getCompressLevel());
            return;
        }
        File fileItem = new File(path);
        if (fileItem.isDirectory()) {
            File[] files = fileItem.listFiles();
            if (files == null) {
                return;
            }
            for (File file : files) {
                if (file.isDirectory()) {
                    compressDirectory(utility, file, baseDir, isCompression);
                } else if (isCompression) {
                    compressFile(utility, file, baseDir, isCompression);
                } else {
                    compressFile(utility, file, baseDir, isCompression);
                }
            }
        } else {
            compressFile(utility, fileItem, baseDir, isCompression);
        }
    }

    private void compressNativeLibsParallel(String path, String baseDir, int compressLevel)
            throws BundleException {
        try {
            int cores = Runtime.getRuntime().availableProcessors();
            ThreadPoolExecutor executorService = new ThreadPoolExecutor(cores, cores, 60L,
                    TimeUnit.SECONDS, new LinkedBlockingQueue<>());
            ParallelScatterZipCreator zipCreator = new ParallelScatterZipCreator(
                    executorService, new DefaultBackingStoreSupplier(null), compressLevel);
            File file = new File(path);
            if (file.isDirectory()) {
                File[] files = file.listFiles();
                if (files == null) {
                    return;
                }
                for (File f : files) {
                    addArchiveEntry(f, baseDir, zipCreator);
                }
            } else {
                addArchiveEntry(file, baseDir, zipCreator);
            }
            zipCreator.writeTo(zipOut);
        } catch (IOException | InterruptedException | ExecutionException e) {
            LOG.error(PackingToolErrMsg.COMPRESS_PARALLEL_EXCEPTION.toString(
                    "Parallel compress exist Exception (IOException | InterruptedException | ExecutionException): " +
                            e.getMessage()));
            throw new BundleException("Parallel compress exception. " + e.getMessage());
        }
    }

    private void addArchiveEntry(File file, String baseDir, ParallelScatterZipCreator zipCreator)
            throws IOException {
        if (file.isDirectory()) {
            File[] files = file.listFiles();
            if (files == null) {
                return;
            }
            for (File f : files) {
                addArchiveEntry(f, baseDir + file.getName() + File.separator, zipCreator);
            }
        } else {
            String entryName = (baseDir + file.getName()).replace(File.separator, LINUX_FILE_SEPARATOR);
            ZipArchiveEntry zipEntry = new ZipArchiveEntry(entryName);
            zipEntry.setMethod(ZipArchiveEntry.DEFLATED);
            InputStreamSupplier supplier = () -> {
                try {
                    return Files.newInputStream(file.toPath());
                } catch (IOException e) {
                    LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(
                            "Add archive entry exist IOException: " + e.getMessage()));
                    return null;
                }
            };
            zipCreator.addArchiveEntry(zipEntry, supplier);
        }
    }

    /**
     * compress file or directory, res mode
     *
     * @param utility       common data
     * @param path          create new file by path
     * @param baseDir       base path for file
     * @param isCompression if need compression
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void pathToFileResMode(Utility utility, String path, String baseDir, boolean isCompression)
            throws BundleException {
        if (path.isEmpty()) {
            return;
        }
        File fileItem = new File(path);
        if (fileItem.isDirectory()) {
            File[] files = fileItem.listFiles();
            if (files == null) {
                return;
            }
            for (File file : files) {
                if (!list.contains(file.getName())) {
                    // moduleName not in pack.info
                    continue;
                }
                if (file.isDirectory()) {
                    compressDirectory(utility, file, baseDir, isCompression);
                } else if (isCompression) {
                    compressFile(utility, file, baseDir, isCompression);
                } else {
                    compressFile(utility, file, baseDir, isCompression);
                }
            }
        } else {
            compressFile(utility, fileItem, baseDir, isCompression);
        }
    }

    /**
     * compress file directory.
     *
     * @param utility       common data
     * @param dir           file directory
     * @param baseDir       current directory name
     * @param isCompression if need compression
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressDirectory(Utility utility, File dir, String baseDir, boolean isCompression)
            throws BundleException {
        File[] files = dir.listFiles();
        if (files == null) {
            return;
        }
        for (File file : files) {
            if (file.isDirectory()) {
                compressDirectory(utility, file, baseDir + dir.getName() + File.separator, isCompression);
            } else {
                compressFile(utility, file, baseDir + dir.getName() + File.separator, isCompression);
            }
        }
    }

    /**
     * compress pack.info
     *
     * @param sourceFile source
     * @param zipOutputStream ZipOutputStream
     * @param name filename
     * @param keepDirStructure Empty File
     */
    private void compress(File sourceFile, ZipOutputStream zipOutputStream, String name, boolean keepDirStructure) {
        FileInputStream in = null;
        try {
            byte[] buf = new byte[BUFFER_SIZE];
            if (sourceFile.isFile()) {
                ZipEntry zipEntry = getStoredZipEntry(sourceFile, name);
                zipOutputStream.putNextEntry(zipEntry);
                in = new FileInputStream(sourceFile);
                int len;
                while ((len = in.read(buf)) != -1) {
                    zipOutputStream.write(buf, 0, len);
                }
                zipOutputStream.closeEntry();
            } else {
                File[] listFiles = sourceFile.listFiles();
                if (listFiles == null || listFiles.length == 0) {
                    if (keepDirStructure) {
                        if (!name.isEmpty()) {
                            ZipEntry zipEntry = getStoredZipEntry(sourceFile, name + "/");
                            zipOutputStream.putNextEntry(zipEntry);
                        } else {
                            ZipEntry zipEntry = getStoredZipEntry(sourceFile, name);
                            zipOutputStream.putNextEntry(zipEntry);
                        }
                        zipOutputStream.closeEntry();
                    }
                } else {
                    for (File file : listFiles) {
                        if (keepDirStructure) {
                            isNameEmpty(zipOutputStream, name, keepDirStructure, file);
                        } else {
                            compress(file, zipOutputStream, file.getName(), keepDirStructure);
                        }
                    }
                }
            }
        } catch (FileNotFoundException ignored) {
            LOG.error("Compressor::compressFile file not found exception.");
        } catch (IOException exception) {
            LOG.error("Compressor::compressFile io exception: " + exception.getMessage());
        } catch (BundleException bundleException) {
            LOG.error("Compressor::compressFile bundle exception" + bundleException.getMessage());
        } finally {
            Utility.closeStream(in);
        }
    }

    private ZipEntry getStoredZipEntry(File sourceFile, String name) throws BundleException {
        ZipEntry zipEntry = new ZipEntry(name);
        zipEntry.setMethod(ZipEntry.STORED);
        zipEntry.setCompressedSize(sourceFile.length());
        zipEntry.setSize(sourceFile.length());
        CRC32 crc = getCrcFromFile(sourceFile);
        zipEntry.setCrc(crc.getValue());
        return zipEntry;
    }

    private CRC32 getCrcFromFile(File file) throws BundleException {
        FileInputStream fileInputStream = null;
        CRC32 crc = new CRC32();
        try {
            fileInputStream = new FileInputStream(file);
            byte[] buffer = new byte[BUFFER_SIZE];

            int count = fileInputStream.read(buffer);
            while (count > 0) {
                crc.update(buffer, 0, count);
                count = fileInputStream.read(buffer);
            }
        } catch (FileNotFoundException ignored) {
            LOG.error(PackingToolErrMsg.FILE_NOT_FOUND.toString("File not found when getting Crc."));
            throw new BundleException("Get Crc from file failed.");
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(
                    "Get Crc from file exist IOException: " + exception.getMessage()));
            throw new BundleException("Get Crc from file failed.");
        } finally {
            Utility.closeStream(fileInputStream);
        }
        return crc;
    }

    /**
     * isNameEmpty
     *
     * @param zipOutputStream ZipOutputStream
     * @param name filename
     * @param keepDirStructure KeepDirStructure
     * @param file file
     */
    private void isNameEmpty(ZipOutputStream zipOutputStream, String name, boolean keepDirStructure, File file) {
        if (!name.isEmpty()) {
            compress(file, zipOutputStream, name + "/" + file.getName(), keepDirStructure);
        } else {
            compress(file, zipOutputStream, file.getName(), keepDirStructure);
        }
    }

    /**
     * compress process.
     *
     * @param utility       common data
     * @param srcFile       source file to zip
     * @param baseDir       current directory name of file
     * @param isCompression if need compression
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressFile(Utility utility, File srcFile, String baseDir, boolean isCompression)
            throws BundleException {
        BufferedInputStream bufferedInputStream = null;
        FileInputStream fileInputStream = null;
        try {
            String entryName = (baseDir + srcFile.getName()).replace(File.separator, LINUX_FILE_SEPARATOR);
            ZipArchiveEntry zipEntry = new ZipArchiveEntry(entryName);
            isEntryOpen = true;
            if (!entryName.contains(RAW_FILE_PATH)
                    && !entryName.contains(RES_FILE_PATH)
                    && srcFile.getName().toLowerCase(Locale.ENGLISH).endsWith(JSON_SUFFIX)
                    && !entryName.equals(Constants.FILE_ENCRYPT_JSON)
                    && !entryName.equals(Constants.FILE_PAC_JSON)) {
                zipEntry.setMethod(ZipEntry.STORED);
                if (jsonSpecialProcess(utility, srcFile, zipEntry)) {
                    return;
                }
            }

            if (isCompression) {
                zipEntry.setMethod(ZipEntry.DEFLATED);
            } else {
                zipEntry.setMethod(ZipEntry.STORED);

                // update size
                zipEntry.setCompressedSize(srcFile.length());
                zipEntry.setSize(srcFile.length());

                // update crc
                CRC32 crc = getCrcFromFile(utility, srcFile);
                zipEntry.setCrc(crc.getValue());
            }

            zipOut.putArchiveEntry(zipEntry);
            byte[] data = new byte[BUFFER_SIZE];
            fileInputStream = new FileInputStream(srcFile);
            bufferedInputStream = new BufferedInputStream(fileInputStream);

            int count = bufferedInputStream.read(data);
            while (count > 0) {
                zipOut.write(data, 0, count);
                count = bufferedInputStream.read(data);
            }
        } catch (FileNotFoundException ignored) {
            throw new BundleException("CompressFile failed.");
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.COMPRESS_FILE_EXCEPTION.toString(
                "IOException: " + exception.getMessage()));
            throw new BundleException("CompressFile failed.");
        } finally {
            Utility.closeStream(bufferedInputStream);
            Utility.closeStream(fileInputStream);
        }
    }

    /**
     * check hap type for pack app.
     *
     * @param hapPath source file to zip
     * @return true is for is stage type and false is for FA type
     * @throws BundleException FileNotFoundException|IOException.
     */
    public static boolean isModuleHap(String hapPath) throws BundleException {
        if (!hapPath.toLowerCase(Locale.ENGLISH).endsWith(HAP_SUFFIX)) {
            return true;
        }

        FileInputStream zipInput = null;
        ZipInputStream zin = null;
        ZipEntry entry = null;
        try {
            zipInput = new FileInputStream(hapPath);
            zin = new ZipInputStream(zipInput);
            while ((entry = zin.getNextEntry()) != null) {
                if (MODULE_JSON.equals(entry.getName().toLowerCase())) {
                    return true;
                }
            }
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.FILE_IO_EXCEPTION.toString(
                    "Check module type exist IOException: " + exception.getMessage()));
            throw new BundleException("Check module type for pack app failed.");
        } finally {
            Utility.closeStream(zipInput);
            Utility.closeStream(zin);
        }
        return false;
    }

    /**
     * get CRC32 from file.
     *
     * @param utility common data
     * @param file    source file
     * @return CRC32
     * @throws BundleException FileNotFoundException|IOException.
     */
    private CRC32 getCrcFromFile(Utility utility, File file) throws BundleException {
        FileInputStream fileInputStream = null;
        CRC32 crc = new CRC32();
        try {
            fileInputStream = new FileInputStream(file);
            byte[] buffer = new byte[BUFFER_SIZE];

            int count = fileInputStream.read(buffer);
            while (count > 0) {
                crc.update(buffer, 0, count);
                count = fileInputStream.read(buffer);
            }
        } catch (FileNotFoundException ignored) {
            LOG.error(PackingToolErrMsg.FILE_NOT_FOUND.toString("File not found when getting Crc."));
            throw new BundleException("Get Crc from file failed: " + file.getName());
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.FILE_IO_EXCEPTION.toString(
                    "Get Crc32 from file exist IOException: " + exception.getMessage()));
            throw new BundleException("Get Crc from file failed.");
        } finally {
            Utility.closeStream(fileInputStream);
        }
        return crc;
    }

    private void parsePackInfoJsonFile(Utility utility, String jsonPath)
            throws BundleException {
        try {
            String jsonString = FileUtils.getFileContent(jsonPath).get();
            List<String> nameList = new ArrayList<>();
            ModuleJsonUtil.parsePackInfoFormsName(jsonString, nameList, formNamesList);
            for (String formName : nameList) {
                if (formName.isEmpty()) {
                    LOG.error(PackingToolErrMsg.PARSE_PACK_INFO_JSON_FAILED.toString("Form name parsed from " +
                            "pack.info is empty."));
                    continue;
                }

                utility.addFormNameList(formName);
            }
        } catch (JSONException e) {
            LOG.error(PackingToolErrMsg.PARSE_PACK_INFO_JSON_FAILED.toString("Parse pack.info eixst JSONException: " +
                    e.getMessage()));
        }
    }

    private void infoSpecialProcess(Utility utility, File srcFile)
            throws BundleException {
        FileInputStream fileInputStream = null;
        BufferedReader bufferedReader = null;
        InputStreamReader inputStreamReader = null;

        try {
            fileInputStream = new FileInputStream(srcFile);
            inputStreamReader = new InputStreamReader(fileInputStream, StandardCharsets.UTF_8);
            bufferedReader = new BufferedReader(inputStreamReader);
            bufferedReader.mark((int) srcFile.length() + 1);
            // parse moduleName from pack.info
            parsePackModuleName(bufferedReader, utility);
            bufferedReader.reset();
            parseDeviceType(bufferedReader, utility);
            bufferedReader.reset();

            Pattern pattern = Pattern.compile(System.lineSeparator());
            String str = bufferedReader.readLine();
            StringBuilder builder = new StringBuilder();
            while (str != null) {
                Matcher matcher = pattern.matcher(str.trim());
                String dest = matcher.replaceAll("");
                builder.append(dest);
                str = bufferedReader.readLine();
            }
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("pack.info special process exist IOException: "
                    + exception.getMessage()));
            throw new BundleException("Json special process failed.");
        } finally {
            Utility.closeStream(bufferedReader);
            Utility.closeStream(inputStreamReader);
            Utility.closeStream(fileInputStream);
        }
    }

    /**
     * trim and remove "\r\n" in *.json file.
     *
     * @param utility common data
     * @param srcFile file input
     * @param entry   zip file entry
     * @return true if process success
     */
    private boolean jsonSpecialProcess(Utility utility, File srcFile, ZipArchiveEntry entry) {
        FileInputStream fileInputStream = null;
        BufferedReader bufferedReader = null;
        InputStreamReader inputStreamReader = null;

        try {
            fileInputStream = new FileInputStream(srcFile);
            inputStreamReader = new InputStreamReader(fileInputStream, StandardCharsets.UTF_8);
            bufferedReader = new BufferedReader(inputStreamReader);
            bufferedReader.mark((int) srcFile.length() + 1);
            bufferedReader.reset();
            Optional<String> optional = FileUtils.getFileContent(utility.getJsonPath());
            String jsonString = optional.get();
            String jsonName = new File(utility.getJsonPath()).getName().toLowerCase(Locale.ENGLISH);
            if (CONFIG_JSON.equals(jsonName)) {
                parseCompressNativeLibs(bufferedReader, utility);
                utility.setModuleName(ModuleJsonUtil.parseFaModuleName(jsonString));
            } else if (MODULE_JSON.equals(jsonName)) {
                utility.setIsCompressNativeLibs(ModuleJsonUtil.stageIsCompressNativeLibs(jsonString));
                utility.setModuleName(ModuleJsonUtil.parseStageModuleName(jsonString));
            } else if (PATCH_JSON.equals(jsonName)) {
                utility.setModuleName(ModuleJsonUtil.parsePatchModuleName(jsonString));
            }
            bufferedReader.reset();
            parseDeviceType(bufferedReader, utility);
            bufferedReader.reset();

            Pattern pattern = Pattern.compile(System.lineSeparator());
            String str = bufferedReader.readLine();
            StringBuilder builder = new StringBuilder();
            while (str != null) {
                Matcher matcher = pattern.matcher(str.trim());
                String dest = matcher.replaceAll("");
                builder.append(dest);
                str = bufferedReader.readLine();
            }
            Object jsonObject = JSON.parse(builder.toString());
            byte[] trimJson = JSON.toJSONBytes(jsonObject,
                    SerializerFeature.WriteMapNullValue, SerializerFeature.WriteDateUseDateFormat);

            // update crc
            CRC32 crc = new CRC32();
            crc.update(trimJson);
            entry.setCrc(crc.getValue());

            // update size
            entry.setSize(trimJson.length);
            entry.setCompressedSize(trimJson.length);

            // compress data
            zipOut.putArchiveEntry(entry);
            zipOut.write(trimJson);
        } catch (Exception exception) {
            LOG.error(PackingToolErrMsg.JSON_SPECIAL_PROCESS_FAILED.toString("Exception: " + exception.getMessage()));
            LOG.warning("Json format err: " + srcFile.getAbsolutePath());
            return false;
        } finally {
            Utility.closeStream(bufferedReader);
            Utility.closeStream(inputStreamReader);
            Utility.closeStream(fileInputStream);
        }
        return true;
    }

    /**
     * Parse module name from pack.info
     *
     * @param bufferedReader pack.info buffered Reader
     * @param utility        common data
     * @throws BundleException IOException
     */
    private void parsePackModuleName(BufferedReader bufferedReader, Utility utility) throws BundleException {
        String lineStr = null;
        try {
            while ((lineStr = bufferedReader.readLine()) != null) {
                if (lineStr.contains(DISTRO)) {
                    continue;
                }
                if (lineStr.contains(JSON_END)) {
                    continue;
                }
                if (lineStr.contains(MODULE_NAME_NEW) || lineStr.contains(MODULE_NAME)) {
                    getModuleNameFromString(lineStr, utility);
                }
            }
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Parse module name exist IOException: "
                    + exception.getMessage()));
            throw new BundleException("Parse module name failed.");
        }
    }

    /**
     * Parse Forms name from pack.info
     *
     * @param bufferedReader pack.info buffered Reader
     * @param utility        common data
     * @throws BundleException IOException
     */
    private void parsePackFormName(BufferedReader bufferedReader, Utility utility) throws BundleException {
        String lineStr = null;
        try {
            while ((lineStr = bufferedReader.readLine()) != null) {
                if (lineStr.contains("abilities")) {
                    continue;
                }
                if (lineStr.contains(FORMS)) {
                    continue;
                }
                if (lineStr.contains(JSON_END)) {
                    continue;
                }
                if (lineStr.contains(NAME)) {
                    getNameFromString(lineStr, utility);
                }
            }
        } catch (IOException exception) {
            LOG.error("Compressor::parseModuleName io exception: " + exception.getMessage());
            throw new BundleException("Parse module name failed.");
        }
    }

    /**
     * Get name from line string
     *
     * @param lineStr line string
     * @param utility common data
     * @throws BundleException StringIndexOutOfBoundsException
     */
    private void getNameFromString(String lineStr, Utility utility) throws BundleException {
        try {
            int endIndex = lineStr.lastIndexOf(SEMICOLON);
            if (endIndex <= 0) {
                LOG.error("Compressor::getModuleNameFromString field the json is not standard.");
                throw new BundleException("Parse module name failed, module-name is invalid.");
            }
            int startIndex = lineStr.lastIndexOf(SEMICOLON, endIndex - 1) + 1;
            String formName = lineStr.substring(startIndex, endIndex);
            if (formName == null || formName.isEmpty()) {
                LOG.error("Compressor::getModuleNameFromString field module-name is empty.");
                throw new BundleException("Parse module name failed, module-name is empty.");
            }
            String[] nameList = formName.split("\\.");
            if (nameList.length <= 1) {
                formNamesList.add(formName);
                utility.addFormNameList(formName);
            }
        } catch (StringIndexOutOfBoundsException exception) {
            LOG.error("Compressor::parseModuleName field module-name is fault: " + exception.getMessage());
            throw new BundleException("Parse module name failed, module-name is invalid.");
        }
    }

    /**
     * Get module name from line string
     *
     * @param lineStr line string
     * @param utility common data
     * @throws BundleException StringIndexOutOfBoundsException
     */
    private void getModuleNameFromString(String lineStr, Utility utility) throws BundleException {
        try {
            int endIndex = lineStr.lastIndexOf(SEMICOLON);
            if (endIndex <= 0) {
                LOG.error(PackingToolErrMsg.GET_MODULE_NAME_FROM_STRING_FAILED.toString("Unable to find '\"' "
                        + "in the lineStr."));
                throw new BundleException("Parse module name failed, module-name is invalid.");
            }
            int startIndex = lineStr.lastIndexOf(SEMICOLON, endIndex - 1) + 1;
            String moduleName = lineStr.substring(startIndex, endIndex);
            list.add(moduleName);
            if (moduleName == null || moduleName.isEmpty()) {
                LOG.error(PackingToolErrMsg.GET_MODULE_NAME_FROM_STRING_FAILED.toString("moduleName or module-name "
                        + "is empty."));
                throw new BundleException("Parse module name failed, module-name is empty.");
            }
            utility.setModuleName(moduleName);
        } catch (StringIndexOutOfBoundsException exception) {
            LOG.error(PackingToolErrMsg.GET_MODULE_NAME_FROM_STRING_FAILED.toString("Get Module name from line string "
                    + "exist StringIndexOutOfBoundsException: " + exception.getMessage()));
            throw new BundleException("Parse module name failed, module-name is invalid.");
        }
    }

    private void parseCompressNativeLibs(BufferedReader bufferedReader, Utility utility) throws BundleException {
        String lineStr = null;
        try {
            while ((lineStr = bufferedReader.readLine()) != null) {
                if (lineStr.contains(COMPRESS_NATIVE_LIBS)) {
                    if (lineStr.contains(Utility.TRUE_STRING)) {
                        utility.setIsCompressNativeLibs(true);
                        break;
                    }
                }
            }
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(
                    "Parse compress native libs exist IOException: " + exception.getMessage()));
            throw new BundleException("Parse compress native libs failed.");
        }
    }

    /**
     * ZipOutputStream flush, closeEntry and finish.
     */
    private void closeZipOutputStream() {
        try {
            if (zipOut != null) {
                zipOut.flush();
            }
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.CLOSE_ZIP_OUTPUT_STREAM_EXCEPTION.toString(
                    "Close zip output stream flush IOException: " + exception.getMessage()));
        }
        try {
            if (zipOut != null && isEntryOpen) {
                zipOut.closeArchiveEntry();
            }
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.CLOSE_ZIP_OUTPUT_STREAM_EXCEPTION.toString(
                    "Close entry IOException: " + exception.getMessage()));
        }
        try {
            if (zipOut != null) {
                zipOut.finish();
            }
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.CLOSE_ZIP_OUTPUT_STREAM_EXCEPTION.toString(
                    "Close zip output stream flush IOException: " + exception.getMessage()));
        }
    }

    /**
     * Parse device type from config.json
     *
     * @param bufferedReader config.json buffered Reader
     * @param utility        common data
     * @throws BundleException IOException
     */
    private void parseDeviceType(BufferedReader bufferedReader, Utility utility) throws BundleException {
        String lineStr = null;
        boolean isDeviceTypeStart = false;
        try {
            while ((lineStr = bufferedReader.readLine()) != null) {
                if (!isDeviceTypeStart) {
                    if (lineStr.contains(DEVICE_TYPE)) {
                        isDeviceTypeStart = true;
                    }
                    continue;
                }
                if (lineStr.contains(JSON_END)) {
                    break;
                }
                utility.setDeviceType(lineStr);
                break;
            }
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(
                    "Parse device type exist IOException: " + exception.getMessage()));
            throw new BundleException("Parse device type failed.");
        }
    }

    /**
     * check hap and hsp is valid in haps when pack app, check type has bundleName,
     * vendor, version, apiVersion moduleName, packageName.
     *
     * @param fileLists is the list of hapPath.
     * @return true is for successful and false is for failed
     * @throws BundleException FileNotFoundException|IOException.
     */
    private boolean checkHapIsValid(List<String> fileLists, boolean isSharedApp) throws BundleException {
        List<HapVerifyInfo> hapVerifyInfos = new ArrayList<>();
        for (String hapPath : fileLists) {
            if (hapPath.isEmpty()) {
                LOG.error(PackingToolErrMsg.INVALID_HAP_FILE.toString("Invalid hap or hsp file input."));
                throw new BundleException("The hap or hsp files are invalid, or the wrong file was provided.");
            }
            File srcFile = new File(hapPath);
            String fileStr = srcFile.getName();
            if (fileStr.isEmpty()) {
                LOG.error(PackingToolErrMsg.INVALID_HAP_FILE.toString("Get file name failed."));
                throw new BundleException("Get file name from the provided path failed.");
            }
            if (!fileStr.toLowerCase(Locale.ENGLISH).endsWith(HAP_SUFFIX)
                    && !fileStr.toLowerCase(Locale.ENGLISH).endsWith(HSP_SUFFIX)) {
                LOG.error(PackingToolErrMsg.INVALID_HAP_FILE.toString("Invalid hap or hsp file input."));
                throw new BundleException("The provided file is not a valid hap or hsp file.");
            }
            if (isModuleHap(hapPath)) {
                hapVerifyInfos.add(parseStageHapVerifyInfo(hapPath));
            } else {
                hapVerifyInfos.add(parseFAHapVerifyInfo(hapPath));
            }

            if (!hapVerifyInfos.isEmpty()) {
                hapVerifyInfoMap.put(getFileNameByPath(hapPath), hapVerifyInfos.get(hapVerifyInfos.size() - 1));
            }
        }

        if (isSharedApp) {
            boolean res = checkSharedAppIsValid(hapVerifyInfos);
            if (!res) {
                return false;
            }
            if (!isOverlay) {
                return true;
            }
        } else {
            for (HapVerifyInfo hapVerifyInfo : hapVerifyInfos) {
                String bundleType = hapVerifyInfo.getBundleType();
                if (TYPE_SHARED.equals(bundleType)) {
                    String cause = "Only one item can be entered in the --hsp-path when bundleType is shared.";
                    String solution = "Ensure that only one item entered in the --hsp-path when bundleType is shared.";
                    LOG.error(PackingToolErrMsg.CHECK_BUNDLETYPE_INVALID.toString(cause, solution));
                    return false;
                }
            }
        }
        setAtomicServiceFileSizeLimit(hapVerifyInfos);
        if (!HapVerify.checkHapIsValid(hapVerifyInfos)) {
            return false;
        }
        return true;
    }

    /**
     * parse stage file to hap verify info from hap path.
     *
     * @param filePath is the hap path
     * @return hapVerifyInfo
     */
    public static HapVerifyInfo parseStageHapVerifyInfo(String filePath) throws BundleException {
        HapVerifyInfo hapVerifyInfo = readStageHapVerifyInfo(filePath);
        hapVerifyInfo.setStageModule(true);
        ModuleJsonUtil.parseStageHapVerifyInfo(hapVerifyInfo);
        hapVerifyInfo.setFileLength(FileUtils.getFileSize(filePath));
        File srcFile = new File(filePath);
        String fileStr = srcFile.getName();
        if (fileStr.toLowerCase(Locale.ENGLISH).endsWith(HAP_SUFFIX)) {
            hapVerifyInfo.setFileType(HAP_SUFFIX);
        } else if (fileStr.toLowerCase(Locale.ENGLISH).endsWith(HSP_SUFFIX)) {
            hapVerifyInfo.setFileType(HSP_SUFFIX);
        }
        return hapVerifyInfo;
    }

    /**
     * set file size limit for each HapVerifyInfo.
     *
     * @param hapVerifyInfos Indicates hapVerifyInfo list.
     */
    public static void setAtomicServiceFileSizeLimit(List<HapVerifyInfo>hapVerifyInfos) {
        for (HapVerifyInfo hapVerifyInfo : hapVerifyInfos) {
            if (!hapVerifyInfo.getBundleType().equals(ATOMIC_SERVICE)) {
                continue;
            }
            hapVerifyInfo.setEntrySizeLimit(getEntryModuleSizeLimit());
            hapVerifyInfo.setNotEntrySizeLimit(getNotEntryModuleSizeLimit());
            hapVerifyInfo.setSumSizeLimit(getSumModuleSizeLimit());
        }
    }

    /**
     * parse fa file to hap verify info from hap path.
     *
     * @param filePath is the hap path
     * @return hapVerifyInfo
     */
    public static HapVerifyInfo parseFAHapVerifyInfo(String filePath) throws BundleException {
        HapVerifyInfo hapVerifyInfo = readFAHapVerifyInfo(filePath);
        hapVerifyInfo.setStageModule(false);
        hapVerifyInfo.setFileLength(FileUtils.getFileSize(filePath));
        ModuleJsonUtil.parseFAHapVerifyInfo(hapVerifyInfo);
        return hapVerifyInfo;
    }

    /**
     * read stage hap verify info from hap file.
     *
     * @param srcPath source file to zip
     * @return HapVerifyInfo of parse result
     * @throws BundleException FileNotFoundException|IOException.
     */
    public static HapVerifyInfo readStageHapVerifyInfo(String srcPath) throws BundleException {
        HapVerifyInfo hapVerifyInfo = new HapVerifyInfo();
        ZipFile zipFile = null;
        try {
            File srcFile = new File(srcPath);
            zipFile = new ZipFile(srcFile);
            hapVerifyInfo.setResourceMap(FileUtils.getProfileJson(zipFile));
            hapVerifyInfo.setProfileStr(FileUtils.getFileStringFromZip(MODULE_JSON, zipFile));
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.READ_STAGE_HAP_VERIFY_INFO_FAILED.toString(
                    "Read Stage hap verify info file exist IOException: " + e.getMessage()));
            throw new BundleException("Parse Stage hap verify info file exist IOException.");
        } finally {
            Utility.closeStream(zipFile);
        }
        return hapVerifyInfo;
    }

    /**
     * read fa hap verify info from hap file.
     *
     * @param srcPath source file to zip
     * @return HapVerifyInfo of parse result
     * @throws BundleException FileNotFoundException|IOException.
     */
    public static HapVerifyInfo readFAHapVerifyInfo(String srcPath) throws BundleException {
        HapVerifyInfo hapVerifyInfo = new HapVerifyInfo();
        ZipFile zipFile = null;
        try {
            File srcFile = new File(srcPath);
            zipFile = new ZipFile(srcFile);
            hapVerifyInfo.setProfileStr(FileUtils.getFileStringFromZip(CONFIG_JSON, zipFile));
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.READ_FA_HAP_VERIFY_INFO_FAILED.toString(
                    "Read FA hap verify info file exist IOException: " + e.getMessage()));
            throw new BundleException("Parse FA hap verify info file exist IOException.");
        } finally {
            Utility.closeStream(zipFile);
        }
        return hapVerifyInfo;
    }

    /**
     * compress in hqf mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressHQFMode(Utility utility) throws BundleException {
        pathToFile(utility, utility.getJsonPath(), NULL_DIR_NAME, false);

        if (!utility.getEtsPath().isEmpty()) {
            pathToFile(utility, utility.getEtsPath(), ETS_PATH, false);
        }
        if (!utility.getLibPath().isEmpty()) {
            pathToFile(utility, utility.getLibPath(), LIBS_DIR_NAME, false);
        }
        if (!utility.getResourcesPath().isEmpty()) {
            pathToFile(utility, utility.getResourcesPath(), RESOURCES_DIR_NAME, false);
        }
    }

    /**
     * compress in appqf mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressAPPQFMode(Utility utility) throws BundleException {
        List<String> fileList = utility.getFormatedHQFList();
        if (!checkHQFIsValid(fileList)) {
            LOG.error(PackingToolErrMsg.COMPRESS_APPQF_FAILED.toString("Verify failed when compress appqf."));
            throw new BundleException("Verify failed when compress appqf.");
        }
        for (String hapPath : fileList) {
            pathToFile(utility, hapPath, NULL_DIR_NAME, false);
        }
    }

    /**
     * check input hqf is valid.
     *
     * @param fileList is input path of hqf files
     * @throws BundleException FileNotFoundException|IOException.
     */
    private boolean checkHQFIsValid(List<String> fileList) throws BundleException {
        List<HQFInfo> hqfVerifyInfos = new ArrayList<>();
        for (String file : fileList) {
            hqfVerifyInfos.add(ModuleJsonUtil.parseHQFInfo(file));
        }
        if (!HQFVerify.checkHQFIsValid(hqfVerifyInfos)) {
            LOG.error(PackingToolErrMsg.CHECK_HQF_INVALID.toString("Input hqf is invalid."));
            return false;
        }
        return true;
    }

    private void compressHSPMode(Utility utility) throws BundleException {
        pathToFile(utility, utility.getJsonPath(), NULL_DIR_NAME, false);

        pathToFile(utility, utility.getProfilePath(), NULL_DIR_NAME, false);

        if (!utility.getIndexPath().isEmpty() && isModuleJSON(utility.getJsonPath())) {
            String assetsPath = NULL_DIR_NAME;
            pathToFile(utility, utility.getIndexPath(), assetsPath, false);
        }

        if (!utility.getLibPath().isEmpty()) {
            pathToFile(utility, utility.getLibPath(), LIBS_DIR_NAME, utility.isCompressNativeLibs());
        }

        if (!utility.getANPath().isEmpty()) {
            pathToFile(utility, utility.getANPath(), AN_DIR_NAME, false);
        }

        if (!utility.getAPPath().isEmpty()) {
            pathToFile(utility, utility.getAPPath(), AP_PATH_NAME, false);
        }

        if (!utility.getFilePath().isEmpty()) {
            pathToFile(utility, utility.getFilePath(), NULL_DIR_NAME, false);
        }

        if (!utility.getResPath().isEmpty() && !utility.getModuleName().isEmpty()) {
            String resPath = ASSETS_DIR_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR
                    + RESOURCES_DIR_NAME;
            if (DEVICE_TYPE_FITNESSWATCH.equals(
                    utility.getDeviceType().replace(SEMICOLON, EMPTY_STRING).trim()) ||
                    DEVICE_TYPE_FITNESSWATCH_NEW.equals(
                            utility.getDeviceType().replace(SEMICOLON, EMPTY_STRING).trim())) {
                resPath = RES_DIR_NAME;
            }
            pathToFile(utility, utility.getResPath(), resPath, false);
        }

        if (!utility.getResourcesPath().isEmpty() && isModuleJSON(utility.getJsonPath())) {
            String resourcesPath = RESOURCES_DIR_NAME;
            pathToFile(utility, utility.getResourcesPath(), resourcesPath, false);
        }
        if (!utility.getJsPath().isEmpty() && isModuleJSON(utility.getJsonPath())) {
            String jsPath = JS_PATH;
            pathToFile(utility, utility.getJsPath(), jsPath, false);
        }

        if (!utility.getEtsPath().isEmpty() && isModuleJSON(utility.getJsonPath())) {
            String etsPath = ETS_PATH;
            pathToFile(utility, utility.getEtsPath(), etsPath, false);
        }

        if (!utility.getRpcidPath().isEmpty()) {
            String rpcidPath = NULL_DIR_NAME;
            pathToFile(utility, utility.getRpcidPath(), rpcidPath, false);
        }

        if (!utility.getAssetsPath().isEmpty()) {
            pathToFile(utility, utility.getAssetsPath(), ASSETS_DIR_NAME, false);
        }

        if (!utility.getBinPath().isEmpty()) {
            pathToFile(utility, utility.getBinPath(), NULL_DIR_NAME, false);
        }

        if (!utility.getPackInfoPath().isEmpty()) {
            pathToFile(utility, utility.getPackInfoPath(), NULL_DIR_NAME, false);
        }

        // pack --dir-list
        if (!utility.getFormatedDirList().isEmpty()) {
            for (int i = 0; i < utility.getFormatedDirList().size(); ++i) {
                String baseDir = new File(utility.getFormatedDirList().get(i)).getName() + File.separator;
                pathToFile(utility, utility.getFormatedDirList().get(i), baseDir, false);
            }
        }
        if (!utility.getPkgContextPath().isEmpty()) {
            pathToFile(utility, utility.getPkgContextPath(), NULL_DIR_NAME, false);
        }

        compressHapModeMultiple(utility);
    }

    private static boolean checkSharedAppIsValid(List<HapVerifyInfo> hapVerifyInfos) throws BundleException {
        if (hapVerifyInfos.isEmpty()) {
            String cause = "Hap verify infos is empty.";
            LOG.error(PackingToolErrMsg.CHECK_HAP_VERIFY_INFO_LIST_EMPTY.toString(cause));
            return false;
        }
        if (hapVerifyInfos.size() > SHARED_APP_HSP_LIMIT) {
            String cause = "The shared App only can contain one module.";
            String solution = "Please ensure that there is only one module in the shared App.";
            LOG.error(PackingToolErrMsg.CHECK_SHARED_APP_INVALID.toString(cause, solution));
            return false;
        }
        for (HapVerifyInfo hapVerifyInfo : hapVerifyInfos) {
            if (!hapVerifyInfo.getTargetBundleName().isEmpty()) {
                isOverlay = true;
                return true;
            }
        }
        return HapVerify.checkSharedApppIsValid(hapVerifyInfos);
    }

    private void versionNormalize(Utility utility) {
        List<VersionNormalizeUtil> utils = new ArrayList<>();
        Path tempDir = null;
        for (String hapPath : utility.getFormattedHapList()) {
            try {
                tempDir = Files.createTempDirectory(Paths.get(utility.getOutPath()), "temp");

                unpackHap(hapPath, tempDir.toAbsolutePath().toString());
                VersionNormalizeUtil util = new VersionNormalizeUtil();
                File moduleFile = new File(
                        tempDir.toAbsolutePath() + LINUX_FILE_SEPARATOR + MODULE_JSON);
                File configFile = new File(
                        tempDir.toAbsolutePath() + LINUX_FILE_SEPARATOR + CONFIG_JSON);
                File packInfoFile = new File(
                        tempDir.toAbsolutePath() + LINUX_FILE_SEPARATOR + PACKINFO_NAME);

                if (moduleFile.exists() && configFile.exists()) {
                    LOG.error(PackingToolErrMsg.VERSION_NORMALIZE_FAILED.toString("Invalid hap structure."));
                    throw new BundleException("versionNormalize failed, invalid hap structure.");
                }
                if (moduleFile.exists()) {
                    String moduleJsonPath = tempDir.resolve(MODULE_JSON).toString();
                    util = parseAndModifyModuleJson(moduleJsonPath, utility);
                } else if (configFile.exists()) {
                    String configJsonPath = tempDir.resolve(CONFIG_JSON).toString();
                    util = parseAndModifyConfigJson(configJsonPath, utility);
                } else {
                    LOG.error(PackingToolErrMsg.VERSION_NORMALIZE_FAILED.toString("Invalid hap structure."));
                    throw new BundleException("versionNormalize failed, invalid hap structure.");
                }
                if (packInfoFile.exists()) {
                    String packInfoPath = tempDir.resolve(PACKINFO_NAME).toString();
                    parseAndModifyPackInfo(packInfoPath, utility);
                }

                verifyModuleVersion(util, utility);
                utils.add(util);

                String modifiedHapPath = Paths.get(utility.getOutPath()) +
                        LINUX_FILE_SEPARATOR + Paths.get(hapPath).getFileName().toString();
                compressDirToHap(tempDir, modifiedHapPath);
            } catch (IOException | BundleException e) {
                LOG.error(PackingToolErrMsg.VERSION_NORMALIZE_FAILED.toString(
                        "Version normalize exist Exception (IOException | BundleException): " + e.getMessage()));
            } finally {
                if (tempDir != null) {
                    deleteDirectory(tempDir.toFile());
                }
            }
        }
        writeVersionRecord(utils, utility.getOutPath());
    }

    private VersionNormalizeUtil parseAndModifyModuleJson(String jsonFilePath, Utility utility)
            throws BundleException {
        VersionNormalizeUtil util = new VersionNormalizeUtil();
        try (FileInputStream jsonStream = new FileInputStream(jsonFilePath)) {
            JSONObject jsonObject = JSON.parseObject(jsonStream, JSONObject.class);
            if (!jsonObject.containsKey(APP)) {
                LOG.error(PackingToolErrMsg.PARSE_AND_MODIFY_MODULEJSON_FAILED.toString("The module.json file " +
                        "does not contain 'app'."));
                throw new BundleException("The module.json file does not contain 'app'. ");
            }
            JSONObject appObject = jsonObject.getJSONObject(APP);
            if (!appObject.containsKey(VERSION_CODE)) {
                LOG.error(PackingToolErrMsg.PARSE_AND_MODIFY_MODULEJSON_FAILED.toString("The app object of " +
                        "module.json file does not contain 'versionCode'."));
                throw new BundleException("The app object of module.json file does not contain 'versionCode'.");
            }
            if (!appObject.containsKey(VERSION_NAME)) {
                LOG.error(PackingToolErrMsg.PARSE_AND_MODIFY_MODULEJSON_FAILED.toString("The app object of " +
                        "module.json file does not contain 'versionName'."));
                throw new BundleException("The app object of module.json file does not contain 'versionName'.");
            }
            util.setOriginVersionCode(appObject.getIntValue(VERSION_CODE));
            util.setOriginVersionName(appObject.getString(VERSION_NAME));

            JSONObject moduleObject = jsonObject.getJSONObject(MODULE);
            if (!moduleObject.containsKey(NAME)) {
                LOG.error(PackingToolErrMsg.PARSE_AND_MODIFY_MODULEJSON_FAILED.toString("The module object of " +
                        "module.json file does not contain 'name'."));
                throw new BundleException("The module object of module.json file does not contain 'name'. ");
            }
            util.setModuleName(moduleObject.getString(NAME));
            appObject.put(VERSION_CODE, utility.getVersionCode());
            appObject.put(VERSION_NAME, utility.getVersionName());
            writeJson(jsonFilePath, jsonObject);
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Parse and modify module.json exist IOException: " +
                    e.getMessage()));
            throw new BundleException("Parse and modify module.json exist IOException: " + e.getMessage());
        }
        return util;
    }

    private void parseAndModifyPackInfo(String packInfoPath, Utility utility)
            throws BundleException {
        try (FileInputStream jsonStream = new FileInputStream(packInfoPath)) {
            JSONObject jsonObject = JSON.parseObject(jsonStream, JSONObject.class);
            if (jsonObject == null) {
                LOG.warning("parseAndModifyPackInfo failed, json format invalid.");
                return;
            }
            JSONObject summaryObject = jsonObject.getJSONObject(SUMMARY);
            if (summaryObject == null) {
                LOG.warning("parseAndModifyPackInfo failed, summary invalid.");
                return;
            }
            JSONObject appObject = summaryObject.getJSONObject(APP);
            if (appObject == null) {
                LOG.warning("parseAndModifyPackInfo failed, app invalid.");
                return;
            }
            JSONObject versionObject = appObject.getJSONObject(VERSION);
            if (versionObject == null) {
                LOG.warning("parseAndModifyPackInfo failed, version invalid.");
                return;
            }
            versionObject.put(CODE, utility.getVersionCode());
            versionObject.put(NAME, utility.getVersionName());
            writeJson(packInfoPath, jsonObject);
        } catch (IOException e) {
            LOG.warning("parseAndModifyPackInfo failed, IOException." + e.getMessage());
        }
    }

    private void writeJson(String jsonFilePath, JSONObject jsonObject) throws IOException, BundleException {
        BufferedWriter bw = null;
        try {
            String pretty = JSON.toJSONString(jsonObject, SerializerFeature.WriteMapNullValue,
                SerializerFeature.WriteDateUseDateFormat);
            bw = new BufferedWriter(new OutputStreamWriter(
                    new FileOutputStream(jsonFilePath), StandardCharsets.UTF_8));
            bw.write(pretty);
        } catch (IOException exception) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Write json exist IOException: "
                    + exception.getMessage()));
            throw new BundleException("Compressor::writeJson failed for IOException");
        } finally {
            if (bw != null) {
                bw.flush();
                bw.close();
            }
        }
    }

    private VersionNormalizeUtil parseAndModifyConfigJson(String jsonFilePath, Utility utility)
            throws BundleException {
        VersionNormalizeUtil util = new VersionNormalizeUtil();
        try (FileInputStream jsonStream = new FileInputStream(jsonFilePath)) {
            JSONObject jsonObject = JSON.parseObject(jsonStream, JSONObject.class);
            if (!jsonObject.containsKey(APP)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The config.json file " +
                        "does not contain 'app'."));
                throw new BundleException("The config.json file does not contain 'app'. ");
            }
            JSONObject appObject = jsonObject.getJSONObject(APP);
            if (!appObject.containsKey(VERSION)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The app object of config.json " +
                        "file does not contain 'version'."));
                throw new BundleException("The app object of config.json file does not contain 'version'. ");
            }
            JSONObject versionObj = appObject.getJSONObject(VERSION);
            if (!versionObj.containsKey(CODE)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The version object of " +
                        "config.json file does not contain 'code'."));
                throw new BundleException("The version object of config.json file does not contain 'code'. ");
            }
            util.setOriginVersionCode(versionObj.getIntValue(CODE));
            if (!versionObj.containsKey(NAME)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The version object of " +
                        "config.json file does not contain 'name'."));
                throw new BundleException("The version object of config.json file does not contain 'name'. ");
            }
            util.setOriginVersionName(versionObj.getString(NAME));

            JSONObject moduleObject = jsonObject.getJSONObject(MODULE);
            if (!moduleObject.containsKey(NAME)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The module object of " +
                        "config.json file does not contain 'name'."));
                throw new BundleException("The module object of config.json file does not contain 'name'. ");
            }
            util.setModuleName(moduleObject.getString(NAME));

            versionObj.put(CODE, utility.getVersionCode());
            versionObj.put(NAME, utility.getVersionName());
            writeJson(jsonFilePath, jsonObject);
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Parse and modify config.json exist IOException: "
                    + e.getMessage()));
            throw new BundleException("Parse and modify config.json exist IOException: " + e.getMessage());
        }
        return util;
    }

    private boolean compressDirToHap(Path sourceDir, String zipFilePath)
            throws IOException, BundleException {
        Utility utility = new Utility();
        utility.setOutPath(zipFilePath);
        if (zipFilePath.endsWith(HAP_SUFFIX)) {
            utility.setMode(Utility.MODE_HAP);
        } else if (zipFilePath.endsWith(HSP_SUFFIX)) {
            utility.setMode(Utility.MODE_HSP);
        }
        try (Stream<Path> pathStream = Files.walk(sourceDir, 1)) {
            pathStream.forEach(path -> {
                String fileName = path.getFileName().toString();
                String filePath = path.toString();

                switch (fileName) {
                    case ETS_FILE_NAME:
                        utility.setEtsPath(filePath);
                        break;
                    case HNP_FILE_NAME:
                        utility.setHnpPath(filePath);
                        break;
                    case LIBS_DIR:
                        utility.setLibPath(filePath);
                        break;
                    case AN_FILE_NAME:
                        utility.setANPath(filePath);
                        break;
                    case AP_FILE_NAME:
                        utility.setAPPath(filePath);
                        break;
                    case RESOURCE_FILE_NAME:
                        utility.setResourcesPath(filePath);
                        break;
                    case JS_FILE_NAME:
                        utility.setJsPath(filePath);
                        break;
                    case ASSETS_FILE_NAME:
                        utility.setAssetsPath(filePath);
                        break;
                    case MAPLE_FILE_NAME:
                        utility.setSoDir(filePath);
                        break;
                    case SHARED_LIBS_FILE_NAME:
                        utility.setSharedLibsPath(filePath);
                        break;
                    case CONFIG_JSON:
                        utility.setJsonPath(filePath);
                        break;
                    case MODULE_JSON:
                        utility.setJsonPath(filePath);
                        break;
                    case RES_INDEX:
                        utility.setIndexPath(filePath);
                        break;
                    case PACKINFO_NAME:
                        utility.setPackInfoPath(filePath);
                        break;
                    case RPCID:
                        utility.setRpcid(filePath);
                        break;
                    case PKG_CONTEXT_INFO:
                        utility.setPkgContextPath(filePath);
                        break;
                }
            });
        }
        return compressProcess(utility);
    }

    private static void deleteDirectory(File dir) {
        if (dir.isDirectory()) {
            File[] children = dir.listFiles();
            if (children != null) {
                for (File child : children) {
                    deleteDirectory(child);
                }
            }
        }
        dir.delete();
    }

    private static void writeVersionRecord(List<VersionNormalizeUtil> utils, String outPath) {
        String jsonString = JSON.toJSONString(utils);
        try (FileWriter fileWriter = new FileWriter(outPath + LINUX_FILE_SEPARATOR + VERSION_RECORD)) {
            fileWriter.write(jsonString);
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Write version record exist IOException: "
                    + e.getMessage()));
        }
    }

    private static void verifyModuleVersion(VersionNormalizeUtil versionNormalizeUtil, Utility utility)
            throws BundleException {
        if (versionNormalizeUtil.getOriginVersionCode() > utility.getVersionCode()) {
            String errorMsg = "Input version code less than module " + versionNormalizeUtil.getModuleName()
                    + " version code.";
            LOG.error(PackingToolErrMsg.VERIFY_MODULE_VERSION_FAILED.toString(errorMsg));
            throw new BundleException(errorMsg);
        } else if (versionNormalizeUtil.getOriginVersionCode() == utility.getVersionCode()) {
            LOG.warning("versionNormalize warning: module " +
                    versionNormalizeUtil.getModuleName() + " version code not changed");
        }
        if (versionNormalizeUtil.getOriginVersionName().equals(utility.getVersionName())) {
            LOG.warning("versionNormalize warning: module " +
                    versionNormalizeUtil.getModuleName() + " version name not changed");
        }
    }

    private static void unpackHap(String srcPath, String outPath) throws BundleException {
        try (FileInputStream fis = new FileInputStream(srcPath);
             ZipInputStream zipInputStream = new ZipInputStream(new BufferedInputStream(fis))) {
            File destDir = new File(outPath);
            if (!destDir.exists()) {
                destDir.mkdirs();
            }

            ZipEntry entry;
            while ((entry = zipInputStream.getNextEntry()) != null) {
                String entryName = entry.getName();
                File entryFile = new File(outPath, entryName);

                if (entry.isDirectory()) {
                    entryFile.mkdirs();
                    zipInputStream.closeEntry();
                    continue;
                }
                File parent = entryFile.getParentFile();
                if (!parent.exists()) {
                    parent.mkdirs();
                }

                writeToFile(zipInputStream, entryFile);

                zipInputStream.closeEntry();
            }
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Unpack hap exist IOException: " + e.getMessage()));
            throw new BundleException("unpack hap failed IOException " + e.getMessage());
        }
    }

    private static void writeToFile(ZipInputStream zipInputStream, File entryFile) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(entryFile)) {
            byte[] buffer = new byte[BUFFER_SIZE];
            int bytesRead;
            while ((bytesRead = zipInputStream.read(buffer)) != -1) {
                fos.write(buffer, 0, bytesRead);
            }
        }
    }

    private static String getFileNameByPath(String path) {
        Path filePath = Paths.get(path);
        return filePath.getFileName().toString();
    }

    private void packEncryptJsonFile(Utility utility) throws BundleException {
        if (!utility.getEncryptPath().isEmpty()) {
            pathToFile(utility, utility.getEncryptPath(), NULL_DIR_NAME, false);
        } else {
            LOG.info("Compressor::packEncryptJsonFile has no encrypt.json");
        }
    }

    private void packPacJsonFile(Utility utility) throws BundleException {
        if (!utility.getPacJsonPath().isEmpty()) {
            pathToFile(utility, utility.getPacJsonPath(), NULL_DIR_NAME, false);
        } else {
            LOG.info("Compressor::packPacJsonFile has no pac.json");
        }
    }

    private void generalNormalize(Utility utility) {
        List<GeneralNormalizeUtil> utils = new ArrayList<>();
        Path tempDir = null;
        boolean isSuccess = true;
        String[] name = new String[2];
        for (String hapPath : utility.getFormattedHapList()) {
            try {
                GeneralNormalizeUtil util = new GeneralNormalizeUtil();
                tempDir = Files.createTempDirectory(Paths.get(utility.getOutPath()), "temp");
                unpackHap(hapPath, tempDir.toAbsolutePath().toString());
                File moduleFile = new File(
                        tempDir.toAbsolutePath() + LINUX_FILE_SEPARATOR + MODULE_JSON);
                File configFile = new File(
                        tempDir.toAbsolutePath() + LINUX_FILE_SEPARATOR + CONFIG_JSON);
                File packInfoFile = new File(
                        tempDir.toAbsolutePath() + LINUX_FILE_SEPARATOR + PACKINFO_NAME);

                if (moduleFile.exists() && configFile.exists()) {
                    LOG.error(PackingToolErrMsg.GENERAL_NORMALIZE_MODE_ARGS_INVALID .toString("Invalid hap structure"));
                    throw new BundleException("generalNormalize failed, invalid hap structure.");
                }
                if (moduleFile.exists()) {
                    String moduleJsonPath = tempDir.resolve(MODULE_JSON).toString();
                    util = parseAndModifyGeneralModuleJson(moduleJsonPath, utility, name);
                } else if (configFile.exists()) {
                    String configJsonPath = tempDir.resolve(CONFIG_JSON).toString();
                    util = parseAndModifyGeneralConfigJson(configJsonPath, utility, name);
                } else {
                    LOG.error(PackingToolErrMsg.GENERAL_NORMALIZE_MODE_ARGS_INVALID .toString("Invalid hap structure"));
                    throw new BundleException("generalNormalize failed, invalid hap structure.");
                }
                if (packInfoFile.exists()) {
                    String packInfoPath = tempDir.resolve(PACKINFO_NAME).toString();
                    parseAndModifyGeneralPackInfo(packInfoPath, utility);
                }
                utils.add(util);
                String modifiedHapPath = Paths.get(utility.getOutPath()) +
                        LINUX_FILE_SEPARATOR + Paths.get(hapPath).getFileName().toString();
                boolean ret = compressDirToHap(tempDir, modifiedHapPath);
                if (!ret) {
                    isSuccess = false;
                    String errMsg = "compressDirToHap failed bundleName:" + name[0] + " moduleName:" + name[1];
                    LOG.error(PackingToolErrMsg.GENERAL_NORMALIZE_MODE_ARGS_INVALID .toString(errMsg));
                    break;
                }
            } catch (Exception e) {
                String errMsg = "general normalize exist Exception bundleName:" + name[0] + " moduleName:" + name[1];
                LOG.error(PackingToolErrMsg.GENERAL_NORMALIZE_MODE_ARGS_INVALID .toString(errMsg + e.getMessage()));
                isSuccess = false;
                break;
            } finally {
                if (tempDir != null) {
                    deleteDirectory(tempDir.toFile());
                }
            }
        }
        if (!isSuccess) {
            if (Paths.get(utility.getOutPath()) != null) {
                deleteFile(Paths.get(utility.getOutPath()).toFile());
            }
            return;
        }
        writeGeneralRecord(utils, utility.getOutPath());
    }

    private GeneralNormalizeUtil parseAndModifyGeneralModuleJson(String jsonFilePath, Utility utility, String[] name)
            throws BundleException {
        GeneralNormalizeUtil util = new GeneralNormalizeUtil();
        try (FileInputStream jsonStream = new FileInputStream(jsonFilePath)) {
            JSONObject jsonObject = JSON.parseObject(jsonStream, JSONObject.class);
            if (!jsonObject.containsKey(APP)) {
                LOG.error(PackingToolErrMsg.PARSE_AND_MODIFY_MODULEJSON_FAILED.toString("The module.json file " +
                        "does not contain 'app'."));
                throw new BundleException("The module.json file does not contain 'app'. ");
            }
            JSONObject appObject = jsonObject.getJSONObject(APP);
            JSONObject moduleObject = jsonObject.getJSONObject(MODULE);
            if (!moduleObject.containsKey(NAME)) {
                LOG.error(PackingToolErrMsg.PARSE_AND_MODIFY_MODULEJSON_FAILED.toString("The module object of " +
                        "module.json file does not contain 'name'."));
                throw new BundleException("The module object of module.json file does not contain 'name'. ");
            }
            if (!appObject.containsKey(BUNDLE_NAME)) {
                LOG.error(PackingToolErrMsg.PARSE_AND_MODIFY_MODULEJSON_FAILED.toString("The app object of " +
                        "app.json file does not contain 'bundleName'."));
                throw new BundleException("The app object of app.json file does not contain 'bundleName'. ");
            }
            name[0] = appObject.getString(BUNDLE_NAME);
            name[1] = moduleObject.getString(NAME);
            util.setModuleName(moduleObject.getString(NAME));

            if (utility.getGeneralNormalizeList().contains(DEVICE_TYPES)) {
                util.setOriginDeviceTypes(getJsonString(moduleObject, DEVICE_TYPES));
                moduleObject.put(DEVICE_TYPES, utility.getDeviceTypes().split(","));
            }

            if (utility.getGeneralNormalizeList().contains(VERSION_CODE)) {
                util.setOriginVersionCode(appObject.getIntValue(VERSION_CODE));
                appObject.put(VERSION_CODE, utility.getVersionCode());
            }

            if (utility.getGeneralNormalizeList().contains(VERSION_NAME)) {
                util.setOriginVersionName(appObject.getString(VERSION_NAME));
                appObject.put(VERSION_NAME, utility.getVersionName());
            }

            if (utility.getGeneralNormalizeList().contains(BUNDLE_NAME)) {
                util.setOriginBundleName(appObject.getString(BUNDLE_NAME));
                appObject.put(BUNDLE_NAME, utility.getBundleName());
            }

            if (utility.getGeneralNormalizeList().contains(MIN_COMPATIBLE_VERSION_CODE)) {
                if (appObject.containsKey(MIN_COMPATIBLE_VERSION_CODE)) {
                    util.setOriginMinCompatibleVersionCode(appObject.getIntValue(MIN_COMPATIBLE_VERSION_CODE));
                } else {
                    util.setOriginMinCompatibleVersionCode(appObject.getIntValue(VERSION_CODE));
                }
                appObject.put(MIN_COMPATIBLE_VERSION_CODE, utility.getMinCompatibleVersionCode());
            }

            if (utility.getGeneralNormalizeList().contains(MIN_API_VERSION)) {
                util.setOriginMinAPIVersion(appObject.getIntValue(MIN_API_VERSION));
                appObject.put(MIN_API_VERSION, utility.getMinAPIVersion());
            }

            if (utility.getGeneralNormalizeList().contains(TARGET_API_VERSION)) {
                util.setOriginTargetAPIVersion(appObject.getIntValue(TARGET_API_VERSION));
                appObject.put(TARGET_API_VERSION, utility.getTargetAPIVersion());
            }

            if (utility.getGeneralNormalizeList().contains(API_RELEASE_TYPE)) {
                util.setOriginApiReleaseType(appObject.getString(API_RELEASE_TYPE));
                appObject.put(API_RELEASE_TYPE, utility.getApiReleaseType());
            }

            if (utility.getGeneralNormalizeList().contains(BUNDLE_TYPE)) {
                if (!appObject.containsKey(BUNDLE_TYPE)) {
                    if (moduleObject.getBoolean(INSTALLATION_FREE)) {
                        String errMsg =
                                "app.json5 file configuration does not match the 'installationFree' setting of true.";
                        String solution = "Add the bundleType field to the app.json5 file or set it atomicService.";
                        LOG.error(PackingToolErrMsg.PARSE_STAGE_BUNDLE_TYPE_FAILED.toString(errMsg, solution));
                        throw new BundleException(errMsg);
                    }
                    util.setOriginBundleType(APP);
                } else {
                    util.setOriginBundleType(appObject.getString(BUNDLE_TYPE));
                }
                appObject.put(BUNDLE_TYPE, utility.getBundleType());
            }

            if (utility.getGeneralNormalizeList().contains(INSTALLATION_FREE)) {
                util.setOriginInstallationFree(moduleObject.getBoolean(INSTALLATION_FREE));
                util.setIsInstallationFree(true);
                moduleObject.put(INSTALLATION_FREE, utility.getDeliveryWithInstall());
            }

            if (utility.getGeneralNormalizeList().contains(DELIVERY_WITH_INSTALL)) {
                util.setOriginDeliveryWithInstall(moduleObject.getBoolean(DELIVERY_WITH_INSTALL));
                util.setIsDeliveryWithInstall(true);
                moduleObject.put(DELIVERY_WITH_INSTALL, utility.getInstallationFree());
            }
            writeJson(jsonFilePath, jsonObject);
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Parse and modify module.json exist IOException: " +
                    e.getMessage()));
            throw new BundleException("Parse and modify module.json exist IOException: " + e.getMessage());
        }
        return util;
    }

    private GeneralNormalizeUtil parseAndModifyGeneralConfigJson(String jsonFilePath, Utility utility, String[] name)
            throws BundleException {
        GeneralNormalizeUtil util = new GeneralNormalizeUtil();
        try (FileInputStream jsonStream = new FileInputStream(jsonFilePath)) {
            JSONObject jsonObject = JSON.parseObject(jsonStream, JSONObject.class);
            if (!jsonObject.containsKey(APP)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The config.json file " +
                        "does not contain 'app'."));
                throw new BundleException("The config.json file does not contain 'app'. ");
            }
            JSONObject appObject = jsonObject.getJSONObject(APP);

            if (!appObject.containsKey(VERSION)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The app object of config.json " +
                        "file does not contain 'version'."));
                throw new BundleException("The app object of config.json file does not contain 'version'. ");
            }
            JSONObject versionObj = appObject.getJSONObject(VERSION);

            if (!appObject.containsKey(API_VERSION)) {
                JSONObject apiVersion = new JSONObject();
                appObject.put(API_VERSION, apiVersion);
            }
            JSONObject apiVersionObj = appObject.getJSONObject(API_VERSION);

            if (!jsonObject.containsKey(MODULE)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The app object of config.json " +
                        "file does not contain 'module'."));
                throw new BundleException("The app object of config.json file does not contain 'module'. ");
            }
            JSONObject moduleObject = jsonObject.getJSONObject(MODULE);

            if (!moduleObject.containsKey(DISTRO)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The app object of config.json " +
                        "file does not contain 'distro'."));
                throw new BundleException("The app object of config.json file does not contain 'distro'. ");
            }
            JSONObject distroObj = moduleObject.getJSONObject(DISTRO);

            if (!distroObj.containsKey(MODULE_NAME_NEW)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The module object of " +
                        "config.json file does not contain 'moduleName'."));
                throw new BundleException("The module object of module.json file does not contain 'moduleName'. ");
            }
            if (!appObject.containsKey(BUNDLE_NAME)) {
                LOG.error(PackingToolErrMsg.PARSE_MODIFY_CONFIG_JSON_FAILED.toString("The app object of " +
                        "config.json file does not contain 'bundleName'."));
                throw new BundleException("The app object of config.json file does not contain 'bundleName'. ");
            }
            name[0] = appObject.getString(BUNDLE_NAME);
            name[1] = distroObj.getString(MODULE_NAME_NEW);
            util.setModuleName(distroObj.getString(MODULE_NAME_NEW));

            if (utility.getGeneralNormalizeList().contains(DEVICE_TYPES)) {
                util.setOriginDeviceTypes(getJsonString(moduleObject, DEVICE_TYPES));
                moduleObject.put(DEVICE_TYPE, utility.getDeviceTypes().split(","));
            }
            if (utility.getGeneralNormalizeList().contains(VERSION_CODE)) {
                util.setOriginVersionCode(versionObj.getIntValue(CODE));
                versionObj.put(CODE, utility.getVersionCode());
            }

            if (utility.getGeneralNormalizeList().contains(VERSION_NAME)) {
                util.setOriginVersionName(versionObj.getString(NAME));
                versionObj.put(NAME, utility.getVersionName());
            }

            if (utility.getGeneralNormalizeList().contains(MIN_COMPATIBLE_VERSION_CODE)) {
                if (versionObj.containsKey(MIN_COMPATIBLE_VERSION_CODE)) {
                    util.setOriginMinCompatibleVersionCode(versionObj.getIntValue(MIN_COMPATIBLE_VERSION_CODE));
                } else {
                    util.setOriginMinCompatibleVersionCode(versionObj.getIntValue(CODE));
                }
                versionObj.put(MIN_COMPATIBLE_VERSION_CODE, utility.getMinCompatibleVersionCode());
            }

            if (utility.getGeneralNormalizeList().contains(BUNDLE_NAME)) {
                util.setOriginBundleName(appObject.getString(BUNDLE_NAME));
                appObject.put(BUNDLE_NAME, utility.getBundleName());
            }

            if (utility.getGeneralNormalizeList().contains(MIN_API_VERSION)) {
                util.setOriginMinAPIVersion(apiVersionObj.getIntValue(COMPATIBLE));
                apiVersionObj.put(COMPATIBLE, utility.getMinAPIVersion());
            }

            if (utility.getGeneralNormalizeList().contains(TARGET_API_VERSION)) {
                util.setOriginTargetAPIVersion(apiVersionObj.getIntValue(TARGET));
                apiVersionObj.put(TARGET, utility.getTargetAPIVersion());
            }

            if (utility.getGeneralNormalizeList().contains(API_RELEASE_TYPE)) {
                util.setOriginApiReleaseType(apiVersionObj.getString(RELEASE_TYPE));
                apiVersionObj.put(RELEASE_TYPE, utility.getApiReleaseType());
            }

            if (utility.getGeneralNormalizeList().contains(BUNDLE_TYPE)) {
                if (!appObject.containsKey(BUNDLE_TYPE)) {
                    if (distroObj.getBoolean(INSTALLATION_FREE)) {
                        util.setOriginBundleType(ATOMIC_SERVICE);
                    } else {
                        util.setOriginBundleType(APP);
                    }
                } else {
                    util.setOriginBundleType(appObject.getString(BUNDLE_TYPE));
                }
                appObject.put(BUNDLE_TYPE, utility.getBundleType());
            }

            if (utility.getGeneralNormalizeList().contains(INSTALLATION_FREE)) {
                util.setOriginInstallationFree(distroObj.getBoolean(INSTALLATION_FREE));
                util.setIsInstallationFree(true);
                distroObj.put(INSTALLATION_FREE, utility.getDeliveryWithInstall());
            }

            if (utility.getGeneralNormalizeList().contains(DELIVERY_WITH_INSTALL)) {
                util.setOriginDeliveryWithInstall(distroObj.getBoolean(DELIVERY_WITH_INSTALL));
                util.setIsDeliveryWithInstall(true);
                distroObj.put(DELIVERY_WITH_INSTALL, utility.getInstallationFree());
            }
            writeJson(jsonFilePath, jsonObject);
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Parse and modify module.json exist IOException: " +
                    e.getMessage()));
            throw new BundleException("Parse and modify config.json exist IOException: " + e.getMessage());
        }
        return util;
    }

    private void parseAndModifyGeneralPackInfo(String packInfoPath, Utility utility)
            throws BundleException {
        try (FileInputStream jsonStream = new FileInputStream(packInfoPath)) {
            JSONObject jsonObject = JSON.parseObject(jsonStream, JSONObject.class);
            if (jsonObject == null) {
                LOG.warning("parseAndModifyGeneralPackInfo failed, json format invalid.");
                return;
            }
            JSONObject summaryObject = jsonObject.getJSONObject(SUMMARY);
            if (summaryObject == null) {
                LOG.warning("parseAndModifyGeneralPackInfo failed, summary invalid.");
                return;
            }
            JSONObject appObject = summaryObject.getJSONObject(APP);
            if (appObject == null) {
                LOG.warning("parseAndModifyGeneralPackInfo failed, app invalid.");
                return;
            }
            JSONArray moduleJsonList = summaryObject.getJSONArray(MODULES);
            if (moduleJsonList.isEmpty()) {
                LOG.warning("parseAndModifyGeneralPackInfo failed, modules invalid.");
                return;
            }

            if (utility.getGeneralNormalizeList().contains(DEVICE_TYPES)) {
                for (int i = 0; i < moduleJsonList.size(); i++) {
                    JSONObject moduleJson = moduleJsonList.getJSONObject(i);
                    if (moduleJson == null) {
                        LOG.warning("parseAndModifyGeneralPackInfo failed, moduleJson invalid.");
                        continue;
                    }
                    moduleJson.put(DEVICE_TYPE, utility.getDeviceTypes().split(","));
                }
            }

            JSONObject versionObject = appObject.getJSONObject(VERSION);
            if (versionObject == null) {
                LOG.warning("parseAndModifyGeneralPackInfo failed, version invalid.");
                return;
            }

            if (utility.getGeneralNormalizeList().contains(VERSION_CODE)) {
                versionObject.put(CODE, utility.getVersionCode());
            }

            if (utility.getGeneralNormalizeList().contains(VERSION_NAME)) {
                versionObject.put(NAME, utility.getVersionName());
            }

            if (utility.getGeneralNormalizeList().contains(BUNDLE_NAME)) {
                appObject.put(BUNDLE_NAME, utility.getBundleName());
            }

            if (utility.getGeneralNormalizeList().contains(MIN_COMPATIBLE_VERSION_CODE)) {
                versionObject.put(MIN_COMPATIBLE_VERSION_CODE, utility.getMinCompatibleVersionCode());
            }

            if (utility.getGeneralNormalizeList().contains(MIN_API_VERSION)) {
                setApiVersion(moduleJsonList, MIN_API_VERSION, utility);
            }

            if (utility.getGeneralNormalizeList().contains(TARGET_API_VERSION)) {
                setApiVersion(moduleJsonList, TARGET_API_VERSION, utility);
            }

            if (utility.getGeneralNormalizeList().contains(API_RELEASE_TYPE)) {
                setApiVersion(moduleJsonList, API_RELEASE_TYPE, utility);
            }

            if (utility.getGeneralNormalizeList().contains(BUNDLE_TYPE)) {
                appObject.put(BUNDLE_TYPE, utility.getBundleType());
            }

            if (utility.getGeneralNormalizeList().contains(INSTALLATION_FREE)) {
                setDistroObj(moduleJsonList, INSTALLATION_FREE, utility);
            }

            if (utility.getGeneralNormalizeList().contains(DELIVERY_WITH_INSTALL)) {
                setDistroObj(moduleJsonList, DELIVERY_WITH_INSTALL, utility);
            }

            JSONArray jsonArray = jsonObject.getJSONArray(PACKAGES);
            if (jsonArray != null) {
                for (int i = 0; i < jsonArray.size(); i++) {
                    JSONObject object = jsonArray.getJSONObject(i);
                    if (utility.getGeneralNormalizeList().contains(DEVICE_TYPES)) {
                        object.put(DEVICE_TYPE, utility.getDeviceTypes().split(","));
                    }
                    if (utility.getGeneralNormalizeList().contains(DELIVERY_WITH_INSTALL)) {
                        object.put(DELIVERY_WITH_INSTALL, Boolean.parseBoolean(utility.getDeliveryWithInstall()));
                    }
                }
            }

            writeJson(packInfoPath, jsonObject);
        } catch (IOException e) {
            LOG.warning("parseAndModifyGeneralPackInfo failed, IOException." + e.getMessage());
        }
    }

    private static void setApiVersion(JSONArray moduleJsonList, String key, Utility utility) {
        for (int i = 0; i < moduleJsonList.size(); i++) {
            JSONObject moduleJson = moduleJsonList.getJSONObject(i);
            if (moduleJson == null) {
                LOG.warning("setApiVersion failed, moduleJson invalid.");
                break;
            }
            JSONObject apiVersionObj = moduleJson.getJSONObject(API_VERSION);
            if (apiVersionObj == null) {
                JSONObject apiVersion = new JSONObject();
                moduleJson.put(API_VERSION, apiVersion);
                apiVersionObj = moduleJson.getJSONObject(API_VERSION);
            }
            if(key == MIN_API_VERSION) {
                apiVersionObj.put(COMPATIBLE, utility.getMinAPIVersion());
            } else if (key == TARGET_API_VERSION) {
                apiVersionObj.put(TARGET, utility.getTargetAPIVersion());
            } else if (key == API_RELEASE_TYPE) {
                apiVersionObj.put(RELEASE_TYPE, utility.getApiReleaseType());
            }
        }
    }

    private static void setDistroObj(JSONArray moduleJsonList, String key, Utility utility) {
        for (int i = 0; i < moduleJsonList.size(); i++) {
            JSONObject moduleJson = moduleJsonList.getJSONObject(i);
            if (moduleJson == null) {
                LOG.warning("setDistroObj failed, moduleJson invalid.");
                break;
            }
            JSONObject distroObj = moduleJson.getJSONObject(DISTRO);
            if (distroObj == null) {
                LOG.warning("setDistroObj failed, distro invalid.");
                break;
            }
            if(key == INSTALLATION_FREE) {
                distroObj.put(INSTALLATION_FREE, Boolean.parseBoolean(utility.getInstallationFree()));
            } else if (key == DELIVERY_WITH_INSTALL) {
                distroObj.put(DELIVERY_WITH_INSTALL, Boolean.parseBoolean(utility.getDeliveryWithInstall()));
            }
        }
    }

    private static void writeGeneralRecord(List<GeneralNormalizeUtil> utils, String outPath) {
        JSONArray jsonArray = new JSONArray();
        try (FileWriter fileWriter = new FileWriter(outPath + LINUX_FILE_SEPARATOR + GENERAL_RECORD)) {
            for (GeneralNormalizeUtil util : utils) {
                JSONObject jsonObject = new JSONObject();
                if (util.originDeviceTypes != null && !util.originDeviceTypes.isEmpty()) {
                    jsonObject.put("deviceTypes", util.originDeviceTypes);
                    if (util.originDeviceTypes instanceof String) {
                        String arrayStr = (String) util.originDeviceTypes;
                        JSONArray deviceTypes = JSON.parseArray(arrayStr);
                        jsonObject.put("deviceTypes", deviceTypes);
                    }
                }
                if (util.originVersionCode != INVALID_VERSION) {
                    jsonObject.put("versionCode", util.originVersionCode);
                }
                if (util.moduleName != null && !util.moduleName.isEmpty()) {
                    jsonObject.put("moduleName", util.moduleName);
                }
                if (util.originVersionName != null && !util.originVersionName.isEmpty()) {
                    jsonObject.put("versionName", util.originVersionName);
                }
                if (util.originMinCompatibleVersionCode != INVALID_VERSION) {
                    jsonObject.put("minCompatibleVersionCode", util.originMinCompatibleVersionCode);
                }
                if (util.originMinAPIVersion != INVALID_VERSION) {
                    jsonObject.put("minAPIVersion", util.originMinAPIVersion);
                }
                if (util.originTargetAPIVersion != INVALID_VERSION) {
                    jsonObject.put("targetAPIVersion", util.originTargetAPIVersion);
                }
                if (util.originApiReleaseType!= null && !util.originApiReleaseType.isEmpty()) {
                    jsonObject.put("apiReleaseType", util.originApiReleaseType);
                }
                if (util.originBundleType != null && !util.originBundleType.isEmpty()) {
                    jsonObject.put("bundleType", util.originBundleType);
                }
                if (util.originBundleName != null && !util.originBundleName.isEmpty()) {
                    jsonObject.put("bundleName", util.originBundleName);
                }
                if (util.isInstallationFree == true) {
                    jsonObject.put("installationFree", util.originInstallationFree);
                }
                if (util.isDeliveryWithInstall == true) {
                    jsonObject.put("deliveryWithInstall", util.originDeliveryWithInstall);
                }
                jsonArray.add(jsonObject);
            }
            fileWriter.write(jsonArray.toString());
        } catch (IOException e) {
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString("Write general record exist IOException: "
                    + e.getMessage()));
        }
    }

    private static void deleteFile(File dir) {
        File[] children = dir.listFiles();
        if (children != null) {
            for (File child : children) {
                child.delete();
            }
        }
    }
}
