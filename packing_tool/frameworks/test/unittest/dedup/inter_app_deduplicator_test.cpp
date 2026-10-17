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

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "dedup/deduplist_sdk_parser.h"
#include "dedup/inter_app_dedup_solver.h"
#include "dedup/inter_app_deduplicator.h"
#include "dedup/module_calculator.h"
#include "utils.h"

using testing::ext::TestSize;
using OHOS::AppPackingTool::DedupApiVersion;
using OHOS::AppPackingTool::DedupListSdkParser;
using OHOS::AppPackingTool::InterAppDedupPlan;
using OHOS::AppPackingTool::InterAppDedupStageResult;
using OHOS::AppPackingTool::InterAppDeduplicator;
using OHOS::AppPackingTool::InterAppPackContext;
using OHOS::AppPackingTool::InterAppRemovedSo;
using OHOS::AppPackingTool::ModuleConfig;
using OHOS::AppPackingTool::ModuleJson;
using OHOS::AppPackingTool::Utils;

namespace {
const std::string SO_ENTRY = "libs/arm64-v8a/libc++_shared.so";

std::filesystem::path MakeTempDir(const std::string& tag)
{
    static int32_t counter = 0;
    std::filesystem::path dir = std::filesystem::temp_directory_path() /
        ("ia_dedup_" + tag + "_" + std::to_string(++counter));
    std::filesystem::create_directories(dir);
    return dir;
}

void WriteFile(const std::filesystem::path& path, const std::string& content)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::trunc | std::ios::binary);
    file << content;
    file.close();
}

std::string BuildManifest(const std::string& sha256)
{
    return "{\"dedupMinApiVersion\":26,\"deviceType\":[\"phone\",\"tablet\"],"
        "\"libraries\":[{\"name\":\"libc++_shared.so\",\"abi\":\"arm64-v8a\",\"26.0.0\":\"" +
        sha256 + "\"}]}";
}

ModuleConfig BuildEligibleConfig()
{
    ModuleConfig config;
    config.stageModel = true;
    config.moduleName = "entry";
    config.moduleType = "entry";
    config.bundleType = "app";
    config.deviceTypeNames = {"phone"};
    config.deviceTypesConfigured = true;
    config.deliveryWithInstallPresent = true;
    config.deliveryWithInstall = true;
    config.extractNativeLibs = true;
    // Any SDK type other than OpenHarmony stands for a closed-source SDK
    config.compileSdkType = "ClosedSource";
    return config;
}

InterAppPackContext BuildContext(const ModuleConfig& config, int apiVersion)
{
    InterAppPackContext context;
    context.applicable = true;
    context.moduleConfig = config;
    context.gateContext.minApiVersion = apiVersion;
    return context;
}

size_t CountOccurrences(const std::string& text, const std::string& token)
{
    size_t count = 0;
    for (size_t position = text.find(token); position != std::string::npos;
        position = text.find(token, position + token.size())) {
        ++count;
    }
    return count;
}

// removedSoFiles must be a json array (whitespace between the key, the colon and the value is ignored)
bool IsRemovedSoFilesArray(const std::string& json)
{
    size_t keyPos = json.find("\"removedSoFiles\"");
    if (keyPos == std::string::npos) {
        return false;
    }
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) {
        return false;
    }
    size_t valuePos = json.find_first_not_of(" \t\r\n", colonPos + 1);
    return valuePos != std::string::npos && json[valuePos] == '[';
}
}  // namespace

class InterAppDeduplicatorTest : public testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
};

void InterAppDeduplicatorTest::SetUp() {}
void InterAppDeduplicatorTest::TearDown() {}

/**
 * @tc.name: ComputeRemovals_AppliedAndSkipped_0100
 * @tc.desc: A matched so is excluded and recorded while the user lib directory stays untouched;
 *           an ineligible module and a so already removed by the intra-app stage are skipped.
 */
