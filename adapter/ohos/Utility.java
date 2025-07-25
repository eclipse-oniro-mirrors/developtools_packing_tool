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

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * common resource.
 *
 */
public class Utility {
    static final String MODE_HAP = "hap";
    static final String MODE_HAR = "har";
    static final String MODE_APP = "app";
    static final String MODE_FAST_APP = "fastApp";
    static final String MODE_MULTI_APP = "multiApp";
    static final String MODE_HQF = "hqf";
    static final String MODE_APPQF = "appqf";
    static final String MODE_RES = "res";
    static final String MODE_HSP = "hsp";
    static final String MODE_HAPADDITION = "hapAddition";
    static final String VERSION_NORMALIZE = "versionNormalize";
    static final String PACKAGE_NORMALIZE = "packageNormalize";
    static final String GENERAL_NORMALIZE = "generalNormalize";
    static final String FALSE_STRING = "false";
    static final String TRUE_STRING = "true";

    private static final String INVALID_PATH = "invalid";
    private static final String CMD_PREFIX = "--";
    private static final Log LOG = new Log(Utility.class.toString());

    private String mode = "";
    private String jsonPath = "";
    private String profilePath = "";
    private String soPath = "";
    private String soDir = "";
    private String abilitySoPath = "";
    private String dexPath = "";
    private String abcPath = "";
    private String indexPath = "";
    private String hapPath = "";
    private String appPath = "";
    private String libPath = "";
    private String resPath = "";
    private String resourcesPath = "";
    private String assetsPath = "";
    private String apkPath = "";
    private String certificatePath = "";
    private String signaturePath = "";
    private String forceRewrite = "false";
    private String outPath = "";
    private String packInfoPath = "";
    private String encryptPath = "";
    private String pacJsonPath = "";
    private String binPath = "";
    private boolean isCompressNativeLibs = false;
    private String moduleName = "";
    private String harPath = "";
    private String hspPath = "";
    private String jarPath = "";
    private String txtPath = "";
    private String parseMode = "";
    private String deviceType = "";
    private String hapName = "";
    private String sharedLibsPath = "";
    private String unpackApk = "false";
    private String unpackCutEntryApk = "false";
    private String entryCardPath = "";
    private String packRes = "";
    private String packResPath = "";
    private String filePath = "";
    private String jsPath = "";
    private String etsPath = "";
    private String hnpPath = "";
    private String rpcidPath = "";
    private int VersionCode = -1;
    private String VersionName = "";
    private boolean isModuleJson = false;
    private String rpcid = FALSE_STRING;
    private String libs = FALSE_STRING;
    private String cpuAbi = "";
    private boolean isParse = false;
    private String appqfPath = "";
    private String anPath = "";
    private String apPath = "";
    private Boolean isSharedApp = false;
    private Boolean isAppService = false;
    private String mainModuleLimit = "";
    private String normalModuleLimit = "";
    private String totalLimit = "";
    private String atomicServiceEntrySizeLimit = "";
    private String atomicServiceNonEntrySizeLimit = "";
    private int compressLevel = 1;
    private String pkgContextPath = "";
    private String bundleName = "";
    private String absoluteHapPath = "";
    private boolean generateBuildHash = false;
    private boolean buildHashFinish = false;
    private int minCompatibleVersionCode = -1;
    private int minAPIVersion = -1;
    private String compileSdkType = "";
    private String compileSdkVersion = "";
    private int targetAPIVersion = -1;
    private String apiReleaseType = "";
    private String bundleType = "";
    private String installationFree = "";
    private String deliveryWithInstall = "";
    private String deviceTypes = "";
    private List<String> generalNormalizeList = new ArrayList<>();

    private List<String> formattedCpuAbiList = new ArrayList<>();
    private List<String> formattedSoPathList = new ArrayList<>();
    private List<String> formattedAbilitySoPathList = new ArrayList<>();
    private List<String> formattedDexPathList = new ArrayList<>();
    private List<String> formattedAbcPathList = new ArrayList<>();
    private List<String> formattedHapPathList = new ArrayList<>();
    private List<String> formattedHspPathList = new ArrayList<>();
    private List<String> formattedApkPathList = new ArrayList<>();
    private List<String> formattedJarPathList = new ArrayList<>();
    private List<String> formattedTxtPathList = new ArrayList<>();
    private List<String> formattedEntryCardPathList = new ArrayList<>();
    private List<String> formNameList = new ArrayList<>();
    private List<String> formattedAppList = new ArrayList<>();
    private List<String> formattedHapList = new ArrayList<>();

