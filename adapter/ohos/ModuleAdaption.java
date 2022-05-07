package ohos;

import jdk.internal.util.xml.impl.Pair;
import ohos.utils.fastjson.JSONObject;

import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;

class ModuleAdaption {

    private static final Log LOG = new Log(ModuleAdaption.class.toString());
    /**
     * convert moduleResult to uncompressResult function.
     *
     * @param moduleResult for module.json result
     * @return  UncomperssResult
     */
    UncomperssResult convertToUncompressResult(ModuleResult moduleResult) {
        UncomperssResult uncomperssResult = new UncomperssResult();
        uncomperssResult.setResult(moduleResult.getResult());
        uncomperssResult.setMessage(moduleResult.getMessage());
        // adapt profileInfos
        for (int i = 0; i < moduleResult.moduleProfileInfos.size(); ++i) {
            ModuleProfileInfo moduleProfileInfo = moduleResult.moduleProfileInfos.get(i);
            ProfileInfo profileInfo = new ProfileInfo();
            profileInfo.hapName = moduleProfileInfo.hapName;
            // adapt appInfo
            profileInfo.appInfo = convertToAppInfo(moduleProfileInfo.moduleAppInfo);
            //adapt deviceconfig
            profileInfo.deviceConfig = convertToDeviceConfig(moduleProfileInfo.moduleAppInfo);
            // adapt hapInfo
            profileInfo.hapInfo = convertToHapInfo(moduleProfileInfo.moduleInfo);
            uncomperssResult.addProfileInfo(profileInfo);
        }
        for (ModuleProfileInfo profileInfo : moduleResult.moduleProfileInfos) {
            if (profileInfo.moduleAppInfo != null) {
                uncomperssResult.setLabel(profileInfo.moduleAppInfo.label);
                uncomperssResult.setIcon(profileInfo.moduleAppInfo.icon);
            }
        }
        return uncomperssResult;
    }

    /**
     * convert ModuleAppInfo to AppInfo function.
     *
     * @param moduleAppInfo for module.json app result
     * @return  AppInfo
     */
    AppInfo convertToAppInfo(ModuleAppInfo moduleAppInfo) {
        AppInfo appInfo = new AppInfo();
        if (moduleAppInfo == null) {
            LOG.error("convertToAppInfo failed: moduleAppInfo is null");
            return appInfo;
        }
        appInfo.bundleName = moduleAppInfo.bundleName;
        appInfo.debug = moduleAppInfo.debug;
        appInfo.icon = moduleAppInfo.icon;
        appInfo.label = moduleAppInfo.label;
        appInfo.description = moduleAppInfo.description;
        appInfo.vendor = moduleAppInfo.vendor;
        appInfo.versionCode = String.valueOf(moduleAppInfo.versionCode);
        appInfo.versionName = moduleAppInfo.versionName;
        appInfo.minCompatibleVersionCode = moduleAppInfo.minCompatibleVersionCode;
        appInfo.compatibleApiVersion = moduleAppInfo.minAPIVersion;
        appInfo.targetApiVersion = moduleAppInfo.targetAPIVersion;
        appInfo.releaseType = moduleAppInfo.apiReleaseType;
        appInfo.distributedNotificationEnabled = moduleAppInfo.distributedNotificationEnabled;
        appInfo.appName = moduleAppInfo.appName;
        appInfo.appNameEN = moduleAppInfo.appNameEN;
        return appInfo;
    }

    /**
     * convert moduleAppInfo deviceTypes to DeviceConfig function.
     *
     * @param moduleAppInfo for module.json app result
     * @return Map<String, DeviceConfig>
     */
    Map<String, DeviceConfig> convertToDeviceConfig(ModuleAppInfo moduleAppInfo) {
        Map<String, DeviceConfig> deviceConfigMap = new HashMap<>();
        if (moduleAppInfo == null) {
            return deviceConfigMap;
        }
        for(Map.Entry<String, ModuleDeviceType> entrty : moduleAppInfo.deviceTypes.entrySet()) {
            DeviceConfig deviceConfig = new DeviceConfig();
            deviceConfig.distributedNotificationEnabled = entrty.getValue().distributedNotificationEnabled;
            deviceConfigMap.put(entrty.getKey(), deviceConfig);
        }
        return deviceConfigMap;
    }

