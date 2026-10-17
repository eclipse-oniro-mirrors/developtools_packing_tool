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

#ifndef DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_INTER_APP_DEDUPLICATOR_H
#define DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_INTER_APP_DEDUPLICATOR_H

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "dedup/inter_app_dedup_solver.h"
#include "json/module_json.h"

namespace OHOS {
namespace AppPackingTool {

// Packaging stage result: failed aborts packaging, applied selects inter-app dedup vs baseline
struct InterAppDedupStageResult {
    bool failed = false;
    bool applied = false;
    std::string errorMessage;
    std::vector<std::string> excludedPaths;  // Package relative paths to drop
    std::string resultJson;                  // dedupfiles.json content; empty when no dedup applied
};

// Single package packaging context: gate input and module configuration parsed from module.json
struct InterAppPackContext {
    bool applicable = false;  // Only Stage model packages participate in inter-app dedup
    InterAppGateContext gateContext;
    ModuleConfig moduleConfig;
};

// Removes the module so files selected by the solve plan and generates the result file
class InterAppDeduplicator {
public:
    InterAppDeduplicator() = default;
    ~InterAppDeduplicator() = default;

    InterAppDeduplicator(const InterAppDeduplicator&) = delete;
    InterAppDeduplicator& operator=(const InterAppDeduplicator&) = delete;

    static const std::string RESULT_FILE_NAME;

    // Builds the context from an already parsed Stage module.json (moduleJson must not be null)
    static bool BuildPackContext(const std::shared_ptr<ModuleJson>& moduleJson, InterAppPackContext& context);

    // Result file json content ordered by package relative path; empty on failure
    std::string GenerateResultJson(const InterAppDedupPlan& plan) const;

    /**
     * Solves the inter-app removals of one module and generates the result file content.
     *
     * libDir (user input) is not modified; the caller drops the returned excluded paths and injects
     * the result file while repacking the module. sha256Cache and removedByIntraApp come from the
     * intra-app stage and avoid a second read / match / record of the same so.
     */
    InterAppDedupStageResult ComputeRemovals(const DedupListSdkParser& dedupList, const std::string& libDir,
        const InterAppPackContext& context, const std::map<std::string, std::string>& sha256Cache = {},
        const std::set<std::string>& removedByIntraApp = {});

private:
    std::vector<InterAppModuleCandidate> CollectLibCandidates(const std::string& libDir,
        const InterAppPackContext& context, const std::map<std::string, std::string>& sha256Cache,
        const std::set<std::string>& removedByIntraApp) const;

    // Collects the so candidates of one abi directory; the complete package relative path is kept
    void CollectSoCandidates(const std::filesystem::path& abiDir, const std::string& abi,
        const std::map<std::string, std::string>& sha256Cache, const std::set<std::string>& removedByIntraApp,
        InterAppModuleCandidate& module) const;

    static std::string GetCurrentTimestamp();
    static bool IsSoFileName(const std::string& fileName);
};

}  // namespace AppPackingTool
}  // namespace OHOS

#endif  // DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_INTER_APP_DEDUPLICATOR_H
