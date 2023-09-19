/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import java.util.AbstractMap;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;

/**
 * command parser.
 *
 */
public class CommandParser {
    /**
     * Parses and returns the hap list that supports the device type.
     */
    public static final String PARSE_MODE_HAPLIST = "hap-list";

    /**
     * Parses and returns the information about the hap.
     */
    public static final String PARSE_MODE_HAPINFO = "hap-info";

    private static final String CMD_MODE = "--mode";
    private static final String CMD_JSON_PATH = "--json-path";
    private static final String CMD_PROFILE_PATH = "--profile-path";
    private static final String CMD_INDEX_PATH = "--index-path";
    private static final String CMD_JS_PATH = "--js-path";
    private static final String CMD_ETS_PATH = "--ets-path";
    private static final String CMD_RPCID_PATH = "--rpcid-path";
    private static final String CMD_RPCID = "--rpcid";
    private static final String CMD_SO_PATH = "--maple-so-path";
    private static final String CMD_SO_DIR = "--maple-so-dir";
    private static final String CMD_ABILITY_SO_PATH = "--ability-so-path";
    private static final String CMD_DEX_PATH = "--dex-path";
    private static final String CMD_ABC_PATH = "--abc-path";
    private static final String CMD_FILE_PATH = "--file-path";
    private static final String CMD_LIB_PATH = "--lib-path";
    private static final String CMD_RES_PATH = "--res-path";
    private static final String CMD_RESOURCES_PATH = "--resources-path";
    private static final String CMD_ASSETS_PATH = "--assets-path";
    private static final String CMD_APK_PATH = "--shell-apk-path";
    private static final String CMD_HAP_PATH = "--hap-path";
    private static final String CMD_APP_PATH = "--app-path";
    private static final String CMD_SIGNATURE_PATH = "--signature-path";
    private static final String CMD_CERTIFICATE_PATH = "--certificate-path";
    private static final String CMD_FORCE = "--force";
    private static final String CMD_OUT_PATH = "--out-path";
    private static final String CMD_PACK_INFO_PATH = "--pack-info-path";
    private static final String CMD_BIN_PATH = "--bin-path";
    private static final String CMD_JAR_PATH = "--jar-path";
    private static final String CMD_TXT_PATH = "--txt-path";
    private static final String CMD_HAR_PATH = "--har-path";
    private static final String CMD_HSP_PATH = "--hsp-path";
    private static final String CMD_PARSE_MODE = "--p";
    private static final String CMD_PACK_RES_PATH = "--pack-res-path";
    private static final String CMD_UNPACKAPK = "--unpackapk";
    private static final String CMD_UNPACK_CUT_ENTRY_APK = "--unpack-cut_entry";
    private static final String CMD_SHAREDLIBS_PATH = "--shared-libs-path";
    private static final String CMD_ENTRYCARD_PATH = "--entrycard-path";
    private static final String CMD_HAP_LIST = "--hap-list";
    private static final String CMD_HSP_LIST = "--hsp-list";
    private static final String CMD_APP_LIST = "--app-list";
    private static final String CMD_DIR_LIST = "--dir-list";
    private static final String CMD_HQF_LIST = "--hqf-list";
    private static final String CMD_APPQF_PATH = "--appqf-path";
    private static final String CMD_AN_PATH = "--an-path";
    private static final String CMD_AP_PATH = "--ap-path";
    private static final String MAIN_MODULE_LIMIT = "--main-module-limit";
    private static final String NORMAL_MODULE_LIMIT = "--normal-module-limit";
    private static final String TOTAL_LIMIT = "--total-limit";
    private static final String VERSION_CODE = "--version-code";
    private static final String VERSION_NAME = "--version-name";
    private static final int PARSE_MODE_VALUE_LENGTH = 2;
    private static final Log LOG = new Log(CommandParser.class.toString());
    private static final Map<String, Function<Map.Entry<Utility, String>, Boolean>> commandFuncs = new HashMap<>();

    static {
        initCommandFuncs();
    }

