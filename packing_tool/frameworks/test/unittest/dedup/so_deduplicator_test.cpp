/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "cJSON.h"
#include "dedup/so_deduplicator.h"
#include "utils.h"
#include "zip_utils.h"
#include "zip_wrapper.h"

using testing::ext::TestSize;

namespace {

constexpr const char* TEST_ROOT = "so_deduplicator_security_test";
constexpr const char* MALICIOUS_MODULE_NAME = "../../target";
constexpr const char* SO_PATH = "libs/arm64-v8a/libsame.so";
constexpr const char* DISTRO_FILTER_PATH = "resources/base/profile/density_filter.json";

std::string EscapeJsonString(const std::string& value)
{
    std::string escaped;
    for (char character : value) {
        if (character == '\\' || character == '"') {
            escaped.push_back('\\');
        }
        escaped.push_back(character);
    }
    return escaped;
}

std::string BuildModuleJson(
    const std::string& moduleName,
    const std::string& deviceType = "phone",
    const std::string& moduleType = "entry",
    bool deliveryWithInstall = true,
    bool hasDistributionFilter = false)
{
    return "{"
        "\"app\":{\"bundleType\":\"app\"},"
        "\"module\":{"
            "\"name\":\"" + EscapeJsonString(moduleName) + "\","
            "\"type\":\"" + moduleType + "\","
            "\"deviceTypes\":[\"" + deviceType + "\"],"
            "\"deliveryWithInstall\":" + (deliveryWithInstall ? "true" : "false") + ","
            + (hasDistributionFilter ?
                "\"metadata\":[{\"name\":\"distributionFilter\",\"resource\":\"$profile:density_filter\"}]," : "") +
            "\"extractNativeLibs\":true"
        "}"
    "}";
}

void RemoveTestRoot()
{
    std::filesystem::remove_all(TEST_ROOT);
}

bool CreateModuleZip(
    const std::filesystem::path& zipPath,
    const std::string& moduleJson,
    bool includeSo = false,
    const std::string& distroFilter = "")
{
    OHOS::AppPackingTool::ZipWrapper zipWrapper(zipPath.string());
    if (zipWrapper.Open() != OHOS::AppPackingTool::ZIP_ERR_SUCCESS) {
        return false;
    }
    bool success = zipWrapper.WriteStringToZip(moduleJson, "module.json") ==
        OHOS::AppPackingTool::ZIP_ERR_SUCCESS;
    if (includeSo) {
        success = success && zipWrapper.WriteStringToZip("same so content", SO_PATH) ==
            OHOS::AppPackingTool::ZIP_ERR_SUCCESS;
    }
    if (!distroFilter.empty()) {
        success = success && zipWrapper.WriteStringToZip(distroFilter, DISTRO_FILTER_PATH) ==
            OHOS::AppPackingTool::ZIP_ERR_SUCCESS;
    }
    zipWrapper.Close();
    return success;
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

bool CreateModuleZipWithEntries(const std::filesystem::path& zipPath, const std::string& moduleJson,
    const std::vector<std::pair<std::string, std::string>>& extraEntries)
{
    OHOS::AppPackingTool::ZipWrapper zipWrapper(zipPath.string());
    if (zipWrapper.Open() != OHOS::AppPackingTool::ZIP_ERR_SUCCESS) {
        return false;
    }
    bool success = zipWrapper.WriteStringToZip(moduleJson, "module.json") ==
        OHOS::AppPackingTool::ZIP_ERR_SUCCESS;
    for (const auto& [entryName, content] : extraEntries) {
        success = success && zipWrapper.WriteStringToZip(content, entryName) ==
            OHOS::AppPackingTool::ZIP_ERR_SUCCESS;
    }
    zipWrapper.Close();
    return success;
}

std::string BuildModuleJsonWithApi(const std::string& moduleName, const std::string& deviceType,
    int apiVersion = 26)
{
    return "{"
        "\"app\":{\"bundleType\":\"app\",\"minAPIVersion\":" + std::to_string(apiVersion) + "},"
        "\"module\":{"
            "\"name\":\"" + EscapeJsonString(moduleName) + "\","
            "\"type\":\"entry\","
            "\"deviceTypes\":[\"" + deviceType + "\"],"
            "\"deliveryWithInstall\":true,"
            "\"extractNativeLibs\":true"
        "}"
    "}";
}

std::string BuildDeduplistManifest(const std::string& sha256)
{
    return "{"
        "\"dedupMinApiVersion\":26,"
        "\"deviceType\":[\"phone\",\"tablet\"],"
        "\"libraries\":[{\"name\":\"libsame.so\",\"abi\":\"arm64-v8a\","
            "\"26.0.0\":\"" + sha256 + "\"}]"
    "}";
}

bool WriteTextFile(const std::filesystem::path& path, const std::string& content)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::trunc | std::ios::binary);
    file << content;
    file.close();
    return true;
}