HWTEST_F(InterAppDeduplicatorTest, ComputeRemovals_AppliedAndSkipped_0100, TestSize.Level0)
{
    std::filesystem::path dir = MakeTempDir("compute");
    std::filesystem::path libDir = dir / "libs";
    std::filesystem::path soPath = libDir / "arm64-v8a" / "libc++_shared.so";
    WriteFile(soPath, "app-so-bytes");
    WriteFile(libDir / "armeabi-v7a" / "libother.so", "other-so-bytes");
    std::string appSha = Utils::GetSha256File(soPath.string());
    ASSERT_FALSE(appSha.empty());

    DedupListSdkParser dedupList;
    ASSERT_TRUE(dedupList.ParseContent(BuildManifest(appSha)));
    InterAppDeduplicator deduplicator;

    InterAppDedupStageResult applied = deduplicator.ComputeRemovals(dedupList, libDir.string(),
        BuildContext(BuildEligibleConfig(), 26));
    EXPECT_FALSE(applied.failed);
    ASSERT_TRUE(applied.applied);
    ASSERT_EQ(applied.excludedPaths.size(), 1U);
    EXPECT_EQ(applied.excludedPaths[0], SO_ENTRY);
    EXPECT_NE(applied.resultJson.find(SO_ENTRY), std::string::npos);
    EXPECT_NE(applied.resultJson.find(appSha), std::string::npos);
    EXPECT_TRUE(std::filesystem::exists(soPath));

    ModuleConfig ineligible = BuildEligibleConfig();
    ineligible.extractNativeLibs = false;
    InterAppDedupStageResult filtered = deduplicator.ComputeRemovals(dedupList, libDir.string(),
        BuildContext(ineligible, 26));
    EXPECT_FALSE(filtered.applied);
    EXPECT_TRUE(filtered.excludedPaths.empty());
    EXPECT_TRUE(filtered.resultJson.empty());

    InterAppDedupStageResult skipped = deduplicator.ComputeRemovals(dedupList, libDir.string(),
        BuildContext(BuildEligibleConfig(), 26), {}, {SO_ENTRY});
    EXPECT_FALSE(skipped.applied);
    EXPECT_TRUE(skipped.excludedPaths.empty());
    std::filesystem::remove_all(dir);
}

/**
 * @tc.name: ComputeRemovals_NoMatchAndNestedPath_0200
 * @tc.desc: Without a sha256 hit nothing is produced; a so under a sub directory of libs/<abi> keeps its
 *           complete package relative path.
 */
HWTEST_F(InterAppDeduplicatorTest, ComputeRemovals_NoMatchAndNestedPath_0200, TestSize.Level0)
{
    std::filesystem::path dir = MakeTempDir("nomatch");
    std::filesystem::path libDir = dir / "libs";
    std::filesystem::path soPath = libDir / "arm64-v8a" / "plugins" / "sub" / "libc++_shared.so";
    WriteFile(soPath, "app-so-bytes");
    std::string appSha = Utils::GetSha256File(soPath.string());
    ASSERT_FALSE(appSha.empty());

    DedupListSdkParser dedupList;
    ASSERT_TRUE(dedupList.ParseContent(BuildManifest("unknownsha")));
    InterAppDeduplicator deduplicator;
    InterAppDedupStageResult noMatch = deduplicator.ComputeRemovals(dedupList, libDir.string(),
        BuildContext(BuildEligibleConfig(), 26));
    EXPECT_FALSE(noMatch.failed);
    EXPECT_FALSE(noMatch.applied);
    EXPECT_TRUE(noMatch.resultJson.empty());

    ASSERT_TRUE(dedupList.ParseContent(BuildManifest(appSha)));
    InterAppDedupStageResult nested = deduplicator.ComputeRemovals(dedupList, libDir.string(),
        BuildContext(BuildEligibleConfig(), 26));
    ASSERT_TRUE(nested.applied);
    ASSERT_EQ(nested.excludedPaths.size(), 1U);
    EXPECT_EQ(nested.excludedPaths[0], "libs/arm64-v8a/plugins/sub/libc++_shared.so");
    EXPECT_TRUE(std::filesystem::exists(soPath));
    std::filesystem::remove_all(dir);
}

/**
 * @tc.name: GenerateResultJson_ArrayFieldsAndOrder_0300
 * @tc.desc: removedSoFiles is an array ordered by the package relative path, each element keeping the two
 *           documented fields of the removed so.
 */
