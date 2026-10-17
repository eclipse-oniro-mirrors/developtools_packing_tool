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

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "dedup/deduplist_sdk_parser.h"
#include "dedup/inter_app_dedup_solver.h"
#include "dedup/module_calculator.h"

using testing::ext::TestSize;
using OHOS::AppPackingTool::DedupApiVersion;
using OHOS::AppPackingTool::DedupListSdkParser;
using OHOS::AppPackingTool::InterAppDedupPlan;
using OHOS::AppPackingTool::InterAppDedupSolver;
using OHOS::AppPackingTool::InterAppGateContext;
using OHOS::AppPackingTool::InterAppModuleCandidate;
using OHOS::AppPackingTool::InterAppSoCandidate;
using OHOS::AppPackingTool::ModuleConfig;

namespace {
const std::string MANIFEST = R"({
  "dedupMinApiVersion": 26,
  "deviceType": ["phone", "tablet"],
  "libraries": [
    {"name": "libc++_shared.so", "abi": "arm64-v8a", "26.0.0": "appsha26"}
  ]
})";

// API version shared by the manifest entries and the app side candidates
constexpr int32_t MATCH_API_VERSION = 26;

ModuleConfig BuildEligibleConfig(const std::vector<std::string>& deviceTypes = {"phone"})
{
    ModuleConfig config;
    config.stageModel = true;
    config.moduleName = "entry";
    config.moduleType = "entry";
    config.bundleType = "app";
    config.deviceTypeNames = deviceTypes;
    config.deviceTypesConfigured = true;
    config.deliveryWithInstallPresent = true;
    config.deliveryWithInstall = true;
    config.extractNativeLibs = true;
    // Any SDK type other than OpenHarmony stands for a closed-source SDK
    config.compileSdkType = "ClosedSource";
    return config;
}

InterAppGateContext BuildGateContext(int apiVersion)
{
    InterAppGateContext context;
    context.minApiVersion = apiVersion;
    return context;
}

InterAppModuleCandidate BuildCandidate(const ModuleConfig& config, const std::string& sha256)
{
    InterAppModuleCandidate module;
    module.moduleName = config.moduleName;
    module.config = config;
    module.matchApiVersion = DedupApiVersion::FromApiVersion(MATCH_API_VERSION);
    module.sos.push_back(InterAppSoCandidate{
        "libc++_shared.so", "arm64-v8a", "libs/arm64-v8a/libc++_shared.so", sha256});
    return module;
}
}  // namespace

class InterAppDedupSolverTest : public testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    InterAppDedupSolver solver_;
    DedupListSdkParser dedupList_;
};

void InterAppDedupSolverTest::SetUp()
{
    ASSERT_TRUE(dedupList_.ParseContent(MANIFEST));
}
void InterAppDedupSolverTest::TearDown() {}

/**
 * @tc.name: Solve_ApplicationAndDeviceGates_0100
 * @tc.desc: The version gate skips the whole package (equality runs), the driver gate skips it, and the
 *           module level device type gate only skips the declaring module.
 */
HWTEST_F(InterAppDedupSolverTest, Solve_ApplicationAndDeviceGates_0100, TestSize.Level0)
{
    std::vector<InterAppModuleCandidate> modules = {BuildCandidate(BuildEligibleConfig(), "appsha26")};

    InterAppDedupPlan lowVersion = solver_.Solve(BuildGateContext(9), modules, dedupList_);
    EXPECT_FALSE(lowVersion.applied);
    EXPECT_TRUE(lowVersion.removedSos.empty());
    EXPECT_FALSE(lowVersion.skipReason.empty());

    InterAppGateContext driverGate = BuildGateContext(26);
    driverGate.hasDriverExtensionAbility = true;
    EXPECT_FALSE(solver_.Solve(driverGate, modules, dedupList_).applied);

    // The device type gate is module level: the unsupported module is skipped but the package runs
    std::vector<InterAppModuleCandidate> wearable = {
        BuildCandidate(BuildEligibleConfig({"wearable"}), "appsha26")};
    InterAppDedupPlan notSubset = solver_.Solve(BuildGateContext(26), wearable, dedupList_);
    EXPECT_TRUE(notSubset.applied);
    EXPECT_TRUE(notSubset.removedSos.empty());

    std::vector<InterAppModuleCandidate> mixed = {
        BuildCandidate(BuildEligibleConfig({"wearable"}), "appsha26"),
        BuildCandidate(BuildEligibleConfig(), "appsha26"),
    };
    InterAppDedupPlan mixedPlan = solver_.Solve(BuildGateContext(26), mixed, dedupList_);
    ASSERT_TRUE(mixedPlan.applied);
    EXPECT_EQ(mixedPlan.removedSos.size(), 1U);
}