// Writes arbitrary entries into a module zip; used for FA and malformed configuration packages
bool CreateModuleZipWithRawEntries(const std::filesystem::path& zipPath,
    const std::vector<std::pair<std::string, std::string>>& entries)
{
    OHOS::AppPackingTool::ZipWrapper zipWrapper(zipPath.string());
    if (zipWrapper.Open() != OHOS::AppPackingTool::ZIP_ERR_SUCCESS) {
        return false;
    }
    bool success = true;
    for (const auto& [entryName, content] : entries) {
        success = success && zipWrapper.WriteStringToZip(content, entryName) ==
            OHOS::AppPackingTool::ZIP_ERR_SUCCESS;
    }
    zipWrapper.Close();
    return success;
}

// Builds a Stage module.json with an injectable module field list, api version and compileSdkType
std::string BuildStageModuleJson(const std::string& moduleName, const std::string& extraModuleFields,
    int apiVersion = 26, const std::string& compileSdkType = "")
{
    std::string appNode = "\"app\":{\"bundleType\":\"app\",\"minAPIVersion\":" + std::to_string(apiVersion);
    if (!compileSdkType.empty()) {
        appNode += ",\"compileSdkType\":\"" + compileSdkType + "\"";
    }
    appNode += "}";
    return "{" + appNode + ",\"module\":{\"name\":\"" + moduleName +
        "\",\"type\":\"entry\",\"deviceTypes\":[\"phone\"],\"deliveryWithInstall\":true" +
        extraModuleFields + "}}";
}

// Counts and recorded result content of both modules after one switch combination
struct ComboOutcome {
    int32_t soCount = 0;
    int32_t resultCount = 0;
    std::string resultJson;
};

const char* RESULT_FILE_NAME = "dedupfiles.json";

// Records one module's repack result and verifies that its user input is preserved
bool CollectModuleOutcome(const std::string& modulePath, ComboOutcome& outcome)
{
    if (OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(modulePath, SO_PATH)) {
        outcome.soCount++;
    }
    if (OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(modulePath, RESULT_FILE_NAME)) {
        outcome.resultCount++;
        std::string content;
        if (!OHOS::AppPackingTool::ZipUtils::GetFileContentFromZip(modulePath, RESULT_FILE_NAME, content)) {
            return false;
        }
        outcome.resultJson += content;
    }
    // User input per module is preserved: module.json always stays inside the product
    return OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(modulePath, "module.json");
}

ComboOutcome RunCombination(const std::filesystem::path& comboDir, const std::string& manifestPath,
    bool intraApp, bool interApp)
{
    std::filesystem::path workDir = comboDir / "work";
    std::filesystem::path reportDir = comboDir / "report";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);
    std::filesystem::path hapA = comboDir / "module_a.hap";
    std::filesystem::path hapB = comboDir / "module_b.hap";
    CreateModuleZip(hapA, BuildModuleJsonWithApi("entryA", "phone"), true);
    CreateModuleZip(hapB, BuildModuleJsonWithApi("entryB", "phone"), true);
    std::list<std::string> modulePaths = {hapA.string(), hapB.string()};
    const std::list<std::string> originalModulePaths = modulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    ComboOutcome outcome;
    if (!deduplicator.DeduplicateModules(modulePaths, originalModulePaths,
        OHOS::AppPackingTool::DedupSwitch{intraApp, interApp ? manifestPath : std::string()},
        workDir.string(), reportDir.string())) {
        return ComboOutcome{-1, -1};
    }
    for (const auto& modulePath : modulePaths) {
        if (!CollectModuleOutcome(modulePath, outcome)) {
            return ComboOutcome{-1, -1};
        }
    }
    // The source packages given by the user stay untouched
    if (!OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(hapA.string(), SO_PATH) ||
        !OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(hapB.string(), SO_PATH)) {
        return ComboOutcome{-1, -1};
    }
    return outcome;
}

}  // namespace

class SODeduplicatorTest : public testing::Test {
protected:
    void SetUp() override
    {
        RemoveTestRoot();
        std::filesystem::create_directories(TEST_ROOT);
    }

    void TearDown() override
    {
        RemoveTestRoot();
    }
};

HWTEST_F(SODeduplicatorTest, DeduplicateModules_ModuleNameDoesNotControlExtractPath, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work";
    std::filesystem::path reportDir = root / "report";
    std::filesystem::path hapPath = root / "malicious.hap";
    std::filesystem::path escapedTarget = workDir / "target";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);

    ASSERT_TRUE(CreateModuleZip(hapPath, BuildModuleJson(MALICIOUS_MODULE_NAME), true));

    std::list<std::string> modulePaths = {hapPath.string()};
    const std::list<std::string> originalModulePaths = modulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    EXPECT_TRUE(deduplicator.DeduplicateModules(modulePaths, originalModulePaths, true,
        workDir.string(), reportDir.string()));
    EXPECT_FALSE(std::filesystem::exists(escapedTarget));

    size_t dedupRootCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(workDir)) {
        ASSERT_TRUE(entry.is_directory());
        EXPECT_EQ(entry.path().filename().string().find("so_dedup_"), 0);
        std::filesystem::path modulesRoot = entry.path() / "modules";
        ASSERT_TRUE(std::filesystem::is_directory(modulesRoot));
        std::vector<std::string> instanceDirectories;
        for (const auto& moduleEntry : std::filesystem::directory_iterator(modulesRoot)) {
            ASSERT_TRUE(moduleEntry.is_directory());
            instanceDirectories.push_back(moduleEntry.path().filename().string());
        }
        EXPECT_EQ(instanceDirectories, std::vector<std::string>({"0"}));
        ++dedupRootCount;
    }
    EXPECT_EQ(dedupRootCount, 1);
}