    private List<String> formatedDirList = new ArrayList<>();

    private List<String> formatedHQFList = new ArrayList<>();
    private List<String> formatedABCList = new ArrayList<>();

    private String appList = "";
    private String hapList = "";
    private String dirList = "";
    private String hqfList = "";
    private String hspList = "";
    private String inputList = "";
    private String input = "";
    private boolean statDuplicate = false;
    private boolean statSuffix = false;
    private String statFileSize = "";
    private boolean isSuccess = true;

    public void setIsParse(boolean isParse) {
        this.isParse = isParse;
    }

    public boolean getIsParse() {
        return isParse;
    }

    public String getMode() {
        return mode;
    }

    public void setMode(String mode) {
        if (!mode.startsWith(CMD_PREFIX)) {
            this.mode = mode;
        }
    }

    public String getJsonPath() {
        return jsonPath;
    }

    public void setJsonPath(String jsonPath) {
        if (!jsonPath.startsWith(CMD_PREFIX)) {
            this.jsonPath = getFormattedPath(jsonPath);
        }
    }

    public String getProfilePath() {
        return profilePath;
    }

    public void setProfilePath(String profilePath) {
        if (!profilePath.startsWith(CMD_PREFIX)) {
            this.profilePath = getFormattedPath(profilePath);
        }
    }

    public String getSoPath() {
        return soPath;
    }

    public void setSoPath(String soPath) {
        if (!soPath.startsWith(CMD_PREFIX)) {
            this.soPath = soPath;
        }
    }

    public String getAbilitySoPath() {
        return abilitySoPath;
    }

    public void setAbilitySoPath(String abilitySoPath) {
        if (!abilitySoPath.startsWith(CMD_PREFIX)) {
            this.abilitySoPath = abilitySoPath;
        }
    }

    public String getSoDir() {
        return soDir;
    }

    public void setSoDir(String soDir) {
        if (!soDir.startsWith(CMD_PREFIX)) {
            this.soDir = soDir;
        }
    }

    public String getDexPath() {
        return dexPath;
    }

    public void setDexPath(String dexPath) {
        if (!dexPath.startsWith(CMD_PREFIX)) {
            this.dexPath = dexPath;
        }
    }

    public String getAbcPath() {
        return abcPath;
    }

    public void setAbcPath(String abcPath) {
        if (!abcPath.startsWith(CMD_PREFIX)) {
            this.abcPath = abcPath;
        }
    }

    public List<String> getABCList() {
        return formatedABCList;
    }

    public String getFilePath() {
        return filePath;
    }

    public void setFilePath(String filePath) {
        if (!filePath.startsWith(CMD_PREFIX)) {
            this.filePath = filePath;
        }
    }

    public String getIndexPath() {
        return indexPath;
    }

    public void setIndexPath(String indexPath) {
        if (!indexPath.startsWith(CMD_PREFIX)) {
            this.indexPath = getFormattedPath(indexPath);
        }
    }

    public String getBinPath() {
        return binPath;
    }

    public void setBinPath(String binPath) {
        if (!binPath.startsWith(CMD_PREFIX)) {
            this.binPath = getFormattedPath(binPath);
        }
    }

    public void setIsModuleJson(boolean isModuleJson) {
        this.isModuleJson = isModuleJson;
    }

    public boolean getIsModuleJson() {
        return isModuleJson;
    }

    public boolean isCompressNativeLibs() {
        return isCompressNativeLibs;
    }

    public void setIsCompressNativeLibs(boolean isCompress) {
        this.isCompressNativeLibs = isCompress;
    }

    public String getHapPath() {
        return hapPath;
    }

    public void setHapPath(String hapPath) {
        if (!hapPath.startsWith(CMD_PREFIX)) {
            this.hapPath = hapPath;
            if (MODE_HAPADDITION.equals(this.getMode())) {
                this.absoluteHapPath = getFormattedPath(hapPath);
            }
        }
    }

