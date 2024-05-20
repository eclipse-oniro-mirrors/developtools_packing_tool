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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <regex>
#include <string>

#include "hap_packager.h"
#include "constants.h"
#include "contrib/minizip/zip.h"
#include "nlohmann/json.hpp"
#include "errors.h"
#include "packager.h"

namespace OHOS {
namespace AppPackingTool {
namespace {}

HapPackager::HapPackager(const std::map<std::string, std::string> &parameterMap, std::string &resultReceiver)
    : Packager(parameterMap, resultReceiver)
{}

ErrCode HapPackager::InitAllowedParam()
{
    allowedParameters_ = {
        {}
    };
    return OHOS::ERR_OK;
}

ErrCode HapPackager::PreProcess()
{
    return OHOS::ERR_OK;
}

ErrCode HapPackager::Process()
{
    std::cout << "Hap DoPackage" << std::endl;
    std::string outPath = parameterMap_.at(Constants::PARAM_OUT_PATH);
    zipFile zf = zipOpen64(outPath.c_str(), APPEND_STATUS_CREATE);
    if (zf == nullptr) {
        std::cout << "err zipOpen64 null" << std::endl;
        return OHOS::ERR_INVALID_VALUE;
    }
    zip_fileinfo fi = {};
    std::map<std::string, std::string>::const_iterator it = parameterMap_.find(Constants::PARAM_JSON_PATH);
    if (it != parameterMap_.end()) {
        if (ParseJsonFile(moduleJson, it->second)) {
            WriteStringToZip(zf, moduleJson.dump(), fs::path(Constants::MODULE_JSON), fi);
        }
        //std::cout << "zip jsonPath: " << it->second << std::endl;
        //std::cout << "zip jsonContent: " << moduleJson.dump() << std::endl;
    }
    it = parameterMap_.find(Constants::PARAM_LIB_PATH);
    if (it != parameterMap_.end()) {
        //std::cout << "zip libPath: " << it->second << std::endl;
        AddFileToZip(zf, fs::path(it->second), fs::path(Constants::LIB_PATH), fi);
    }
    it = parameterMap_.find(Constants::PARAM_RESOURCES_PATH);
    if (it != parameterMap_.end()) {
        //std::cout << "zip resourcesPath: " << it->second << std::endl;
        AddFileToZip(zf, fs::path(it->second), fs::path(Constants::RESOURCES_PATH), fi);
    }
    it = parameterMap_.find(Constants::PARAM_INDEX_PATH);
    if (it != parameterMap_.end()) {
        //std::cout << "zip indexPath: " << it->second << std::endl;
        AddFileToZip(zf, fs::path(it->second), fs::path(Constants::RESOURCES_INDEX), fi);
    }
    it = parameterMap_.find(Constants::PARAM_PACK_INFO_PATH);
    if (it != parameterMap_.end()) {
        //std::cout << "zip packInfoPath: " << it->second << std::endl;
        AddFileToZip(zf, fs::path(it->second), fs::path(Constants::PACK_INFO), fi);
    }
    it = parameterMap_.find(Constants::PARAM_ETS_PATH);
    if (it != parameterMap_.end()) {
        //std::cout << "zip etsPath: " << it->second << std::endl;
        AddFileToZip(zf, fs::path(it->second), fs::path(Constants::ETS_PATH), fi);
    }
    it = parameterMap_.find(Constants::PARAM_RPCID_PATH);
    if (it != parameterMap_.end()) {
        //std::cout << "zip rpcidPath: " << it->second << std::endl;
        AddFileToZip(zf, fs::path(it->second), fs::path(Constants::RPCID_SC), fi);
    }
    it = parameterMap_.find(Constants::PARAM_PKG_CONTEXT_PATH);
    if (it != parameterMap_.end()) {
        //std::cout << "zip pkgContextPath: " << it->second << std::endl;
        AddFileToZip(zf, fs::path(it->second), fs::path(Constants::PKG_CONTEXT_JSON), fi);
    }
    zipClose(zf, nullptr);
    return OHOS::ERR_OK;
}
ErrCode HapPackager::PostProcess()
{
    //std::cout << "Hap PostCheck" << std::endl;
    //resultReceiver_.append("Hap PostCheck\n");
    return OHOS::ERR_OK;
}

} // namespace AppPackingTool
} // namespace OHOS