HWTEST_F(SODeduplicatorTest, DeduplicateModules_RejectsMismatchedOriginalPathsWhenDisabled, TestSize.Level0)
{
    std::list<std::string> modulePaths = {"module.hap"};
    const std::list<std::string> originalModulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    EXPECT_FALSE(deduplicator.DeduplicateModules(
        modulePaths, originalModulePaths, false, TEST_ROOT, TEST_ROOT));
    EXPECT_EQ(deduplicator.GetErrorMessage(), "Module path count does not match original module path count");
}

HWTEST_F(SODeduplicatorTest, DeduplicateModules_AllModuleDirectoriesUseInstanceIds, TestSize.Level0)
{
    const std::vector<std::string> moduleNames = {
        "../../target", "a/b", "a\\b", "1entry", ".entry", "_entry", "entry-name", "entry name", ""
    };
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work";
    std::filesystem::path reportDir = root / "report";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);
    std::list<std::string> modulePaths;
    size_t index = 0;
    for (const auto& moduleName : moduleNames) {
        std::filesystem::path hapPath = root / ("module_" + std::to_string(index) + ".hap");
        ASSERT_TRUE(CreateModuleZip(hapPath, BuildModuleJson(moduleName)));
        modulePaths.push_back(hapPath.string());
        ++index;
    }

    OHOS::AppPackingTool::SODeduplicator deduplicator;
    ASSERT_TRUE(deduplicator.DeduplicateModules(modulePaths, true, workDir.string(), reportDir.string()));

    std::filesystem::path dedupRoot;
    for (const auto& entry : std::filesystem::directory_iterator(workDir)) {
        if (entry.is_directory() && entry.path().filename().string().find("so_dedup_") == 0) {
            ASSERT_TRUE(dedupRoot.empty());
            dedupRoot = entry.path();
        }
    }
    ASSERT_FALSE(dedupRoot.empty());
    std::set<std::string> actualDirectories;
    for (const auto& moduleEntry : std::filesystem::directory_iterator(dedupRoot / "modules")) {
        ASSERT_TRUE(moduleEntry.is_directory());
        actualDirectories.insert(moduleEntry.path().filename().string());
    }
    std::set<std::string> expectedDirectories;
    for (size_t moduleIndex = 0; moduleIndex < moduleNames.size(); ++moduleIndex) {
        expectedDirectories.insert(std::to_string(moduleIndex));
    }
    EXPECT_EQ(actualDirectories, expectedDirectories);
    EXPECT_FALSE(std::filesystem::exists(workDir / "target"));
}

HWTEST_F(SODeduplicatorTest, DeduplicateModules_AllowsDuplicateModuleNames, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work";
    std::filesystem::path reportDir = root / "report";
    std::filesystem::path phoneHap = root / "phone.hap";
    std::filesystem::path tabletHap = root / "tablet.hap";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);

    ASSERT_TRUE(CreateModuleZip(phoneHap, BuildModuleJson("entry", "phone"), true));
    ASSERT_TRUE(CreateModuleZip(tabletHap, BuildModuleJson("entry", "tablet"), true));

    std::list<std::string> modulePaths = {phoneHap.string(), tabletHap.string()};
    std::list<std::string> originalModulePaths = {"original/phone.hap", "original/tablet.hap"};
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    ASSERT_TRUE(deduplicator.DeduplicateModules(
        modulePaths, originalModulePaths, true, workDir.string(), reportDir.string()));
    ASSERT_EQ(modulePaths.size(), 2);
    for (const auto& modulePath : modulePaths) {
        EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(modulePath, SO_PATH));
    }

    std::ifstream reportFile(reportDir / "so_dedup_report.json");
    std::string report((std::istreambuf_iterator<char>(reportFile)), std::istreambuf_iterator<char>());
    cJSON* parsed = cJSON_Parse(report.c_str());
    ASSERT_NE(parsed, nullptr);
    cJSON* modules = cJSON_GetObjectItemCaseSensitive(parsed, "modules");
    ASSERT_NE(modules, nullptr);
    cJSON* phoneModule = cJSON_GetObjectItemCaseSensitive(modules, "0");
    cJSON* tabletModule = cJSON_GetObjectItemCaseSensitive(modules, "1");
    ASSERT_NE(phoneModule, nullptr);
    ASSERT_NE(tabletModule, nullptr);
    EXPECT_STREQ(cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(phoneModule, "moduleName")), "entry");
    EXPECT_STREQ(cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(phoneModule, "filePath")),
        "original/phone.hap");
    EXPECT_STREQ(cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(tabletModule, "moduleName")), "entry");
    EXPECT_STREQ(cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(tabletModule, "filePath")),
        "original/tablet.hap");
    cJSON_Delete(parsed);
}

