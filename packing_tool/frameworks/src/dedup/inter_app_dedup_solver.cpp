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

#include "dedup/inter_app_dedup_solver.h"

#include "dedup/dedup_log.h"

namespace OHOS {
namespace AppPackingTool {

bool InterAppDedupSolver::IsModuleEligible(const ModuleConfig& config)
{
    if (!config.stageModel) {
        LOG(DEBUG) << "So deduplication for sdk skip: module " << config.moduleName << " is not a Stage model";
        return false;
    }
    if (!ModuleCalculator::IsValidForInterAppDedup(config)) {
        LOG(DEBUG) << "So deduplication for sdk skip: module " << config.moduleName << " is not valid for dedup";
        return false;
    }
    // So must be extracted: compressNativeLibs or extractNativeLibs is true
    if (!config.compressNativeLibs && !config.extractNativeLibs) {
        LOG(DEBUG) << "So deduplication for sdk skip: module " << config.moduleName << " does not extract native libs";
        return false;
    }
    if (config.libIsolation) {
        LOG(DEBUG) << "So deduplication for sdk skip: module " << config.moduleName << " enables libIsolation";
        return false;
    }
    if (config.compileSdkType == "OpenHarmony") {
        LOG(DEBUG) << "So deduplication for sdk skip: module " << config.moduleName << " compileSdkType is OpenHarmony";
        return false;
    }
    return true;
}

bool InterAppDedupSolver::IsDeviceTypeSubset(const std::vector<std::string>& deviceTypes,
    const std::set<std::string>& supportedDeviceTypes)
{
    for (const auto& deviceType : deviceTypes) {
        if (supportedDeviceTypes.find(deviceType) == supportedDeviceTypes.end()) {
            LOG(DEBUG) << "So deduplication for sdk skip: deviceType " << deviceType
                       << " is not declared by the deduplist sdk";
            return false;
        }
    }
    return true;
}

bool InterAppDedupSolver::CheckGates(const InterAppGateContext& context, const DedupListSdkParser& dedupList,
    std::string& skipReason) const
{
    if (context.hasDriverExtensionAbility) {
        skipReason = "driver extensionAbility detected";
        return false;
    }
    // The version gate cannot be evaluated when minAPIVersion is unparsable
    if (!context.HasValidVersion()) {
        skipReason = "compatibleSdkVersion is invalid";
        return false;
    }
    // Both sides are normalised to the same three segments before comparing; equal versions run dedup
    if (context.GetApiVersion() < dedupList.GetDedupMinApiVersion()) {
        skipReason = "compatibleSdkVersion is lower than dedupMinApiVersion";
        return false;
    }
    return true;
}

void InterAppDedupSolver::MergeGateContext(InterAppGateContext& appGate, const InterAppGateContext& moduleGate)
{
    appGate.hasDriverExtensionAbility = appGate.hasDriverExtensionAbility || moduleGate.hasDriverExtensionAbility;
    if (!appGate.HasValidVersion() || appGate.GetApiVersion() < moduleGate.GetApiVersion()) {
        appGate.minApiVersion = moduleGate.minApiVersion;
    }
}

void InterAppDedupSolver::CollectModuleRemovals(const InterAppModuleCandidate& module,
    const DedupListSdkParser& dedupList, InterAppDedupPlan& plan) const
{
    for (const auto& candidate : module.sos) {
        DedupSoMatch match;
        if (!dedupList.FindMatch(candidate.soName, candidate.abi, candidate.sha256,
            module.matchApiVersion, match)) {
            continue;
        }
        InterAppRemovedSo removed;
        removed.soName = candidate.soName;
        removed.relativePath = candidate.relativePath;
        removed.sha256 = match.sha256;
        plan.removedSos.push_back(std::move(removed));
    }
}

InterAppDedupPlan InterAppDedupSolver::Solve(const InterAppGateContext& context,
    const std::vector<InterAppModuleCandidate>& modules, const DedupListSdkParser& dedupList) const
{
    InterAppDedupPlan plan;
    if (!CheckGates(context, dedupList, plan.skipReason)) {
        LOG(INFO) << "So deduplication for sdk skipped: " << plan.skipReason;
        return plan;
    }
    plan.applied = true;
    for (const auto& module : modules) {
        if (!IsModuleEligible(module.config)) {
            LOG(DEBUG) << "Module " << module.moduleName << " is not eligible for inter-app deduplication";
            continue;
        }
        if (!IsDeviceTypeSubset(module.config.deviceTypeNames, dedupList.GetDeviceTypes())) {
            LOG(DEBUG) << "Module " << module.moduleName << " deviceTypes is not a subset of deduplist sdk deviceType";
            continue;
        }
        CollectModuleRemovals(module, dedupList, plan);
    }
    return plan;
}

}  // namespace AppPackingTool
}  // namespace OHOS