HWTEST_F(InterAppDeduplicatorTest, GenerateResultJson_ArrayFieldsAndOrder_0300, TestSize.Level0)
{
    InterAppDeduplicator deduplicator;
    InterAppDedupPlan plan;
    plan.applied = true;
    InterAppRemovedSo first;
    first.soName = "libb.so";
    first.relativePath = "libs/arm64-v8a/libb.so";
    first.sha256 = "sha-b";
    InterAppRemovedSo second;
    second.soName = "liba.so";
    second.relativePath = "libs/armeabi-v7a/liba.so";
    second.sha256 = "sha-a";
    plan.removedSos.push_back(first);
    plan.removedSos.push_back(second);
    std::string json = deduplicator.GenerateResultJson(plan);
    EXPECT_NE(json.find("\"version\""), std::string::npos);
    EXPECT_NE(json.find("1.0"), std::string::npos);
    EXPECT_NE(json.find("\"generatedTime\""), std::string::npos);
    EXPECT_NE(json.find("\"description\""), std::string::npos);
    EXPECT_TRUE(IsRemovedSoFilesArray(json));
    // Each removed so is one element holding exactly the two documented fields, ordered by path
    EXPECT_EQ(CountOccurrences(json, "\"originalPathInHap\""), 2U);
    EXPECT_EQ(CountOccurrences(json, "\"originalSha256\""), 2U);
    EXPECT_LT(json.find("libs/arm64-v8a/libb.so"), json.find("libs/armeabi-v7a/liba.so"));
    EXPECT_NE(json.find("sha-a"), std::string::npos);
    EXPECT_NE(json.find("sha-b"), std::string::npos);
}

/**
 * @tc.name: GenerateResultJson_EmptyPlan_0400
 * @tc.desc: An empty plan still produces a parseable document whose removedSoFiles array stays empty.
 */
HWTEST_F(InterAppDeduplicatorTest, GenerateResultJson_EmptyPlan_0400, TestSize.Level0)
{
    InterAppDeduplicator deduplicator;
    InterAppDedupPlan plan;
    std::string json = deduplicator.GenerateResultJson(plan);
    EXPECT_NE(json.find("\"version\""), std::string::npos);
    EXPECT_NE(json.find("\"description\""), std::string::npos);
    EXPECT_TRUE(IsRemovedSoFilesArray(json));
    EXPECT_EQ(CountOccurrences(json, "\"originalPathInHap\""), 0U);
}

/**
 * @tc.name: BuildPackContext_ModuleJsonBranches_0500
 * @tc.desc: A null model is rejected while a Stage module.json fills the gate context and the module
 *           configuration: a composite minAPIVersion, the driver extensionAbility, an unevaluable
 *           version and a malformed extensionAbilities node are all handled.
 */