    public String getAppPath() {
        return appPath;
    }

    public void setAppPath(String appPath) {
        if (!appPath.startsWith(CMD_PREFIX)) {
            this.appPath = appPath;
        }
    }

    public String getLibPath() {
        return libPath;
    }

    public void setLibPath(String libPath) {
        if (!libPath.startsWith(CMD_PREFIX)) {
            this.libPath = getFormattedPath(libPath);
        }
    }

    public String getResPath() {
        return resPath;
    }

    public void setResPath(String resPath) {
        if (!resPath.startsWith(CMD_PREFIX)) {
            this.resPath = getFormattedPath(resPath);
        }
    }

    public String getResourcesPath() {
        return resourcesPath;
    }

    public void setResourcesPath(String resourcesPath) {
        if (!resourcesPath.startsWith(CMD_PREFIX)) {
            this.resourcesPath = getFormattedPath(resourcesPath);
        }
    }

    public String getAssetsPath() {
        return assetsPath;
    }

    public void setAssetsPath(String assetsPath) {
        if (!assetsPath.startsWith(CMD_PREFIX)) {
            this.assetsPath = getFormattedPath(assetsPath);
        }
    }

    public String getModuleName() {
        if (moduleName == null) {
            moduleName = "";
        }
        return moduleName;
    }

    public void setModuleName(String moduleName) {
        this.moduleName = moduleName;
    }

    public String getApkPath() {
        return apkPath;
    }

    public void setApkPath(String apkPath) {
        if (!apkPath.startsWith(CMD_PREFIX)) {
            this.apkPath = apkPath;
        }
    }

    public String getCertificatePath() {
        return certificatePath;
    }

    public void setCertificatePath(String certificatePath) {
        if (!certificatePath.startsWith(CMD_PREFIX)) {
            this.certificatePath = getFormattedPath(certificatePath);
        }
    }

    public String getSignaturePath() {
        return signaturePath;
    }

    public void setSignaturePath(String signaturePath) {
        if (!signaturePath.startsWith(CMD_PREFIX)) {
            this.signaturePath = getFormattedPath(signaturePath);
        }
    }

    public String getForceRewrite() {
        return forceRewrite;
    }

    public void setForceRewrite(String forceRewrite) {
        if (!forceRewrite.startsWith(CMD_PREFIX)) {
            this.forceRewrite = forceRewrite.toLowerCase(Locale.ENGLISH);
        }
    }

    public String getOutPath() {
        return outPath;
    }

    public void setOutPath(String outPath) {
        if (!outPath.startsWith(CMD_PREFIX)) {
            this.outPath = getFormattedPath(outPath);
        }
    }

    public String getPackInfoPath() {
        return packInfoPath;
    }

    public void setPackInfoPath(String packInfoPath) {
        if (!packInfoPath.startsWith(CMD_PREFIX)) {
            this.packInfoPath = getFormattedPath(packInfoPath);
        }
    }

    public String getEncryptPath() {
        return encryptPath;
    }

    public void setEncryptPath(String encryptPath) {
        if (!encryptPath.startsWith(CMD_PREFIX)) {
            this.encryptPath = getFormattedPath(encryptPath);
        }
    }

    public String getPacJsonPath() {
        return pacJsonPath;
    }

    public void setPacJsonPath(String pacJsonPath) {
        this.pacJsonPath = pacJsonPath;
    }

    public String getJarPath() {
        return jarPath;
    }

    public void setJarPath(String jarPath) {
        if (!jarPath.startsWith(CMD_PREFIX)) {
            this.jarPath = jarPath;
        }
    }

    public String getTxtPath() {
        return txtPath;
    }

    public void setTxtPath(String txtPath) {
        if (!txtPath.startsWith(CMD_PREFIX)) {
            this.txtPath = txtPath;
        }
    }

    public String getHarPath() {
        return harPath;
    }

    public void setHarPath(String harPath) {
        if (!harPath.startsWith(CMD_PREFIX)) {
            this.harPath = harPath;
        }
    }

