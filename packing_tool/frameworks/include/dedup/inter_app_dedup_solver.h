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

#ifndef DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_INTER_APP_DEDUP_SOLVER_H
#define DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_INTER_APP_DEDUP_SOLVER_H

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "dedup/deduplist_sdk_parser.h"
#include "dedup/module_calculator.h"

namespace OHOS {
namespace AppPackingTool {

// Application level gate input: the declared app minAPIVersion and the driver flag. The raw integer is
// normalised to three segments on demand, so the version gate and the compatibility group matching
// compare it against the same value as dedupMinApiVersion and the manifest keys.
struct InterAppGateContext {
    int32_t minApiVersion = 0;  // app.minAPIVersion; 0 when absent or unparsable
    bool hasDriverExtensionAbility = false;

    bool HasValidVersion() const
    {
        return minApiVersion > 0;
    }

    // Normalised api version compared by the gate and the compatibility groups
    DedupApiVersion GetApiVersion() const
    {
        return DedupApiVersion::FromApiVersion(minApiVersion);
    }
};

// A candidate so extracted from a module, with its sha256
struct InterAppSoCandidate {
    std::string soName;
    std::string abi;
    std::string relativePath;  // Package relative path, for example libs/arm64-v8a/libc++_shared.so
    std::string sha256;
};

// Solve input of one module
struct InterAppModuleCandidate {
    std::string moduleName;
    ModuleConfig config;              // Carries the declared device types for the device type gate
    DedupApiVersion matchApiVersion;  // Module's own compatibleSdkVersion for compatibility group matching
    std::vector<InterAppSoCandidate> sos;
};

// An so removed by inter-app dedup, recorded in the result file
struct InterAppRemovedSo {
    std::string soName;
    std::string relativePath;
    std::string sha256;
};

// Inter-app dedup plan
struct InterAppDedupPlan {
    bool applied = false;
    std::string skipReason;  // Gate reason when not applied (logging only)
    std::vector<InterAppRemovedSo> removedSos;
};

// Filters modules by eligibility, checks the application / module gates and matches sha256
class InterAppDedupSolver {
public:
    InterAppDedupSolver() = default;
    ~InterAppDedupSolver() = default;

    InterAppDedupSolver(const InterAppDedupSolver&) = delete;
    InterAppDedupSolver& operator=(const InterAppDedupSolver&) = delete;

    // applied=false with an empty removedSos when a gate is not satisfied
    InterAppDedupPlan Solve(const InterAppGateContext& context,
        const std::vector<InterAppModuleCandidate>& modules, const DedupListSdkParser& dedupList) const;

    // Module level dedup eligibility check
    static bool IsModuleEligible(const ModuleConfig& config);

    // Shared version / driver gates; a failure skips inter-app dedup for the whole package
    bool CheckGates(const InterAppGateContext& context, const DedupListSdkParser& dedupList,
        std::string& skipReason) const;

    // OR-es the driver flag and keeps the highest version; an invalid module version does not contribute
    static void MergeGateContext(InterAppGateContext& appGate, const InterAppGateContext& moduleGate);

    // Whether the declared module device types are a subset of the manifest device types
    static bool IsDeviceTypeSubset(const std::vector<std::string>& deviceTypes,
        const std::set<std::string>& supportedDeviceTypes);

private:
    void CollectModuleRemovals(const InterAppModuleCandidate& module,
        const DedupListSdkParser& dedupList, InterAppDedupPlan& plan) const;
};

}  // namespace AppPackingTool
}  // namespace OHOS

#endif  // DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_INTER_APP_DEDUP_SOLVER_H