HWTEST_F(InterAppDeduplicatorTest, BuildPackContext_ModuleJsonBranches_0500, TestSize.Level0)
{
    InterAppPackContext context;
    std::shared_ptr<ModuleJson> nullModule;
    EXPECT_FALSE(InterAppDeduplicator::BuildPackContext(nullModule, context));
    EXPECT_FALSE(context.applicable);

    auto driverModule = std::make_shared<ModuleJson>();
    ASSERT_TRUE(driverModule->ParseFromString(
        "{\"app\":{\"bundleType\":\"app\",\"minAPIVersion\":260000026},"
        "\"module\":{\"name\":\"entry\",\"type\":\"entry\",\"deviceTypes\":[\"phone\"],"
        "\"deliveryWithInstall\":true,\"extractNativeLibs\":true,"
        "\"extensionAbilities\":[{\"name\":\"a\",\"type\":\"service\"},{\"name\":\"b\",\"type\":\"driver\"}]}}"));
    ASSERT_TRUE(InterAppDeduplicator::BuildPackContext(driverModule, context));
    EXPECT_TRUE(context.applicable);
    EXPECT_TRUE(context.gateContext.hasDriverExtensionAbility);
    // A composite minAPIVersion is normalised to three segments for the gate and the compatibility groups
    EXPECT_EQ(context.gateContext.minApiVersion, 260000026);
    EXPECT_TRUE(context.gateContext.GetApiVersion() == DedupApiVersion::FromApiVersion(26));
    EXPECT_EQ(context.moduleConfig.moduleName, "entry");
    ASSERT_EQ(context.moduleConfig.deviceTypeNames.size(), 1U);
    EXPECT_EQ(context.moduleConfig.deviceTypeNames[0], "phone");

    // Without minAPIVersion the version gate stays unevaluable but the context is still applicable
    auto noVersionModule = std::make_shared<ModuleJson>();
    ASSERT_TRUE(noVersionModule->ParseFromString(
        "{\"app\":{\"bundleType\":\"app\"},"
        "\"module\":{\"name\":\"entry\",\"type\":\"entry\",\"deviceTypes\":[\"phone\"]}}"));
    ASSERT_TRUE(InterAppDeduplicator::BuildPackContext(noVersionModule, context));
    EXPECT_TRUE(context.applicable);
    EXPECT_FALSE(context.gateContext.HasValidVersion());
    EXPECT_FALSE(context.gateContext.GetApiVersion().IsValid());
    EXPECT_FALSE(context.gateContext.hasDriverExtensionAbility);

    // A malformed extensionAbilities node does not abort the context: no driver is reported
    auto malformedModule = std::make_shared<ModuleJson>();
    ASSERT_TRUE(malformedModule->ParseFromString(
        "{\"app\":{\"bundleType\":\"app\",\"minAPIVersion\":26},"
        "\"module\":{\"name\":\"entry\",\"type\":\"entry\",\"deviceTypes\":[\"phone\"],"
        "\"extensionAbilities\":\"test\"}}"));
    ASSERT_TRUE(InterAppDeduplicator::BuildPackContext(malformedModule, context));
    EXPECT_TRUE(context.applicable);
    EXPECT_FALSE(context.gateContext.hasDriverExtensionAbility);
    EXPECT_TRUE(context.gateContext.GetApiVersion() == DedupApiVersion::FromApiVersion(26));

    // A non positive minAPIVersion cannot be translated into a compatible sdk version
    auto zeroVersionModule = std::make_shared<ModuleJson>();
    ASSERT_TRUE(zeroVersionModule->ParseFromString(
        "{\"app\":{\"bundleType\":\"app\",\"minAPIVersion\":0},"
        "\"module\":{\"name\":\"entry\",\"type\":\"entry\",\"deviceTypes\":[\"phone\"]}}"));
    ASSERT_TRUE(InterAppDeduplicator::BuildPackContext(zeroVersionModule, context));
    EXPECT_TRUE(context.applicable);
    EXPECT_FALSE(context.gateContext.HasValidVersion());
    EXPECT_FALSE(context.gateContext.GetApiVersion().IsValid());
}

/**
 * @tc.name: ComputeRemovals_LibDirectoryAndCacheBranches_0600
 * @tc.desc: A missing / non directory lib dir and a lib dir without any so yield nothing, a cached sha256 is
 *           reused instead of reading the file, and entries that are not so files are ignored.
 */
HWTEST_F(InterAppDeduplicatorTest, ComputeRemovals_LibDirectoryAndCacheBranches_0600, TestSize.Level0)
{
    std::filesystem::path dir = MakeTempDir("libdir");
    std::filesystem::path libDir = dir / "libs";
    std::filesystem::path soPath = libDir / "arm64-v8a" / "libc++_shared.so";
    WriteFile(soPath, "app-so-bytes");
    // Not a directory and not a so file name: both entries must be ignored
    WriteFile(libDir / "notes.txt", "not a directory");
    WriteFile(libDir / "arm64-v8a" / "libc++_shared.so.bak", "backup");
    std::string appSha = Utils::GetSha256File(soPath.string());
    ASSERT_FALSE(appSha.empty());
    const std::string relativePath = "libs/arm64-v8a/libc++_shared.so";

    DedupListSdkParser dedupList;
    ASSERT_TRUE(dedupList.ParseContent(BuildManifest(appSha)));
    InterAppDeduplicator deduplicator;
    InterAppPackContext context = BuildContext(BuildEligibleConfig(), 26);

    EXPECT_FALSE(deduplicator.ComputeRemovals(dedupList, "", context).applied);
    InterAppDedupStageResult missingDir =
        deduplicator.ComputeRemovals(dedupList, (dir / "missing").string(), context);
    EXPECT_FALSE(missingDir.failed);
    EXPECT_FALSE(missingDir.applied);
    InterAppDedupStageResult notADirectory = deduplicator.ComputeRemovals(dedupList, soPath.string(), context);
    EXPECT_FALSE(notADirectory.failed);
    EXPECT_FALSE(notADirectory.applied);

    // The cached sha256 is used as is: a cached mismatch never matches although the file itself would
    InterAppDedupStageResult cachedMiss = deduplicator.ComputeRemovals(dedupList, libDir.string(), context,
        {{relativePath, "cachedsha"}});
    EXPECT_FALSE(cachedMiss.failed);
    EXPECT_FALSE(cachedMiss.applied);

    InterAppDedupStageResult cachedHit = deduplicator.ComputeRemovals(dedupList, libDir.string(), context,
        {{relativePath, appSha}});
    ASSERT_TRUE(cachedHit.applied);
    ASSERT_EQ(cachedHit.excludedPaths.size(), 1U);
    EXPECT_EQ(cachedHit.excludedPaths[0], relativePath);
    EXPECT_NE(cachedHit.resultJson.find(relativePath), std::string::npos);

    // A module configuration without a name falls back to the lib directory as the module identity; the
    // module itself is then not eligible for inter-app dedup
    ModuleConfig unnamed = BuildEligibleConfig();
    unnamed.moduleName.clear();
    InterAppPackContext unnamedContext = BuildContext(unnamed, 26);
    InterAppDedupStageResult unnamedResult = deduplicator.ComputeRemovals(dedupList, libDir.string(), unnamedContext);
    EXPECT_FALSE(unnamedResult.failed);
    EXPECT_FALSE(unnamedResult.applied);
    EXPECT_TRUE(unnamedResult.excludedPaths.empty());
    std::filesystem::remove_all(dir);
}