    public void setHspPath(String hspPath) {
        if (!hspPath.startsWith(CMD_PREFIX)) {
            this.hspPath = hspPath;
        }
    }

    public String getHspPath() {
        return hspPath;
    }

    public String getParseMode() {
        return parseMode;
    }

    public void setParseMode(String parseMode) {
        if (!parseMode.startsWith(CMD_PREFIX)) {
            this.parseMode = parseMode;
        }
    }

    public String getDeviceType() {
        return deviceType;
    }

    public void setDeviceType(String deviceType) {
        if (!deviceType.startsWith(CMD_PREFIX)) {
            this.deviceType = deviceType;
        }
    }

    public String getUnpackApk() {
        return unpackApk;
    }

    public void setUnpackApk(String unpackApk) {
        this.unpackApk = unpackApk;
    }

    public String getUnpackCutEntryApk() {
        return unpackCutEntryApk;
    }

    public void setUnpackCutEntryApk(String unpackCutEntryApk) {
        this.unpackCutEntryApk = unpackCutEntryApk;
    }

    public String getHapName() {
        return hapName;
    }

    public void setHapName(String hapName) {
        if (!hapName.startsWith(CMD_PREFIX)) {
            this.hapName = hapName;
        }
    }

    public String getSharedLibsPath() {
        return sharedLibsPath;
    }

    public void setSharedLibsPath(String sharedLibsPath) {
        if (!sharedLibsPath.startsWith(CMD_PREFIX)) {
            this.sharedLibsPath = sharedLibsPath;
        }
    }

    public String getEntryCardPath() {
        return entryCardPath;
    }

    public void setEntryCardPath(String entryCardPath) {
        if (!entryCardPath.startsWith(CMD_PREFIX)) {
            this.entryCardPath = entryCardPath;
        }
    }

    public String getPackRes() {
        return packRes;
    }

    public void setPackRes(String packRes) {
        this.packRes = packRes;
    }

    public String getPackResPath() {
        return packResPath;
    }

    public void setPackResPath(String packResPath) {
        this.packResPath = packResPath;
    }

    public String getJsPath() { return jsPath; }

    public void setJsPath(String jsPath) { this.jsPath = jsPath; }

    public String getEtsPath() { return etsPath; }

    public void setEtsPath(String etsPath) { this.etsPath = etsPath; }

    public String getHnpPath() { return hnpPath; }

    public void setHnpPath(String hnpPath) { this.hnpPath = hnpPath; }

    public String getRpcidPath() {
        return rpcidPath;
    }

    public void setRpcidPath(String rpcidPath) {
        this.rpcidPath = rpcidPath;
    }

    public String getRpcid() { return rpcid; }

    public void setRpcid(String rpcid) {
        this.rpcid = rpcid;
    }

    public String getLibs() {
        return libs;
    }

    public void setLibs(String libs) {
        this.libs = libs;
    }

    public String getCpuAbis() {
        return cpuAbi;
    }

    public void setCpuAbis(String cpuAbi) {
        this.cpuAbi = cpuAbi;
    }

    public String getVersionName() {
        return VersionName;
    }

    public void setVersionName(String VersionName) {
        this.VersionName = VersionName;
    }

    public int getVersionCode() {
        return VersionCode;
    }

    public void setVersionCode(int VersionCode) {
        this.VersionCode = VersionCode;
    }

    public List<String> getFormattedCpuAbiList() {
        return formattedCpuAbiList;
    }

    public List<String> getFormattedSoPathList() {
        return formattedSoPathList;
    }

    public List<String> getFormattedAbilitySoPathList() {
        return formattedAbilitySoPathList;
    }

    public List<String> getFormattedDexPathList() {
        return formattedDexPathList;
    }

    public List<String> getFormattedAbcPathList() {
        return formattedAbcPathList;
    }

    public List<String> getFormattedHapPathList() {
        return formattedHapPathList;
    }

    public List<String> getFormattedHspPathList() {
        return formattedHspPathList;
    }

    public List<String> getFormattedApkPathList() {
        return formattedApkPathList;
    }

    public List<String> getFormattedJarPathList() {
        return formattedJarPathList;
    }

