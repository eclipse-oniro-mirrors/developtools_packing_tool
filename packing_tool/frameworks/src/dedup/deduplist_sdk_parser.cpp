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

#include "dedup/deduplist_sdk_parser.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <utility>

#include "dedup/dedup_log.h"
#include "utils.h"

namespace OHOS {
namespace AppPackingTool {
namespace {
const std::string JSON_DEDUP_MIN_API_VERSION = "dedupMinApiVersion";
const std::string JSON_DEVICE_TYPE = "deviceType";
const std::string JSON_LIBRARIES = "libraries";
const std::string JSON_NAME = "name";
const std::string JSON_ABI = "abi";

const size_t API_VERSION_PARTS = 3;
const int32_t COMPOSITE_API_VERSION_MIN = 10000000;

// Composite app API version layout: a leading scale plus major / minor / patch segments
const int32_t COMPOSITE_VERSION_SCALE = 1000;
const int32_t VERSION_MAJOR_DIVISOR = 10000;
const int32_t VERSION_SEGMENT_DIVISOR = 100;
const int32_t VERSION_SEGMENT_MODULUS = 100;
}  // namespace

bool DedupApiVersion::Parse(const std::string& text, DedupApiVersion& version)
{
    version.parts.clear();
    if (text.empty() || text.back() == '.') {
        return false;
    }
    std::vector<int32_t> parts;
    std::stringstream stream(text);
    std::string part;
    while (std::getline(stream, part, '.')) {
        if (part.empty() || !Utils::IsPositiveInteger(part, 0, std::numeric_limits<int32_t>::max())) {
            return false;
        }
        parts.push_back(std::stoi(part));
    }
    if (parts.size() != API_VERSION_PARTS) {
        return false;
    }
    version.parts = std::move(parts);
    return true;
}

DedupApiVersion DedupApiVersion::FromApiVersion(int32_t apiVersion)
{
    DedupApiVersion version;
    if (apiVersion <= 0) {
        return version;
    }
    if (apiVersion >= COMPOSITE_API_VERSION_MIN) {
        // A composite app api version packs major / minor / patch in its high part
        const int32_t sdkVersion = apiVersion / COMPOSITE_VERSION_SCALE;
        version.parts.push_back(sdkVersion / VERSION_MAJOR_DIVISOR);
        version.parts.push_back((sdkVersion / VERSION_SEGMENT_DIVISOR) % VERSION_SEGMENT_MODULUS);
        version.parts.push_back(sdkVersion % VERSION_SEGMENT_MODULUS);
    } else {
        // A plain api version is the major segment
        version.parts.push_back(apiVersion);
        version.parts.push_back(0);
        version.parts.push_back(0);
    }
    return version;
}

bool DedupApiVersion::operator<(const DedupApiVersion& other) const
{
    size_t size = std::max(parts.size(), other.parts.size());
    for (size_t index = 0; index < size; ++index) {
        int32_t left = index < parts.size() ? parts[index] : 0;
        int32_t right = index < other.parts.size() ? other.parts[index] : 0;
        if (left != right) {
            return left < right;
        }
    }
    return false;
}

bool DedupApiVersion::operator<=(const DedupApiVersion& other) const
{
    return !(other < *this);
}

bool DedupApiVersion::operator==(const DedupApiVersion& other) const
{
    return !(*this < other) && !(other < *this);
}

bool DedupListSdkParser::SetError(const std::string& error)
{
    // The caller logs the failure once, with the wrapped inter-app dedup error code
    errorMessage_ = error;
    return false;
}

void DedupListSdkParser::Reset()
{
    dedupMinApiVersion_ = DedupApiVersion();
    deviceTypes_.clear();
    libraryIndex_.clear();
    errorMessage_.clear();
}

bool DedupListSdkParser::ParseFile(const std::string& path)
{
    Reset();
    if (path.empty()) {
        return SetError("deduplist sdk path is empty");
    }
    if (!Utils::IsFile(path)) {
        return SetError("deduplist sdk file does not exist: " + path);
    }
    std::string content = Utils::GetFileContent(path);
    if (content.empty()) {
        return SetError("deduplist sdk file is empty or unreadable: " + path);
    }
    return ParseContent(content);
}

bool DedupListSdkParser::ParseContent(const std::string& content)
{
    Reset();
    if (content.empty()) {
        return SetError("deduplist sdk content is empty");
    }
    std::unique_ptr<PtJson> root = PtJson::Parse(content);
    if (!root || !root->IsObject()) {
        return SetError("deduplist sdk json must be an object");
    }
    int32_t minApiVersionValue = 0;
    if (root->GetInt(JSON_DEDUP_MIN_API_VERSION.c_str(), &minApiVersionValue) != Result::SUCCESS ||
        minApiVersionValue <= 0) {
        return SetError("deduplist sdk json missing or invalid field dedupMinApiVersion");
    }
    std::unique_ptr<PtJson> deviceTypesJson;
    if (root->GetArray(JSON_DEVICE_TYPE.c_str(), &deviceTypesJson) != Result::SUCCESS) {
        return SetError("deduplist sdk field deviceType must be an array");
    }
    if (deviceTypesJson->GetSize() == 0) {
        return SetError("deduplist sdk field deviceType must not be empty");
    }
    if (!ParseDeviceTypes(deviceTypesJson)) {
        return false;
    }
    std::unique_ptr<PtJson> librariesJson;
    if (root->GetArray(JSON_LIBRARIES.c_str(), &librariesJson) != Result::SUCCESS) {
        return SetError("deduplist sdk field libraries must be an array");
    }
    if (librariesJson->GetSize() == 0) {
        return SetError("deduplist sdk field libraries must not be empty");
    }
    if (!ParseLibraries(librariesJson)) {
        return false;
    }
    dedupMinApiVersion_ = DedupApiVersion::FromApiVersion(minApiVersionValue);
    return true;
}

bool DedupListSdkParser::ParseDeviceTypes(const std::unique_ptr<PtJson>& deviceTypesJson)
{
    int32_t size = deviceTypesJson->GetSize();
    for (int32_t index = 0; index < size; ++index) {
        std::unique_ptr<PtJson> item = deviceTypesJson->Get(index);
        if (!item || !item->IsString() || item->GetString().empty()) {
            return SetError("deduplist sdk field deviceType must contain non-empty strings");
        }
        deviceTypes_.insert(item->GetString());
    }
    return true;
}

bool DedupListSdkParser::ParseLibraries(const std::unique_ptr<PtJson>& librariesJson)
{
    int32_t size = librariesJson->GetSize();
    for (int32_t index = 0; index < size; ++index) {
        std::unique_ptr<PtJson> libraryJson = librariesJson->Get(index);
        if (!libraryJson || !libraryJson->IsObject()) {
            return SetError("deduplist sdk libraries element must be an object");
        }
        if (!ParseLibrary(libraryJson)) {
            return false;
        }
    }
    return true;
}

bool DedupListSdkParser::ParseLibraryIdentity(const std::unique_ptr<PtJson>& libraryJson,
    DedupLibraryInfo& library)
{
    if (libraryJson->GetString(JSON_NAME.c_str(), &library.name) != Result::SUCCESS) {
        return SetError("deduplist sdk library field name invalid or missing");
    }
    if (library.name.empty()) {
        return SetError("deduplist sdk library field name must not be empty");
    }
    if (libraryJson->GetString(JSON_ABI.c_str(), &library.abi) != Result::SUCCESS) {
        return SetError("deduplist sdk library field abi invalid or missing");
    }
    if (library.abi.empty()) {
        return SetError("deduplist sdk library field abi must not be empty");
    }
    return true;
}

bool DedupListSdkParser::ParseLibrary(const std::unique_ptr<PtJson>& libraryJson)
{
    DedupLibraryInfo library;
    if (!ParseLibraryIdentity(libraryJson, library)) {
        return false;
    }
    // Every remaining key of the library is an API version mapped to a sha256
    cJSON* node = libraryJson->GetJson();
    for (cJSON* item = (node == nullptr ? nullptr : node->child); item != nullptr; item = item->next) {
        if (item->string == nullptr) {
            continue;
        }
        std::string key = item->string;
        if (key == JSON_NAME || key == JSON_ABI) {
            continue;
        }
        DedupApiVersion apiVersion;
        if (!DedupApiVersion::Parse(key, apiVersion)) {
            return SetError("deduplist sdk library has invalid api version key: " + key);
        }
        DedupCompatGroup group;
        group.apiVersion = apiVersion;
        if (!ParseCompatGroup(key, item, group)) {
            return false;
        }
        library.compatGroups.push_back(std::move(group));
    }
    if (library.compatGroups.empty()) {
        return SetError("deduplist sdk library " + library.name +
            " must define at least one compatibility group");
    }
    libraryIndex_[library.name].push_back(std::move(library));
    return true;
}

bool DedupListSdkParser::ParseCompatGroup(const std::string& apiKey, cJSON* groupNode, DedupCompatGroup& group)
{
    if (groupNode == nullptr || cJSON_IsString(groupNode) == 0 || groupNode->valuestring == nullptr ||
        groupNode->valuestring[0] == '\0') {
        return SetError("deduplist sdk compatibility group " + apiKey + " must map a non-empty sha256 string");
    }
    group.sha256 = groupNode->valuestring;
    return true;
}

const DedupLibraryInfo* DedupListSdkParser::FindLibrary(
    const std::vector<DedupLibraryInfo>& libraries, const std::string& abi) const
{
    for (const auto& library : libraries) {
        if (library.abi == abi) {
            return &library;
        }
    }
    return nullptr;
}

bool DedupListSdkParser::FindMatch(const std::string& name, const std::string& abi, const std::string& appSha256,
    const DedupApiVersion& compatibleSdkVersion, DedupSoMatch& match) const
{
    if (!compatibleSdkVersion.IsValid()) {
        LOG(DEBUG) << "So deduplication for sdk skip: compatibleSdkVersion is invalid, so=" << name;
        return false;
    }
    auto indexIter = libraryIndex_.find(name);
    if (indexIter == libraryIndex_.end()) {
        LOG(DEBUG) << "So deduplication for sdk skip: so is not listed in deduplist sdk, so=" << name;
        return false;
    }
    const DedupLibraryInfo* library = FindLibrary(indexIter->second, abi);
    if (library == nullptr) {
        LOG(DEBUG) << "So deduplication for sdk skip: so abi is not listed in deduplist sdk, so=" << name
                   << ", abi=" << abi;
        return false;
    }
    // The first group not above compatibleSdkVersion with the same sha256 wins (groups are sorted ascending)
    for (const auto& group : library->compatGroups) {
        if (group.apiVersion <= compatibleSdkVersion && group.sha256 == appSha256) {
            match.abi = abi;
            match.sha256 = group.sha256;
            return true;
        }
    }
    LOG(DEBUG) << "So deduplication for sdk skip: no compatibility group matched for so=" << name << ", abi=" << abi;
    return false;
}

}  // namespace AppPackingTool
}  // namespace OHOS
