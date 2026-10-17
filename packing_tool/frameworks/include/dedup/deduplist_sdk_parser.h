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

#ifndef DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_DEDUPLIST_SDK_PARSER_H
#define DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_DEDUPLIST_SDK_PARSER_H

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cJSON.h"
#include "json/pt_json.h"

namespace OHOS {
namespace AppPackingTool {

/**
 * API version.
 *
 * The app minAPIVersion packs two versions in one integer: the high part carries the SDK version and
 * the last three digits carry the api version number. Every api version is normalised to three
 * segments before comparing, where missing components are treated as 0.
 */
struct DedupApiVersion {
    std::vector<int32_t> parts;

    bool IsValid() const
    {
        return !parts.empty();
    }

    bool operator<(const DedupApiVersion& other) const;
    bool operator<=(const DedupApiVersion& other) const;
    bool operator==(const DedupApiVersion& other) const;

    // Parses a dotted three-component manifest library key; other forms are rejected
    static bool Parse(const std::string& text, DedupApiVersion& version);

    // Normalises an integer api version to three segments: a composite app value keeps the SDK version
    // from its high part and a plain value becomes the major segment, so app.minAPIVersion and
    // dedupMinApiVersion compare the same value
    static DedupApiVersion FromApiVersion(int32_t apiVersion);
};

// One compatibility group: the system-preset so sha256 of a manifest API version
struct DedupCompatGroup {
    DedupApiVersion apiVersion;
    std::string sha256;
};

// One manifest libraries element
struct DedupLibraryInfo {
    std::string name;
    std::string abi;
    std::vector<DedupCompatGroup> compatGroups;
};

// Match result: the compatibility group matched by the app so sha256
struct DedupSoMatch {
    std::string abi;
    std::string sha256;
};

/**
 * Parser for the deduplist_sdk.json manifest.
 *
 * Exposes dedupMinApiVersion, deviceType and the sha256 of every API version organised by
 * name / abi / api, and looks up the group matching an app so sha256.
 */
class DedupListSdkParser {
public:
    DedupListSdkParser() = default;
    ~DedupListSdkParser() = default;

    DedupListSdkParser(const DedupListSdkParser&) = delete;
    DedupListSdkParser& operator=(const DedupListSdkParser&) = delete;

    // Parses the deduplist_sdk.json file; returns false on failure
    bool ParseFile(const std::string& path);

    // Parses the deduplist_sdk.json content; returns false on failure
    bool ParseContent(const std::string& content);

    const DedupApiVersion& GetDedupMinApiVersion() const
    {
        return dedupMinApiVersion_;
    }

    const std::set<std::string>& GetDeviceTypes() const
    {
        return deviceTypes_;
    }

    const std::string& GetErrorMessage() const
    {
        return errorMessage_;
    }

    /**
     * Looks up the compatibility group matching an app side so.
     *
     * Only groups whose API version is <= compatibleSdkVersion are considered, so a group declared
     * for a lower API version stays visible to higher ones; a group matches when its sha256 equals
     * appSha256.
     * @return whether a match was found
     */
    bool FindMatch(const std::string& name, const std::string& abi, const std::string& appSha256,
        const DedupApiVersion& compatibleSdkVersion, DedupSoMatch& match) const;

private:
    bool ParseDeviceTypes(const std::unique_ptr<PtJson>& deviceTypesJson);
    bool ParseLibraries(const std::unique_ptr<PtJson>& librariesJson);
    bool ParseLibraryIdentity(const std::unique_ptr<PtJson>& libraryJson, DedupLibraryInfo& library);
    bool ParseLibrary(const std::unique_ptr<PtJson>& libraryJson);
    bool ParseCompatGroup(const std::string& apiKey, cJSON* groupNode, DedupCompatGroup& group);
    const DedupLibraryInfo* FindLibrary(const std::vector<DedupLibraryInfo>& libraries,
        const std::string& abi) const;
    bool SetError(const std::string& error);
    void Reset();

    DedupApiVersion dedupMinApiVersion_;
    std::set<std::string> deviceTypes_;
    std::map<std::string, std::vector<DedupLibraryInfo>> libraryIndex_;
    std::string errorMessage_;
};

}  // namespace AppPackingTool
}  // namespace OHOS

#endif  // DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_DEDUPLIST_SDK_PARSER_H