    public List<String> getFormattedTxtPathList() {
        return formattedTxtPathList;
    }

    public List<String> getFormattedAppList() {
        return formattedAppList;
    }

    public List<String> getFormattedHapList() {
        return formattedHapList;
    }

    public void setAppList(String appList) {
        this.appList = appList;
    }

    public String getAppList() {
        return this.appList;
    }

    public void setHapList(String hapList) {
        this.hapList = hapList;
    }

    public String getHapList() {
        return this.hapList;
    }

    public String getDirList() {
        return this.dirList;
    }

    public void setDirList(String dirList) {
        this.dirList = dirList;
    }

    public List<String> getFormatedDirList() {
        return this.formatedDirList;
    }

    /**
     * get dEntryCard path
     *
     * @return formattedEntryCardPathList
     */
    public List<String> getformattedEntryCardPathList() {
        return formattedEntryCardPathList;
    }

    /**
     * get canonical path
     *
     * @param path path input
     * @return formatted path
     */
    public String getFormattedPath(String path) {
        if (path == null) {
            return "";
        }

        File file = new File(path);
        String canonicalPath;
        try {
            canonicalPath = file.getCanonicalPath();
        } catch (IOException exception) {
            canonicalPath = INVALID_PATH;
            LOG.error(PackingToolErrMsg.IO_EXCEPTION.toString(
                    "Get formatted path exist IOException: " + exception.getMessage()));
        }
        return canonicalPath;
    }

    /**
     * close stream.
     *
     * @param stream stream to close
     */
    static void closeStream(Closeable stream) {
        if (stream != null) {
            try {
                stream.close();
            } catch (IOException exception) {
                LOG.error(PackingToolErrMsg.CLOSE_STREAM_EXCEPTION.toString(
                    "Close stream exist IOException: " + exception.getMessage()));
            }
        }
    }

    /**
     * Add form name.
     *
     * @param formName the name of form
     */
    public void addFormNameList(String formName) {
        this.formNameList.add(formName);
    }

    /**
     * Get all form names
     *
     * @return all form names
     */
    public List<String> getFormNameList() {
        return formNameList;
    }

    public String getHqfList() {
        return hqfList;
    }

    public void setHqfList(String hqfList) {
        this.hqfList = hqfList;
    }

    public List<String> getFormatedHQFList() {
        return formatedHQFList;
    }

    public void setFormatedHQFList(List<String> formatedHQFList) {
        this.formatedHQFList = formatedHQFList;
    }

    public void setAPPQFPath(String appqfPath) {
        this.appqfPath = appqfPath;
    }

    public String getAPPQFPath() {
        return this.appqfPath;
    }

    public void setANPath(String anPath) {
        this.anPath = anPath;
    }

    public String getANPath() {
        return this.anPath;
    }

    public void setAPPath(String apPath) {
        this.apPath = apPath;
    }

    public String getAPPath() {
        return this.apPath;
    }

    public String getAbsoluteHapPath() {
        return this.absoluteHapPath;
    }

    public void setAbsoluteHapPath(String absoluteHapPath) {
        if (!absoluteHapPath.startsWith(CMD_PREFIX)) {
            this.absoluteHapPath = getFormattedPath(absoluteHapPath);
        }
    }

    public void setIsSharedApp(boolean isSharedApp) {
        this.isSharedApp = isSharedApp;
    }

    public Boolean getSharedApp() {
        return isSharedApp;
    }

    public void setIsAppService(boolean isAppService) {
        this.isAppService = isAppService;
    }

    public Boolean getIsAppService() {
        return isAppService;
    }

    public String getMainModuleLimit() {
        return mainModuleLimit;
    }

    public void setMainModuleLimit(String limit) {
        this.mainModuleLimit = limit;
    }

    public String getNormalModuleLimit() {
        return normalModuleLimit;
    }

    public void setNormalModuleLimit(String limit) {
        this.normalModuleLimit = limit;
    }

    public String getAtomicServiceTotalSizeLimit() {
        return totalLimit;
    }

    public void setAtomicServiceTotalSizeLimit(String limit) {
        this.totalLimit = limit;
    }

    public String getAtomicServiceEntrySizeLimit() {
        return atomicServiceEntrySizeLimit;
    }