HWTEST_F(SODeduplicatorTest, DeduplicateModules_RemovesSoFromOnlyOneDuplicateNameModule, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work";
    std::filesystem::path reportDir = root / "report";
    std::filesystem::path phoneEntry = root / "phone_entry.hap";
    std::filesystem::path tabletEntry = root / "tablet_entry.hap";
    std::filesystem::path phoneFeature = root / "phone_feature.hap";
    std::filesystem::path tabletFeature = root / "tablet_feature.hap";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);

    ASSERT_TRUE(CreateModuleZip(phoneEntry, BuildModuleJson("phoneEntry", "phone"), true));
    ASSERT_TRUE(CreateModuleZip(tabletEntry, BuildModuleJson("tabletEntry", "tablet")));
    ASSERT_TRUE(CreateModuleZip(phoneFeature, BuildModuleJson("shared", "phone", "feature", false), true));
    ASSERT_TRUE(CreateModuleZip(tabletFeature, BuildModuleJson("shared", "tablet", "feature", false), true));

    std::list<std::string> modulePaths = {
        phoneEntry.string(), tabletEntry.string(), phoneFeature.string(), tabletFeature.string()
    };
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    ASSERT_TRUE(deduplicator.DeduplicateModules(modulePaths, true, workDir.string(), reportDir.string()));
    ASSERT_EQ(modulePaths.size(), 4);

    auto moduleIt = modulePaths.begin();
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(*moduleIt++, SO_PATH));
    EXPECT_FALSE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(*moduleIt++, SO_PATH));
    EXPECT_FALSE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(*moduleIt++, SO_PATH));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(*moduleIt, SO_PATH));
}

HWTEST_F(SODeduplicatorTest, DeduplicateModules_UsesDistributionFilterResource, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work";
    std::filesystem::path reportDir = root / "report";
    std::filesystem::path xldpiEntry = root / "xldpi_entry.hap";
    std::filesystem::path xxldpiEntry = root / "xxldpi_entry.hap";
    std::filesystem::path xxldpiFeature = root / "xxldpi_feature.hsp";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);

    const std::string xldpiFilter =
        R"({"distributionFilter":{"screenDensity":{"policy":"include","value":["xldpi"]}}})";
    const std::string xxldpiFilter =
        R"({"distributionFilter":{"screenDensity":{"policy":"include","value":["xxldpi"]}}})";
    ASSERT_TRUE(CreateModuleZip(xldpiEntry,
        BuildModuleJson("xldpiEntry", "phone", "entry", true, true), true, xldpiFilter));
    ASSERT_TRUE(CreateModuleZip(xxldpiEntry,
        BuildModuleJson("xxldpiEntry", "phone", "entry", true, true), true, xxldpiFilter));
    ASSERT_TRUE(CreateModuleZip(xxldpiFeature,
        BuildModuleJson("xxldpiFeature", "phone", "feature", false, true), true, xxldpiFilter));

    std::list<std::string> modulePaths = {
        xldpiEntry.string(), xxldpiEntry.string(), xxldpiFeature.string()
    };
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    ASSERT_TRUE(deduplicator.DeduplicateModules(modulePaths, true, workDir.string(), reportDir.string()));
    ASSERT_EQ(modulePaths.size(), 3);

    auto moduleIt = modulePaths.begin();
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(*moduleIt++, SO_PATH));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(*moduleIt++, SO_PATH));
    EXPECT_FALSE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(*moduleIt, SO_PATH));
}