/**
 * @tc.name: Solve_ModuleEligibility_0200
 * @tc.desc: Modules that do not extract so, enable libIsolation, use compileSdkType OpenHarmony or are not
 *           Stage are excluded; bundleType and deliveryWithInstall are not part of the inter-app validity.
 */
HWTEST_F(InterAppDedupSolverTest, Solve_ModuleEligibility_0200, TestSize.Level0)
{
    ModuleConfig noExtract = BuildEligibleConfig();
    noExtract.compressNativeLibs = false;
    noExtract.extractNativeLibs = false;
    EXPECT_FALSE(InterAppDedupSolver::IsModuleEligible(noExtract));

    ModuleConfig libIsolation = BuildEligibleConfig();
    libIsolation.libIsolation = true;
    EXPECT_FALSE(InterAppDedupSolver::IsModuleEligible(libIsolation));

    ModuleConfig openHarmony = BuildEligibleConfig();
    openHarmony.compileSdkType = "OpenHarmony";
    EXPECT_FALSE(InterAppDedupSolver::IsModuleEligible(openHarmony));

    ModuleConfig fa = BuildEligibleConfig();
    fa.stageModel = false;
    EXPECT_FALSE(InterAppDedupSolver::IsModuleEligible(fa));

    ModuleConfig noDeviceTypes = BuildEligibleConfig();
    noDeviceTypes.deviceTypesConfigured = false;
    EXPECT_FALSE(InterAppDedupSolver::IsModuleEligible(noDeviceTypes));

    ModuleConfig unsupportedType = BuildEligibleConfig();
    unsupportedType.bundleType = "shared";
    unsupportedType.deliveryWithInstallPresent = false;
    EXPECT_TRUE(InterAppDedupSolver::IsModuleEligible(unsupportedType));

    std::vector<InterAppModuleCandidate> modules = {
        BuildCandidate(noExtract, "appsha26"),
        BuildCandidate(libIsolation, "appsha26"),
        BuildCandidate(openHarmony, "appsha26"),
        BuildCandidate(fa, "appsha26"),
    };
    InterAppDedupPlan plan = solver_.Solve(BuildGateContext(26), modules, dedupList_);
    EXPECT_TRUE(plan.applied);
    EXPECT_TRUE(plan.removedSos.empty());

    std::vector<InterAppModuleCandidate> mixed = {
        BuildCandidate(noExtract, "appsha26"),
        BuildCandidate(BuildEligibleConfig(), "appsha26"),
    };
    InterAppDedupPlan mixedPlan = solver_.Solve(BuildGateContext(26), mixed, dedupList_);
    ASSERT_TRUE(mixedPlan.applied);
    ASSERT_EQ(mixedPlan.removedSos.size(), 1U);
    EXPECT_EQ(mixedPlan.removedSos[0].soName, "libc++_shared.so");
    EXPECT_EQ(mixedPlan.removedSos[0].relativePath, "libs/arm64-v8a/libc++_shared.so");
    EXPECT_EQ(mixedPlan.removedSos[0].sha256, "appsha26");
}

/**
 * @tc.name: Solve_VersionComparisonAndMatch_0300
 * @tc.desc: The version gate compares the three segments produced from a composite minAPIVersion and from
 *           dedupMinApiVersion, a lower or unevaluable version cannot pass, and a non matching sha256
 *           produces an empty plan although the gates passed.
 */