    public void setAtomicServiceEntrySizeLimit(String limit) {
        this.atomicServiceEntrySizeLimit = limit;
    }

    public String getAtomicServiceNonEntrySizeLimit() {
        return atomicServiceNonEntrySizeLimit;
    }

    public void setAtomicServiceNonEntrySizeLimit(String limit) {
        this.atomicServiceNonEntrySizeLimit = limit;
    }

    public int getCompressLevel() {
        return compressLevel;
    }

    public void setCompressLevel(int compressLevel) {
        this.compressLevel = compressLevel;
    }

    public String getPkgContextPath() {
        return pkgContextPath;
    }

    public void setPkgContextPath(String pkgContextPath) {
        this.pkgContextPath = pkgContextPath;
    }

    public String getBundleName() {
        return bundleName;
    }

    public void setBundleName(String bundleName) {
        this.bundleName = bundleName;
    }

    public String getHspList() {
        return hspList;
    }

    public void setHspList(String hspList) {
        this.hspList = hspList;
    }

    public void setGenerateBuildHash(boolean generateBuildHash) {
        this.generateBuildHash = generateBuildHash;
    }

    public boolean getGenerateBuildHash() {
        return generateBuildHash;
    }

    public void setBuildHashFinish(boolean buildHashFinish) {
        this.buildHashFinish = buildHashFinish;
    }

    public boolean isBuildHashFinish() {
        return buildHashFinish;
    }

    public String getInputList() {
        return inputList;
    }

    public void setInputList(String inputList) {
        this.inputList = inputList;
    }

    public String getInput() {
        return input;
    }

    public void setInput(String input) {
        this.input = getFormattedPath(input);
    }

    public boolean getStatDuplicate() {
        return statDuplicate;
    }

    public void setStatDuplicate(boolean statDuplicate) {
        this.statDuplicate = statDuplicate;
    }

    public boolean getStatSuffix() {
        return statSuffix;
    }

    public void setStatSuffix(boolean statSuffix) {
        this.statSuffix = statSuffix;
    }
    public String getStatFileSize() {
        return statFileSize;
    }

    public void setStatFileSize(String statFileSize) {
        this.statFileSize = statFileSize;
    }

    public int getMinCompatibleVersionCode() {
        return minCompatibleVersionCode;
    }

    public void setMinCompatibleVersionCode(int minCompatibleVersionCode) {
        this.minCompatibleVersionCode = minCompatibleVersionCode;
    }

    public int getMinAPIVersion() {
        return minAPIVersion;
    }

    public void setMinAPIVersion(int minAPIVersion) {
        this.minAPIVersion = minAPIVersion;
    }

    public int getTargetAPIVersion() {
        return targetAPIVersion;
    }

    public void setTargetAPIVersion(int targetAPIVersion) {
        this.targetAPIVersion = targetAPIVersion;
    }

    public String getApiReleaseType() {
        return apiReleaseType;
    }

    public void setApiReleaseType(String apiReleaseType) {
        this.apiReleaseType = apiReleaseType;
    }

    public String getBundleType() {
        return bundleType;
    }

    public void setBundleType(String bundleType) {
        this.bundleType = bundleType;
    }

    public String getDeliveryWithInstall() {
        return deliveryWithInstall;
    }

    public void setDeliveryWithInstall(String deliveryWithInstall) {
        this.deliveryWithInstall = deliveryWithInstall;
    }

    public String getInstallationFree() {
        return installationFree;
    }

    public void setInstallationFree(String installationFree) {
        this.installationFree = installationFree;
    }

    public String getDeviceTypes() {
        return deviceTypes;
    }

    public void setDeviceTypes(String deviceTypes) {
        this.deviceTypes = deviceTypes;
    }

    public void addGeneralNormalizeList(String generalNormalize) {
        this.generalNormalizeList.add(generalNormalize);
    }

    public List<String> getGeneralNormalizeList() {
        return generalNormalizeList;
    }

    public Boolean getParameterIsInvalid() {
        return isSuccess;
    }

    public void setParameterIsInvalid(Boolean isSuccess) {
        this.isSuccess = isSuccess;
    }
}