    private static void initCommandFuncs() {
        commandFuncs.put(CMD_MODE, entry -> {
            entry.getKey().setMode(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_JSON_PATH, entry -> {
            entry.getKey().setJsonPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_PROFILE_PATH, entry -> {
            entry.getKey().setProfilePath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_INDEX_PATH, entry -> {
            entry.getKey().setIndexPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_JS_PATH, entry -> {
            entry.getKey().setJsPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_ETS_PATH, entry -> {
            entry.getKey().setEtsPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_RPCID_PATH, entry -> {
            entry.getKey().setRpcidPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_RPCID, entry -> {
            entry.getKey().setRpcid(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_SO_PATH, entry -> {
            entry.getKey().setSoPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_SO_DIR, entry -> {
            entry.getKey().setSoDir(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_ABILITY_SO_PATH, entry -> {
            entry.getKey().setAbilitySoPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_DEX_PATH, entry -> {
            entry.getKey().setDexPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_ABC_PATH, entry -> {
            entry.getKey().setAbcPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_FILE_PATH, entry -> {
            entry.getKey().setFilePath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_LIB_PATH, entry -> {
            entry.getKey().setLibPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_RES_PATH, entry -> {
            entry.getKey().setResPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_RESOURCES_PATH, entry -> {
            entry.getKey().setResourcesPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_ASSETS_PATH, entry -> {
            entry.getKey().setAssetsPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_APK_PATH, entry -> {
            entry.getKey().setApkPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_HAP_PATH, entry -> {
            entry.getKey().setHapPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_APP_PATH, entry -> {
            entry.getKey().setAppPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_SIGNATURE_PATH, entry -> {
            entry.getKey().setSignaturePath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_CERTIFICATE_PATH, entry -> {
            entry.getKey().setCertificatePath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_FORCE, entry -> {
            entry.getKey().setForceRewrite(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_OUT_PATH, entry -> {
            entry.getKey().setOutPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_PACK_INFO_PATH, entry -> {
            entry.getKey().setPackInfoPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_BIN_PATH, entry -> {
            entry.getKey().setBinPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_JAR_PATH, entry -> {
            entry.getKey().setJarPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_TXT_PATH, entry -> {
            entry.getKey().setTxtPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_HAR_PATH, entry -> {
            entry.getKey().setHarPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_HSP_PATH, entry -> {
            entry.getKey().setHspPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_PACK_RES_PATH, entry -> {
            entry.getKey().setPackResPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_UNPACKAPK, entry -> {
            entry.getKey().setUnpackApk(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_UNPACK_CUT_ENTRY_APK, entry -> {
            entry.getKey().setUnpackCutEntryApk(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_SHAREDLIBS_PATH, entry -> {
            entry.getKey().setSharedLibsPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_ENTRYCARD_PATH, entry -> {
            entry.getKey().setEntryCardPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_HAP_LIST, entry -> {
            entry.getKey().setHapList(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_HSP_LIST, entry -> {
            entry.getKey().setHspList(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_APP_LIST, entry -> {
            entry.getKey().setAppList(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_DIR_LIST, entry -> {
            entry.getKey().setDirList(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_HQF_LIST, entry -> {
            entry.getKey().setHqfList(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_APPQF_PATH, entry -> {
            entry.getKey().setAPPQFPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_AN_PATH, entry -> {
            entry.getKey().setANPath(entry.getValue());
            return true;
        });
        commandFuncs.put(CMD_AP_PATH, entry -> {
            entry.getKey().setAPPath(entry.getValue());
            return true;
        });
        commandFuncs.put(MAIN_MODULE_LIMIT, entry -> {
            entry.getKey().setMainModuleLimit(entry.getValue());
            return true;
        });
        commandFuncs.put(NORMAL_MODULE_LIMIT, entry -> {
            entry.getKey().setNormalModuleLimit(entry.getValue());
            return true;
        });
        commandFuncs.put(TOTAL_LIMIT, entry -> {
            entry.getKey().setTotalLimit(entry.getValue());
            return true;
        });
        commandFuncs.put(VERSION_CODE, entry -> {
            entry.getKey().setVersionCode(Integer.parseInt(entry.getValue()));
            return true;
        });
        commandFuncs.put(VERSION_NAME, entry -> {
            entry.getKey().setVersionName(entry.getValue());
            return true;
        });
    }


    /**
     * judge args is null and enter parser.
     *
     * @param utility common data
     * @param args command line
     * @return commandParser if input valid
     */
    public static boolean commandParser(Utility utility, String[] args) {
        if (args == null) {
            LOG.error("CommandParser::commandParser args is null!");
            return false;
        }
        for (int i = 0; i < args.length - 1; ++i) {
            String key = args[i];
            String value = args[i + 1];
            Map.Entry<Utility, String> entry = new AbstractMap.SimpleEntry<>(utility, value);
            if (commandFuncs.get(key) != null) {
                commandFuncs.get(key).apply(entry);
                ++i;
            } else if (CMD_PARSE_MODE.equals(key)) {
                if (i + PARSE_MODE_VALUE_LENGTH >= args.length) {
                    LOG.error("input wrong number value for --p command");
                    return false;
                }
                utility.setParseMode(args[i + 1]);
                if (PARSE_MODE_HAPLIST.equals(utility.getParseMode())) {
                    utility.setDeviceType(args[i + PARSE_MODE_VALUE_LENGTH]);
                } else if (PARSE_MODE_HAPINFO.equals(utility.getParseMode())) {
                    utility.setHapName(args[i + PARSE_MODE_VALUE_LENGTH]);
                }
                i += PARSE_MODE_VALUE_LENGTH;
            } else {
                LOG.warning(key + " is invalid!");
            }
        }
        return true;
    }
}