/**
 * @tc.name: DeduplicateModules_SwitchCombinations_1800
 * @tc.desc: The intra-app and inter-app switches are independent; with both on the exclusion sets are
 *           merged so the same .so is removed once and each deduplicated module gets a result file.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_SwitchCombinations_1800, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path probe = root / "probe.bin";
    ASSERT_TRUE(WriteTextFile(probe, "same so content"));
    std::string appSha = OHOS::AppPackingTool::Utils::GetSha256File(probe.string());
    ASSERT_FALSE(appSha.empty());
    std::filesystem::path manifestPath = root / "deduplist_sdk.json";
    ASSERT_TRUE(WriteTextFile(manifestPath, BuildDeduplistManifest(appSha)));

    std::filesystem::create_directories(root / "off_off");
    std::filesystem::create_directories(root / "intra_only");
    std::filesystem::create_directories(root / "inter_only");
    std::filesystem::create_directories(root / "intra_and_inter");

    ComboOutcome offOff = RunCombination(root / "off_off", manifestPath.string(), false, false);
    EXPECT_EQ(offOff.soCount, 2);
    EXPECT_EQ(offOff.resultCount, 0);

    ComboOutcome intraOnly = RunCombination(root / "intra_only", manifestPath.string(), true, false);
    EXPECT_EQ(intraOnly.soCount, 1);
    EXPECT_EQ(intraOnly.resultCount, 0);

    ComboOutcome interOnly = RunCombination(root / "inter_only", manifestPath.string(), false, true);
    EXPECT_EQ(interOnly.soCount, 0);
    EXPECT_EQ(interOnly.resultCount, 2);

    ComboOutcome intraAndInter = RunCombination(root / "intra_and_inter", manifestPath.string(), true, true);
    EXPECT_EQ(intraAndInter.soCount, 0);
    // The intra-app stage already removed the so of one module, so only the module whose so the inter-app
    // stage removed records a result file, once and with the complete package relative path
    EXPECT_EQ(intraAndInter.resultCount, 1);
    EXPECT_NE(intraAndInter.resultJson.find(SO_PATH), std::string::npos);
    EXPECT_EQ(CountOccurrences(intraAndInter.resultJson, SO_PATH), 1U);
}

/**
 * @tc.name: DeduplicateModules_SkipsModuleWithExistingResultFile_1900
 * @tc.desc: A hap/hsp that already carries dedupfiles.json is not deduplicated again: its so files and
 *           its result file stay as they are, while the other module is deduplicated normally.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_SkipsModuleWithExistingResultFile_1900, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path probe = root / "probe_skip.bin";
    ASSERT_TRUE(WriteTextFile(probe, "same so content"));
    std::string appSha = OHOS::AppPackingTool::Utils::GetSha256File(probe.string());
    ASSERT_FALSE(appSha.empty());
    std::filesystem::path manifestPath = root / "deduplist_sdk_skip.json";
    ASSERT_TRUE(WriteTextFile(manifestPath, BuildDeduplistManifest(appSha)));

    std::filesystem::path workDir = root / "work_skip";
    std::filesystem::path reportDir = root / "report_skip";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);
    std::filesystem::path hapA = root / "module_a_skip.hap";
    std::filesystem::path hapB = root / "module_b_skip.hap";
    ASSERT_TRUE(CreateModuleZipWithEntries(hapA, BuildModuleJsonWithApi("entryA", "phone"),
        {{SO_PATH, "same so content"}, {RESULT_FILE_NAME, "{\"version\":\"1.0\"}"}}));
    ASSERT_TRUE(CreateModuleZip(hapB, BuildModuleJsonWithApi("entryB", "phone"), true));

    std::list<std::string> modulePaths = {hapA.string(), hapB.string()};
    const std::list<std::string> originalModulePaths = modulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    ASSERT_TRUE(deduplicator.DeduplicateModules(modulePaths, originalModulePaths,
        OHOS::AppPackingTool::DedupSwitch{false, manifestPath.string()},
        workDir.string(), reportDir.string()));

    auto moduleIt = modulePaths.begin();
    const std::string repackedA = *moduleIt++;
    const std::string repackedB = *moduleIt;
    // Module A was skipped: its so and its own result file are kept untouched
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(repackedA, SO_PATH));
    std::string contentA;
    ASSERT_TRUE(OHOS::AppPackingTool::ZipUtils::GetFileContentFromZip(repackedA, RESULT_FILE_NAME, contentA));
    EXPECT_EQ(contentA, "{\"version\":\"1.0\"}");
    // Module B is deduplicated as usual
    EXPECT_FALSE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(repackedB, SO_PATH));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(repackedB, RESULT_FILE_NAME));
    // The user input stays untouched
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(hapA.string(), SO_PATH));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(hapB.string(), SO_PATH));
}

/**
 * @tc.name: DeduplicateModules_NestedSoPathRemovedAndRecorded_2000
 * @tc.desc: The so may sit under any sub directory of libs/<abi>; it is removed and the result file
 *           records its complete package relative path.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_NestedSoPathRemovedAndRecorded_2000, TestSize.Level0)
{
    const std::string nestedSoPath = "libs/arm64-v8a/plugins/libsame.so";
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path probe = root / "probe_nested.bin";
    ASSERT_TRUE(WriteTextFile(probe, "same so content"));
    std::string appSha = OHOS::AppPackingTool::Utils::GetSha256File(probe.string());
    ASSERT_FALSE(appSha.empty());
    std::filesystem::path manifestPath = root / "deduplist_sdk_nested.json";
    ASSERT_TRUE(WriteTextFile(manifestPath, BuildDeduplistManifest(appSha)));

    std::filesystem::path workDir = root / "work_nested";
    std::filesystem::path reportDir = root / "report_nested";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);
    std::filesystem::path hap = root / "module_nested.hap";
    ASSERT_TRUE(CreateModuleZipWithEntries(hap, BuildModuleJsonWithApi("entryNested", "phone"),
        {{nestedSoPath, "same so content"}}));

    std::list<std::string> modulePaths = {hap.string()};
    const std::list<std::string> originalModulePaths = modulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    ASSERT_TRUE(deduplicator.DeduplicateModules(modulePaths, originalModulePaths,
        OHOS::AppPackingTool::DedupSwitch{false, manifestPath.string()},
        workDir.string(), reportDir.string()));

    const std::string repacked = modulePaths.front();
    EXPECT_FALSE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(repacked, nestedSoPath));
    std::string resultJson;
    ASSERT_TRUE(OHOS::AppPackingTool::ZipUtils::GetFileContentFromZip(repacked, RESULT_FILE_NAME, resultJson));
    EXPECT_NE(resultJson.find(nestedSoPath), std::string::npos);
    EXPECT_NE(resultJson.find(appSha), std::string::npos);
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(hap.string(), nestedSoPath));
}

/**
 * @tc.name: DeduplicateModules_InvalidDeduplistSdk_2100
 * @tc.desc: An unparsable deduplist sdk aborts the inter-app stage with the inter-app error and keeps the
 *           user input untouched.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_InvalidDeduplistSdk_2100, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work_bad_inter";
    std::filesystem::path reportDir = root / "report_bad_inter";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);
    // Missing dedupMinApiVersion: the manifest cannot be parsed
    std::filesystem::path manifestPath = root / "deduplist_sdk_invalid.json";
    ASSERT_TRUE(WriteTextFile(manifestPath, "{\"deviceType\":[\"phone\"],\"libraries\":[]}"));

    std::filesystem::path hap = root / "module_bad_inter.hap";
    ASSERT_TRUE(CreateModuleZip(hap, BuildModuleJsonWithApi("entryBadInter", "phone"), true));

    std::list<std::string> modulePaths = {hap.string()};
    const std::list<std::string> originalModulePaths = modulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    EXPECT_FALSE(deduplicator.DeduplicateModules(modulePaths, originalModulePaths,
        OHOS::AppPackingTool::DedupSwitch{false, manifestPath.string()},
        workDir.string(), reportDir.string()));
    // The user input is not modified by a failed inter-app stage
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(hap.string(), SO_PATH));
}

/**
 * @tc.name: DeduplicateModules_EmptyModulePathsWithSwitchesOn_2200
 * @tc.desc: An empty module list short circuits successfully once a stage is enabled and creates no
 *           extraction root under the work directory.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_EmptyModulePathsWithSwitchesOn_2200, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work_empty";
    std::filesystem::path reportDir = root / "report_empty";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);

    std::list<std::string> modulePaths;
    const std::list<std::string> originalModulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    EXPECT_TRUE(deduplicator.DeduplicateModules(modulePaths, originalModulePaths,
        OHOS::AppPackingTool::DedupSwitch{true, "unused.json"},
        workDir.string(), reportDir.string()));
    size_t dedupRootCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(workDir)) {
        if (entry.path().filename().string().find("so_dedup_") == 0) {
            ++dedupRootCount;
        }
    }
    EXPECT_EQ(dedupRootCount, 0U);
}

/**
 * @tc.name: DeduplicateModules_FaModuleSkippedByInterApp_2300
 * @tc.desc: An FA module carries config.json instead of module.json, does not participate in the
 *           inter-app stage and keeps its so, while the Stage module is deduplicated as usual.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_FaModuleSkippedByInterApp_2300, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path probe = root / "probe_fa.bin";
    ASSERT_TRUE(WriteTextFile(probe, "same so content"));
    std::string appSha = OHOS::AppPackingTool::Utils::GetSha256File(probe.string());
    ASSERT_FALSE(appSha.empty());
    std::filesystem::path manifestPath = root / "deduplist_sdk_fa.json";
    ASSERT_TRUE(WriteTextFile(manifestPath, BuildDeduplistManifest(appSha)));

    std::filesystem::path workDir = root / "work_fa";
    std::filesystem::path reportDir = root / "report_fa";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);
    std::filesystem::path faHap = root / "module_fa.hap";
    ASSERT_TRUE(CreateModuleZipWithRawEntries(faHap, {
        {"config.json", "{\"module\":{\"name\":\"faEntry\",\"type\":\"entry\"}}"},
        {SO_PATH, "same so content"}}));
    std::filesystem::path stageHap = root / "module_stage.hap";
    ASSERT_TRUE(CreateModuleZip(stageHap, BuildModuleJsonWithApi("entryStage", "phone"), true));

    std::list<std::string> modulePaths = {faHap.string(), stageHap.string()};
    const std::list<std::string> originalModulePaths = modulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    ASSERT_TRUE(deduplicator.DeduplicateModules(modulePaths, originalModulePaths,
        OHOS::AppPackingTool::DedupSwitch{false, manifestPath.string()},
        workDir.string(), reportDir.string()));

    auto moduleIt = modulePaths.begin();
    const std::string repackedFa = *moduleIt++;
    const std::string repackedStage = *moduleIt;
    // The FA module keeps its so and records no result file
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(repackedFa, SO_PATH));
    EXPECT_FALSE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(repackedFa, RESULT_FILE_NAME));
    // The Stage module is deduplicated as usual
    EXPECT_FALSE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(repackedStage, SO_PATH));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(repackedStage, RESULT_FILE_NAME));
    // The user inputs stay untouched
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(faHap.string(), SO_PATH));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(stageHap.string(), SO_PATH));
}

/**
 * @tc.name: DeduplicateModules_MissingOrInvalidConfiguration_2400
 * @tc.desc: A module without module.json / config.json and a module with an unparsable config.json are
 *           both skipped with a warning; the package without any entry module still reports the none
 *           strategy successfully.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_MissingOrInvalidConfiguration_2400, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work_noconfig";
    std::filesystem::path reportDir = root / "report_noconfig";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);

    // Neither module.json nor config.json: the only module is skipped, so no entry device remains
    std::filesystem::path bareHap = root / "module_bare.hap";
    ASSERT_TRUE(CreateModuleZipWithRawEntries(bareHap, {{SO_PATH, "same so content"}}));
    std::list<std::string> barePaths = {bareHap.string()};
    OHOS::AppPackingTool::SODeduplicator bareDeduplicator;
    EXPECT_TRUE(bareDeduplicator.DeduplicateModules(barePaths, true,
        workDir.string(), reportDir.string()));
    EXPECT_TRUE(std::filesystem::exists(reportDir / "so_dedup_report.json"));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(barePaths.front(), SO_PATH));

    // An unparsable config.json is skipped while the Stage module is still processed normally
    std::filesystem::path brokenHap = root / "module_broken.hap";
    ASSERT_TRUE(CreateModuleZipWithRawEntries(brokenHap, {{"config.json", "{not json"},
        {SO_PATH, "same so content"}}));
    std::filesystem::path stageHap = root / "module_mixed.hap";
    ASSERT_TRUE(CreateModuleZip(stageHap, BuildModuleJson("entryMixed", "phone"), true));
    std::list<std::string> mixedPaths = {brokenHap.string(), stageHap.string()};
    OHOS::AppPackingTool::SODeduplicator mixedDeduplicator;
    EXPECT_TRUE(mixedDeduplicator.DeduplicateModules(mixedPaths, true,
        workDir.string(), reportDir.string()));
    // The broken module is not deduplicated and the stage module keeps its only so (no duplicate)
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(mixedPaths.front(), SO_PATH));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(mixedPaths.back(), SO_PATH));
}

/**
 * @tc.name: DeduplicateModules_InvalidDistributionFilterAborts_2500
 * @tc.desc: A referenced distributionFilter profile that is missing from the package aborts the whole
 *           deduplication and keeps the user input untouched.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_InvalidDistributionFilterAborts_2500, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work_distro";
    std::filesystem::path reportDir = root / "report_distro";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);
    std::filesystem::path hap = root / "module_distro.hap";
    ASSERT_TRUE(CreateModuleZipWithRawEntries(hap, {
        {"module.json", BuildStageModuleJson("entryDistro",
            ",\"extractNativeLibs\":true,"
            "\"metadata\":[{\"name\":\"distributionFilter\",\"resource\":\"$profile:density_filter\"}]")},
        {SO_PATH, "same so content"}}));

    std::list<std::string> modulePaths = {hap.string()};
    const std::list<std::string> originalModulePaths = modulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    EXPECT_FALSE(deduplicator.DeduplicateModules(modulePaths, originalModulePaths,
        OHOS::AppPackingTool::DedupSwitch{true, ""}, workDir.string(), reportDir.string()));
    EXPECT_NE(deduplicator.GetErrorMessage().find("Failed to parse distributionFilter"), std::string::npos);
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(hap.string(), SO_PATH));
}

/**
 * @tc.name: DeduplicateModules_InterAppGatesSkipWholePackage_2600
 * @tc.desc: The application level gates decide for the whole package: a compatibleSdkVersion below
 *           dedupMinApiVersion and a driver extensionAbility both skip inter-app dedup entirely while
 *           the so files and the user inputs stay untouched.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_InterAppGatesSkipWholePackage_2600, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path probe = root / "probe_gate.bin";
    ASSERT_TRUE(WriteTextFile(probe, "same so content"));
    std::string appSha = OHOS::AppPackingTool::Utils::GetSha256File(probe.string());
    ASSERT_FALSE(appSha.empty());
    std::filesystem::path manifestPath = root / "deduplist_sdk_gate.json";
    ASSERT_TRUE(WriteTextFile(manifestPath, BuildDeduplistManifest(appSha)));

    std::filesystem::path workDir = root / "work_gate";
    std::filesystem::path reportDir = root / "report_gate";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);

    // The module minAPIVersion is lower than dedupMinApiVersion
    std::filesystem::path lowHap = root / "module_lowapi.hap";
    ASSERT_TRUE(CreateModuleZip(lowHap, BuildModuleJsonWithApi("entryLowApi", "phone", 9), true));
    std::list<std::string> lowPaths = {lowHap.string()};
    const std::list<std::string> lowOriginal = lowPaths;
    OHOS::AppPackingTool::SODeduplicator lowDeduplicator;
    EXPECT_TRUE(lowDeduplicator.DeduplicateModules(lowPaths, lowOriginal,
        OHOS::AppPackingTool::DedupSwitch{false, manifestPath.string()},
        workDir.string(), reportDir.string()));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(lowPaths.front(), SO_PATH));
    EXPECT_FALSE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(lowPaths.front(), RESULT_FILE_NAME));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(lowHap.string(), SO_PATH));

    // A driver extensionAbility skips the whole package as well
    std::filesystem::path driverHap = root / "module_driver.hap";
    ASSERT_TRUE(CreateModuleZipWithRawEntries(driverHap, {
        {"module.json", BuildStageModuleJson("entryDriver",
            ",\"extractNativeLibs\":true,"
            "\"extensionAbilities\":[{\"name\":\"drv\",\"type\":\"driver\"}]")},
        {SO_PATH, "same so content"}}));
    std::list<std::string> driverPaths = {driverHap.string()};
    const std::list<std::string> driverOriginal = driverPaths;
    OHOS::AppPackingTool::SODeduplicator driverDeduplicator;
    EXPECT_TRUE(driverDeduplicator.DeduplicateModules(driverPaths, driverOriginal,
        OHOS::AppPackingTool::DedupSwitch{false, manifestPath.string()},
        workDir.string(), reportDir.string()));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(driverPaths.front(), SO_PATH));
    EXPECT_FALSE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(driverPaths.front(), RESULT_FILE_NAME));
    EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(driverHap.string(), SO_PATH));
}

/**
 * @tc.name: DeduplicateModules_IneligibleModulesGenerateNoneReport_2700
 * @tc.desc: Modules that do not extract so, enable libIsolation or use compileSdkType OpenHarmony are
 *           all filtered out, so the intra-app stage reports the none strategy and removes nothing.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_IneligibleModulesGenerateNoneReport_2700, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work_ineligible";
    std::filesystem::path reportDir = root / "report_ineligible";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);

    std::filesystem::path noExtractHap = root / "module_noextract.hap";
    ASSERT_TRUE(CreateModuleZipWithRawEntries(noExtractHap, {
        {"module.json", BuildStageModuleJson("entryNoExtract", "")},
        {SO_PATH, "same so content"}}));
    std::filesystem::path isolationHap = root / "module_isolation.hap";
    ASSERT_TRUE(CreateModuleZipWithRawEntries(isolationHap, {
        {"module.json", BuildStageModuleJson("entryIsolation",
            ",\"extractNativeLibs\":true,\"libIsolation\":true")},
        {SO_PATH, "same so content"}}));
    std::filesystem::path openHarmonyHap = root / "module_oh.hap";
    ASSERT_TRUE(CreateModuleZipWithRawEntries(openHarmonyHap, {
        {"module.json", BuildStageModuleJson("entryOh", ",\"extractNativeLibs\":true", 26, "OpenHarmony")},
        {SO_PATH, "same so content"}}));

    std::list<std::string> modulePaths = {
        noExtractHap.string(), isolationHap.string(), openHarmonyHap.string()
    };
    const std::list<std::string> originalModulePaths = modulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    EXPECT_TRUE(deduplicator.DeduplicateModules(modulePaths, originalModulePaths,
        OHOS::AppPackingTool::DedupSwitch{true, ""}, workDir.string(), reportDir.string()));
    // The none strategy report is still generated and no so is removed
    EXPECT_TRUE(std::filesystem::exists(reportDir / "so_dedup_report.json"));
    for (const auto& modulePath : modulePaths) {
        EXPECT_TRUE(OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(modulePath, SO_PATH));
    }
}

/**
 * @tc.name: DeduplicateModules_GreedyAlgorithmForLargeDuplicateGroup_3000
 * @tc.desc: A duplicate group larger than the exact algorithm threshold takes the greedy solver, which
 *           keeps a single copy of the so among the 21 identical modules.
 */
