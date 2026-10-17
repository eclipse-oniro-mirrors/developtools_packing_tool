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

#include "dedup/inter_app_deduplicator.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <map>
#include <regex>
#include <sstream>
#include <vector>

#include "cJSON.h"
#include "dedup/dedup_error.h"
#include "dedup/dedup_log.h"
#include "dedup/inter_app_dedup_solver.h"
#include "dedup/module_calculator.h"
#include "json/module_json.h"
#include "utils.h"

namespace OHOS {
namespace AppPackingTool {
namespace {
const std::string RESULT_FILE_VERSION = "1.0";
const std::string RESULT_FILE_DESCRIPTION = "SO files removed during packaging";
const std::string KEY_VERSION = "version";
const std::string KEY_GENERATED_TIME = "generatedTime";
const std::string KEY_DESCRIPTION = "description";
const std::string KEY_REMOVED_SO_FILES = "removedSoFiles";
const std::string KEY_ORIGINAL_PATH_IN_HAP = "originalPathInHap";
const std::string KEY_ORIGINAL_SHA256 = "originalSha256";
const std::string IN_PACKAGE_LIB_PREFIX = "libs/";

cJSON* BuildRemovedSoItem(const InterAppRemovedSo& entry)
{
    cJSON* item = cJSON_CreateObject();
    if (item == nullptr ||
        cJSON_AddStringToObject(item, KEY_ORIGINAL_PATH_IN_HAP.c_str(), entry.relativePath.c_str()) == nullptr ||
        cJSON_AddStringToObject(item, KEY_ORIGINAL_SHA256.c_str(), entry.sha256.c_str()) == nullptr) {
        cJSON_Delete(item);
        return nullptr;
    }
    return item;
}

}  // namespace

bool InterAppDeduplicator::IsSoFileName(const std::string& fileName)
{
    try {
        static const std::regex soPattern(".*\\.so(\\.\\d+)*", std::regex_constants::icase);
        return std::regex_match(fileName, soPattern);
    } catch (const std::exception& exception) {
        LOG(WARNING) << "Failed to match so file name: " << exception.what();
        return false;
    }
}

bool InterAppDeduplicator::BuildPackContext(const std::shared_ptr<ModuleJson>& moduleJson,
    InterAppPackContext& context)
{
    context = InterAppPackContext();
    if (!moduleJson) {
        LOG(WARNING) << "module.json model is null for inter-app dedup";
        return false;
    }
    ModuleApiVersion moduleApiVersion;
    if (moduleJson->GetStageModuleApiVersion(moduleApiVersion) && moduleApiVersion.compatibleApiVersion > 0) {
        context.gateContext.minApiVersion = moduleApiVersion.compatibleApiVersion;
    }
    bool hasDriverExtensionAbility = false;
    if (moduleJson->GetStageHasDriverExtensionAbility(hasDriverExtensionAbility)) {
        context.gateContext.hasDriverExtensionAbility = hasDriverExtensionAbility;
    }
    std::map<std::string, std::string> resourceMap;
    context.moduleConfig = ModuleCalculator::ExtractModuleConfig(moduleJson, true, resourceMap);
    context.applicable = true;
    return true;
}

std::vector<InterAppModuleCandidate> InterAppDeduplicator::CollectLibCandidates(const std::string& libDir,
    const InterAppPackContext& context, const std::map<std::string, std::string>& sha256Cache,
    const std::set<std::string>& removedByIntraApp) const
{
    std::vector<InterAppModuleCandidate> modules;
    if (libDir.empty() || !std::filesystem::is_directory(libDir)) {
        LOG(DEBUG) << "So deduplication for sdk skipped: lib directory is missing or not a directory: " << libDir;
        return modules;
    }
    InterAppModuleCandidate module;
    module.moduleName = context.moduleConfig.moduleName.empty() ? libDir : context.moduleConfig.moduleName;
    module.config = context.moduleConfig;
    // Compatibility group matching uses the module's own normalised minAPIVersion
    module.matchApiVersion = context.gateContext.GetApiVersion();
    std::error_code errorCode;
    for (const auto& abiEntry : std::filesystem::directory_iterator(libDir, errorCode)) {
        if (errorCode || !abiEntry.is_directory()) {
            continue;
        }
        CollectSoCandidates(abiEntry.path(), abiEntry.path().filename().string(), sha256Cache, removedByIntraApp,
            module);
    }
    if (!module.sos.empty()) {
        modules.push_back(std::move(module));
    }
    return modules;
}

void InterAppDeduplicator::CollectSoCandidates(const std::filesystem::path& abiDir, const std::string& abi,
    const std::map<std::string, std::string>& sha256Cache, const std::set<std::string>& removedByIntraApp,
    InterAppModuleCandidate& module) const
{
    std::error_code errorCode;
    for (const auto& soEntry : std::filesystem::recursive_directory_iterator(abiDir, errorCode)) {
        if (errorCode || !soEntry.is_regular_file()) {
            continue;
        }
        std::string soName = soEntry.path().filename().string();
        if (!IsSoFileName(soName)) {
            continue;
        }
        // The so may sit under any sub directory of libs/<abi>: keep the complete package relative path
        std::string subPath = std::filesystem::relative(soEntry.path(), abiDir, errorCode).generic_string();
        if (errorCode || subPath.empty()) {
            continue;
        }
        std::string relativePath = IN_PACKAGE_LIB_PREFIX + abi + "/" + subPath;
        // Already removed by the intra-app stage: do not hash, match or record it again
        if (removedByIntraApp.find(relativePath) != removedByIntraApp.end()) {
            LOG(DEBUG) << "So deduplication for sdk skips so already removed by the intra-app stage: " << relativePath;
            continue;
        }
        std::string sha256;
        auto cacheIt = sha256Cache.find(relativePath);
        if (cacheIt != sha256Cache.end()) {
            sha256 = cacheIt->second;
        } else {
            sha256 = Utils::GetSha256File(soEntry.path().string());
        }
        if (sha256.empty()) {
            LOG(WARNING) << "Failed to calculate sha256 for " << soEntry.path().string();
            continue;
        }
        InterAppSoCandidate candidate;
        candidate.soName = soName;
        candidate.abi = abi;
        candidate.relativePath = relativePath;
        candidate.sha256 = sha256;
        module.sos.push_back(std::move(candidate));
    }
}

InterAppDedupStageResult InterAppDeduplicator::ComputeRemovals(const DedupListSdkParser& dedupList,
    const std::string& libDir, const InterAppPackContext& context,
    const std::map<std::string, std::string>& sha256Cache, const std::set<std::string>& removedByIntraApp)
{
    InterAppDedupStageResult result;
    InterAppDedupSolver solver;
    std::vector<InterAppModuleCandidate> modules = CollectLibCandidates(libDir, context, sha256Cache,
        removedByIntraApp);
    InterAppDedupPlan plan = solver.Solve(context.gateContext, modules, dedupList);
    if (plan.removedSos.empty()) {
        LOG(DEBUG) << "So deduplication for sdk skipped for module " << context.moduleConfig.moduleName
                   << ": no so file matched the deduplist sdk";
        return result;
    }
    for (const auto& removed : plan.removedSos) {
        result.excludedPaths.push_back(removed.relativePath);
    }
    result.resultJson = GenerateResultJson(plan);
    if (result.resultJson.empty()) {
        result.failed = true;
        result.errorMessage = "failed to generate inter-app dedup result json";
        LOG(ERROR) << FormatInterAppDedupError(result.errorMessage);
        return result;
    }
    result.applied = true;
    LOG(INFO) << "So deduplication for sdk matched " << plan.removedSos.size() << " so file(s) to exclude";
    return result;
}

const std::string InterAppDeduplicator::RESULT_FILE_NAME = "dedupfiles.json";

std::string InterAppDeduplicator::GetCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* utcTime = std::gmtime(&currentTime);
    if (utcTime == nullptr) {
        return "";
    }
    std::stringstream stream;
    stream << std::put_time(utcTime, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

std::string InterAppDeduplicator::GenerateResultJson(const InterAppDedupPlan& plan) const
{
    // Elements are ordered by package relative path for stable output
    std::map<std::string, const InterAppRemovedSo*> orderedRemovals;
    for (const auto& removed : plan.removedSos) {
        orderedRemovals[removed.relativePath] = &removed;
    }
    cJSON* root = cJSON_CreateObject();
    cJSON* removedSoFiles = cJSON_CreateArray();
    if (root == nullptr || removedSoFiles == nullptr ||
        cJSON_AddStringToObject(root, KEY_VERSION.c_str(), RESULT_FILE_VERSION.c_str()) == nullptr ||
        cJSON_AddStringToObject(root, KEY_GENERATED_TIME.c_str(), GetCurrentTimestamp().c_str()) == nullptr ||
        cJSON_AddStringToObject(root, KEY_DESCRIPTION.c_str(), RESULT_FILE_DESCRIPTION.c_str()) == nullptr) {
        cJSON_Delete(removedSoFiles);
        cJSON_Delete(root);
        return "";
    }
    for (const auto& soEntry : orderedRemovals) {
        cJSON* item = BuildRemovedSoItem(*soEntry.second);
        if (item == nullptr || cJSON_AddItemToArray(removedSoFiles, item) == 0) {
            cJSON_Delete(item);
            cJSON_Delete(removedSoFiles);
            cJSON_Delete(root);
            return "";
        }
    }
    if (cJSON_AddItemToObject(root, KEY_REMOVED_SO_FILES.c_str(), removedSoFiles) == 0) {
        cJSON_Delete(removedSoFiles);
        cJSON_Delete(root);
        return "";
    }
    char* jsonString = cJSON_PrintUnformatted(root);
    std::string json = jsonString == nullptr ? "" : std::string(jsonString);
    cJSON_free(jsonString);
    cJSON_Delete(root);
    return json;
}

}  // namespace AppPackingTool
}  // namespace OHOS