HWTEST_F(InterAppDedupSolverTest, Solve_VersionComparisonAndMatch_0300, TestSize.Level0)
{
    std::vector<InterAppModuleCandidate> modules = {BuildCandidate(BuildEligibleConfig(), "appsha26")};

    // A composite minAPIVersion equal to dedupMinApiVersion passes the gate and matches the group
    InterAppDedupPlan composite = solver_.Solve(BuildGateContext(260000026), modules, dedupList_);
    EXPECT_TRUE(composite.applied);
    ASSERT_EQ(composite.removedSos.size(), 1U);

    // A composite minAPIVersion below dedupMinApiVersion skips the whole package
    EXPECT_FALSE(solver_.Solve(BuildGateContext(250909026), modules, dedupList_).applied);

    // A higher composite minAPIVersion inherits the lower group
    InterAppDedupPlan higher = solver_.Solve(BuildGateContext(260100026), modules, dedupList_);
    EXPECT_TRUE(higher.applied);
    ASSERT_EQ(higher.removedSos.size(), 1U);

    InterAppGateContext invalid;
    EXPECT_FALSE(solver_.Solve(invalid, modules, dedupList_).applied);

    std::vector<InterAppModuleCandidate> noMatch = {BuildCandidate(BuildEligibleConfig(), "unknownsha")};
    InterAppDedupPlan emptyPlan = solver_.Solve(BuildGateContext(260000026), noMatch, dedupList_);
    EXPECT_TRUE(emptyPlan.applied);
    EXPECT_TRUE(emptyPlan.removedSos.empty());
}

/**
 * @tc.name: MergeGateContext_DriverAndHighestVersion_0400
 * @tc.desc: The driver flag is OR-ed and the highest version wins; an unevaluable module version does not
 *           contribute to the application level maximum.
 */
HWTEST_F(InterAppDedupSolverTest, MergeGateContext_DriverAndHighestVersion_0400, TestSize.Level0)
{
    InterAppGateContext appGate = BuildGateContext(260000026);
    InterAppGateContext moduleGate = BuildGateContext(23);
    moduleGate.hasDriverExtensionAbility = true;
    InterAppDedupSolver::MergeGateContext(appGate, moduleGate);
    EXPECT_TRUE(appGate.hasDriverExtensionAbility);
    EXPECT_EQ(appGate.minApiVersion, 260000026);
    EXPECT_TRUE(appGate.GetApiVersion() == DedupApiVersion::FromApiVersion(26));

    InterAppDedupSolver::MergeGateContext(appGate, BuildGateContext(28));
    EXPECT_EQ(appGate.minApiVersion, 28);
    EXPECT_TRUE(appGate.GetApiVersion() == DedupApiVersion::FromApiVersion(28));

    InterAppGateContext invalidModule;
    InterAppDedupSolver::MergeGateContext(appGate, invalidModule);
    EXPECT_EQ(appGate.minApiVersion, 28);
}

/**
 * @tc.name: Solve_EmptyModulesAndDeviceTypeSubsetEdge_0500
 * @tc.desc: An empty module list still runs the application gates but removes nothing, and a module
 *           that declares no device type is a subset of every deduplist sdk device set.
 */
HWTEST_F(InterAppDedupSolverTest, Solve_EmptyModulesAndDeviceTypeSubsetEdge_0500, TestSize.Level0)
{
    std::vector<InterAppModuleCandidate> noModules;
    InterAppDedupPlan emptyPlan = solver_.Solve(BuildGateContext(26), noModules, dedupList_);
    EXPECT_TRUE(emptyPlan.applied);
    EXPECT_TRUE(emptyPlan.removedSos.empty());

    // The gate loop of an empty declared device type list never rejects the module
    EXPECT_TRUE(InterAppDedupSolver::IsDeviceTypeSubset({}, dedupList_.GetDeviceTypes()));
    EXPECT_TRUE(InterAppDedupSolver::IsDeviceTypeSubset({}, {}));
}
