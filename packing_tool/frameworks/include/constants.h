/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_CONSTANTS_H
#define DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_CONSTANTS_H

#include <getopt.h>
#include <iostream>
#include <map>
#include <string>

namespace OHOS {
namespace AppPackingTool {
namespace Constants {
const std::string CMD_HELP = "help";
const std::string CMD_PACK = "pack";
const std::string CMD_UNPACK = "unpack";

const std::string MODE_HAP = "hap";
const std::string MODE_HSP = "hsp";
const std::string MODE_APP = "app";
const std::string MODE_HQF = "hqf";
const std::string MODE_APPQF = "appqf";
const std::string MODE_MULTIAPP = "multiApp";
const std::string MODE_VERSION_NORMALIZE = "versionNormalize";
const std::string MODE_PACKAGE_NORMALIZE = "packageNormalize";

const std::string PARAM_PREFIX = "--";
const std::string PARAM_MODE = "mode";
const std::string PARAM_JSON_PATH = "json-path";
const std::string PARAM_LIB_PATH = "lib-path";
const std::string PARAM_RESOURCES_PATH = "resources-path";
const std::string PARAM_INDEX_PATH = "index-path";
const std::string PARAM_PACK_INFO_PATH = "pack-info-path";
const std::string PARAM_PROFILE_PATH = "profile-path";
const std::string PARAM_ETS_PATH = "ets-path";
const std::string PARAM_OUT_PATH = "out-path";
const std::string PARAM_MAPLE_SO_PATH = "maple-so-path";
const std::string PARAM_MAPLE_SO_DIR = "maple-so-dir";
const std::string PARAM_RPCID_PATH = "rpcid-path";
const std::string PARAM_JS_PATH = "js-path";
const std::string PARAM_FORCE = "force";
const std::string PARAM_AN_PATH = "an-path";
const std::string PARAM_AP_PATH = "ap-path";
const std::string PARAM_DIR_LIST = "dir-list";
const std::string PARAM_COMPRESS_LEVEL = "compress-level";
const std::string PARAM_PKG_CONTEXT_PATH = "pkg-context-path";
const std::string PARAM_HAP_PATH = "hap-path";
const std::string PARAM_HSP_PATH = "hsp-path";
const std::string PARAM_SIGNATURE_PATH = "signature-path";
const std::string PARAM_CERTIFICATE_PATH = "certificate-path";
const std::string PARAM_HAP_LIST = "hap-list";
const std::string PARAM_HSP_LIST = "hsp-list";
const std::string PARAM_APP_LIST = "app-list";
const std::string PARAM_HQF_LIST = "hqf-list";
const std::string PARAM_INPUT_LIST = "input-list";
const std::string PARAM_VERSION_CODE = "version-code";
const std::string PARAM_VERSION_NAME = "version-name";
const std::string PARAM_APP_PATH = "app-path";
const std::string PARAM_RPCID = "rpcid";
const std::string PARAM_APPQF_PATH = "appqf-path";
const std::string PARAM_MAIN_MODULE_LIMIT = "main-module-limit";
const std::string PARAM_NORMAL_MODULE_LIMIT = "normal-module-limit";

const std::string MODULE_JSON = "module.json";
const std::string CONFIG_JSON = "config.json";
const std::string LIB_PATH = "libs";
const std::string RESOURCES_PATH = "resources";
const std::string RESOURCES_INDEX = "resources.index";
const std::string PACK_INFO = "pack.info";
const std::string ETS_PATH = "ets";
const std::string RPCID_SC = "rpcid.sc";
const std::string PKG_CONTEXT_JSON = "pkgContextInfo.json";

const std::string JSON_SUFFIX = ".json";
const std::string HAP_SUFFIX = ".hap";
const std::string HSP_SUFFIX = ".hsp";
const std::string APP_SUFFIX = ".app";

constexpr const char* SHORT_OPTIONS = "";
const struct option LONG_OPTIONS[] = {
    {PARAM_MODE.c_str(), required_argument, nullptr, 1},
    {PARAM_JSON_PATH.c_str(), required_argument, nullptr, 2},
    {PARAM_LIB_PATH.c_str(), required_argument, nullptr, 3},
    {PARAM_RESOURCES_PATH.c_str(), required_argument, nullptr, 4},
    {PARAM_INDEX_PATH.c_str(), required_argument, nullptr, 5},
    {PARAM_PACK_INFO_PATH.c_str(), required_argument, nullptr, 6},
    {PARAM_PROFILE_PATH.c_str(), required_argument, nullptr, 7},
    {PARAM_ETS_PATH.c_str(), required_argument, nullptr, 8},
    {PARAM_OUT_PATH.c_str(), required_argument, nullptr, 9},
    {PARAM_MAPLE_SO_PATH.c_str(), required_argument, nullptr, 10},
    {PARAM_MAPLE_SO_DIR.c_str(), required_argument, nullptr, 11},
    {PARAM_RPCID_PATH.c_str(), required_argument, nullptr, 13},
    {PARAM_JS_PATH.c_str(), required_argument, nullptr, 14},
    {PARAM_FORCE.c_str(), required_argument, nullptr, 15},
    {PARAM_AN_PATH.c_str(), required_argument, nullptr, 16},
    {PARAM_AP_PATH.c_str(), required_argument, nullptr, 17},
    {PARAM_DIR_LIST.c_str(), required_argument, nullptr, 18},
    {PARAM_COMPRESS_LEVEL.c_str(), required_argument, nullptr, 19},
    {PARAM_PKG_CONTEXT_PATH.c_str(), required_argument, nullptr, 20},
    {PARAM_HAP_PATH.c_str(), required_argument, nullptr, 21},
    {PARAM_HSP_PATH.c_str(), required_argument, nullptr, 22},
    {PARAM_SIGNATURE_PATH.c_str(), required_argument, nullptr, 23},
    {PARAM_CERTIFICATE_PATH.c_str(), required_argument, nullptr, 24},
    {PARAM_HAP_LIST.c_str(), required_argument, nullptr, 25},
    {PARAM_HSP_LIST.c_str(), required_argument, nullptr, 26},
    {PARAM_APP_LIST.c_str(), required_argument, nullptr, 27},
    {PARAM_HQF_LIST.c_str(), required_argument, nullptr, 28},
    {PARAM_INPUT_LIST.c_str(), required_argument, nullptr, 29},
    {PARAM_VERSION_CODE.c_str(), required_argument, nullptr, 30},
    {PARAM_VERSION_NAME.c_str(), required_argument, nullptr, 31},
    {PARAM_APP_PATH.c_str(), required_argument, nullptr, 32},
    {PARAM_RPCID.c_str(), required_argument, nullptr, 33},
    {PARAM_APPQF_PATH.c_str(), required_argument, nullptr, 34},
    {PARAM_MAIN_MODULE_LIMIT.c_str(), required_argument, nullptr, 35},
    {PARAM_NORMAL_MODULE_LIMIT.c_str(), required_argument, nullptr, 36},
    {nullptr, 0, nullptr, 0},
};
constexpr const int32_t OPTIONS_SIZE = sizeof(LONG_OPTIONS) / sizeof(LONG_OPTIONS[0]);

enum PARAM_TYPE {
    DEFAULT,
    REGULAR_FILE,
    REGULAR_FILE_ARRAY,
    DIRECTORY,
    DIRECTORY_ARRAY,
    FILE_OR_DIRECTORY,
    FILE_OR_DIRECTORY_ARRAY,
};

constexpr const int32_t BUF_SIZE = 1024 * 4;
}
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_CONSTANTS_H