    /**
     * convert moduleInfo to HapInfo function.
     *
     * @param moduleInfo for module.json module result
     * @return HapInfo
     */
    HapInfo convertToHapInfo(ModuleInfo moduleInfo) {
        HapInfo hapInfo = new HapInfo();
        if (moduleInfo == null) {
            LOG.error("convertToHapInfo failed: moduleInfo is null");
            return hapInfo;
        }
        hapInfo.name = moduleInfo.name;
        hapInfo.distro = new Distro();
        hapInfo.distro.moduleName = moduleInfo.name;
        hapInfo.distro.moduleType = moduleInfo.type;
        hapInfo.distro.installationFree = moduleInfo.installationFree;
        hapInfo.distro.virtualMachine = moduleInfo.virtualMachine;
        hapInfo.distro.deliveryWithInstall = moduleInfo.deliveryWithInstall;
        hapInfo.srcEntrance = moduleInfo.srcEntrance;
        hapInfo.description = moduleInfo.description;
        hapInfo.process = moduleInfo.process;
        hapInfo.mainElement = moduleInfo.mainElement;
        hapInfo.deviceType = moduleInfo.deviceTypes;
        hapInfo.uiSyntax = moduleInfo.uiSyntax;
        // convert pages
        hapInfo.pages = moduleInfo.pages;
        // convert moduleshortcut to short
        hapInfo.shortcuts = convertToShortcut(moduleInfo.moduleShortcuts);
        // convert metadata
        hapInfo.metaData = convertToMetadata(moduleInfo.moduleMetadataInfos);
        // convert abilities
        hapInfo.abilities = convertToAbilityInfo(moduleInfo.abilities);
        // convert extension abilities to abilities
        hapInfo.extensionAbilityInfos = moduleInfo.extensionAbilityInfos;
        // convert request permissions
        hapInfo.reqPermissions = moduleInfo.requestPermissions;
        // convert distrofilter
        hapInfo.distroFilter = moduleInfo.distroFilter;
        hapInfo.formInfos = moduleInfo.abilityFormInfos;
        hapInfo.commonEvents = moduleInfo.commonEvents;
        hapInfo.definePermissions = moduleInfo.definePermissions;
        return hapInfo;
    }

    /**
     * convert moduleShortcuts to HapInfo Shortcut.
     *
     * @param moduleShortcuts for module.json shortcut  result
     * @return List<Shortcut>
     */
    List<Shortcut> convertToShortcut(List<ModuleShortcut> moduleShortcuts) {
        List<Shortcut> shortcuts = new ArrayList<>();
        for (ModuleShortcut value : moduleShortcuts) {
            Shortcut shortcut = new Shortcut();
            shortcut.shortcutId = value.shortcutId;
            shortcut.label = value.label;
            shortcut.icon = value.icon;
            shortcut.intents = new ArrayList<>();
            for (int j = 0; j < value.wants.size(); ++j) {
                Want want = value.wants.get(j);
                IntentInfo intentInfo = new IntentInfo();
                intentInfo.targetBundle = want.bundleName;
                intentInfo.targetClass = want.abilityName;
                shortcut.intents.add(intentInfo);
            }
            shortcuts.add(shortcut);
        }
        return shortcuts;
    }

    /**
     * convert metadataInfos to MetaData function.
     *
     * @param metadataInfos for module.json metadata  result
     * @return MetaData
     */
    MetaData convertToMetadata(List<ModuleMetadataInfo> metadataInfos) {
        MetaData metaData = new MetaData();
        List<CustomizeData> customizeDatas = new ArrayList<>();
        for (ModuleMetadataInfo metadataInfo : metadataInfos) {
            CustomizeData customizeData = new CustomizeData();
            customizeData.name = metadataInfo.name;
            customizeData.value = metadataInfo.value;
            customizeData.extra = metadataInfo.resource;
            customizeDatas.add(customizeData);
        }
        metaData.customizeDatas = customizeDatas;
        return metaData;
    }

    /**
     * convert ModuleAbilityInfo to AbilityInfo function.
     *
     * @param moduleAbilityInfos for module.json ability  result
     * @return List<AbilityInfo>
     */
    List<AbilityInfo> convertToAbilityInfo(List<ModuleAbilityInfo> moduleAbilityInfos) {
        List<AbilityInfo> abilityInfos = new ArrayList<>();
        for (ModuleAbilityInfo info : moduleAbilityInfos) {
            AbilityInfo abilityInfo = new AbilityInfo();
            abilityInfo.type = "page";
            abilityInfo.name = info.name;
            abilityInfo.srcEntrance = info.srcEntrance;
            abilityInfo.description = info.description;
            abilityInfo.launchType = info.launchType;
            abilityInfo.icon = info.icon;
            abilityInfo.iconPath = info.icon;
            abilityInfo.label = info.label;
            abilityInfo.labelRes = info.label;
            abilityInfo.permissions = info.permissions;
            abilityInfo.visible = info.visible;
            abilityInfo.continuable = info.continuable;
            // convert metadata
            abilityInfo.metaData = convertToMetadata(info.metadata);
            // convert skillInfo
            abilityInfo.skills = info.skills;
            // convert configchanges
            abilityInfo.backgroundModes = info.backgroundModes;
            abilityInfos.add(abilityInfo);
        }
        return abilityInfos;
    }
}