/**
 * @tc.name: ComputeRemovals_VersionedSoAndEmptyDigest_0700
 * @tc.desc: A versioned so file name is collected and matched like a plain one, a cached but empty
 *           sha256 skips the so, and a lib directory that holds no so file collects no candidate.
 */
HWTEST_F(InterAppDeduplicatorTest, ComputeRemovals_VersionedSoAndEmptyDigest_0700, TestSize.Level0)
{
    std::filesystem::path dir = MakeTempDir("versioned");
    std::filesystem::path libDir = dir / "libs";
    std::filesystem::path soPath = libDir / "arm64-v8a" / "libc++_shared.so.1";
    WriteFile(soPath, "app-so-bytes");
    std::string appSha = Utils::GetSha256File(soPath.string());
    ASSERT_FALSE(appSha.empty());
    const std::string versionedEntry = "libs/arm64-v8a/libc++_shared.so.1";

    // The manifest library name is the complete versioned file name of the collected so
    DedupListSdkParser dedupList;
    ASSERT_TRUE(dedupList.ParseContent(
        "{\"dedupMinApiVersion\":26,\"deviceType\":[\"phone\"],"
        "\"libraries\":[{\"name\":\"libc++_shared.so.1\",\"abi\":\"arm64-v8a\",\"26.0.0\":\"" +
        appSha + "\"}]}"));
    InterAppDeduplicator deduplicator;
    InterAppDedupStageResult versioned = deduplicator.ComputeRemovals(dedupList, libDir.string(),
        BuildContext(BuildEligibleConfig(), 26));
    ASSERT_TRUE(versioned.applied);
    ASSERT_EQ(versioned.excludedPaths.size(), 1U);
    EXPECT_EQ(versioned.excludedPaths[0], versionedEntry);

    // A cached but empty sha256 cannot match: the so is skipped without reading the file again
    InterAppDedupStageResult emptyDigest = deduplicator.ComputeRemovals(dedupList, libDir.string(),
        BuildContext(BuildEligibleConfig(), 26), {{versionedEntry, ""}});
    EXPECT_FALSE(emptyDigest.failed);
    EXPECT_FALSE(emptyDigest.applied);
    EXPECT_TRUE(emptyDigest.excludedPaths.empty());

    // A lib directory whose abi directory holds no so file collects no module candidate at all
    std::filesystem::path noSoDir = dir / "libs_nososo";
    WriteFile(noSoDir / "arm64-v8a" / "readme.txt", "not a so");
    InterAppDedupStageResult noSo = deduplicator.ComputeRemovals(dedupList, noSoDir.string(),
        BuildContext(BuildEligibleConfig(), 26));
    EXPECT_FALSE(noSo.failed);
    EXPECT_FALSE(noSo.applied);
    EXPECT_TRUE(noSo.excludedPaths.empty());
    std::filesystem::remove_all(dir);
}