HWTEST_F(SODeduplicatorTest, DeduplicateModules_GreedyAlgorithmForLargeDuplicateGroup_3000, TestSize.Level0)
{
    std::filesystem::path root(TEST_ROOT);
    std::filesystem::path workDir = root / "work_greedy";
    std::filesystem::path reportDir = root / "report_greedy";
    std::filesystem::create_directories(workDir);
    std::filesystem::create_directories(reportDir);

    std::list<std::string> modulePaths;
    for (int32_t index = 0; index < 21; ++index) {
        std::filesystem::path hap = root / ("module_greedy_" + std::to_string(index) + ".hap");
        ASSERT_TRUE(CreateModuleZip(hap,
            BuildModuleJsonWithApi("entryGreedy" + std::to_string(index), "phone"), true));
        modulePaths.push_back(hap.string());
    }
    const std::list<std::string> originalModulePaths = modulePaths;
    OHOS::AppPackingTool::SODeduplicator deduplicator;
    ASSERT_TRUE(deduplicator.DeduplicateModules(modulePaths, originalModulePaths,
        OHOS::AppPackingTool::DedupSwitch{true, ""}, workDir.string(), reportDir.string()));
    ASSERT_EQ(modulePaths.size(), 21U);

    int32_t keptCount = 0;
    for (const auto& modulePath : modulePaths) {
        if (OHOS::AppPackingTool::ZipUtils::IsFileExistsInZip(modulePath, SO_PATH)) {
            ++keptCount;
        }
    }
    EXPECT_EQ(keptCount, 1);
    EXPECT_TRUE(std::filesystem::exists(reportDir / "so_dedup_report.json"));
}
