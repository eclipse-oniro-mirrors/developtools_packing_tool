/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "dedup/so_deduplicator.h"
#include "dedup/device_calculator.h"
#include "dedup/module_calculator.h"
#include "dedup/exact_solver.h"
#include "dedup/greedy_solver.h"
#include "dedup/report_generator.h"
#include "dedup/inter_app_deduplicator.h"
#include "dedup/dedup_log.h"
#include "dedup/dedup_error.h"
#include "zip_utils.h"
#include "zip_wrapper.h"
#include "unzip_wrapper.h"
#include "utils.h"
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <limits>
#include <regex>
#include <set>
#include <sstream>
#include <tuple>

namespace OHOS {
namespace AppPackingTool {
namespace {

// 读取文件摘要时的缓冲区大小
constexpr size_t DIGEST_BUFFER_SIZE = 8192;

// 统一包内路径分隔符
std::string NormalizeSoPath(const std::string& path)
{
    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    return normalized;
}

// 直接复制源 ZIP 条目到目标 ZIP，跳过待排除的 SO；返回是否存在复制错误
bool CopyZipEntriesExcluding(UnzipWrapper& srcUnzip, ZipWrapper& destZip,
    const std::set<std::string>& excludedPaths, int32_t& copiedCount, int32_t& skippedCount)
{
    unzFile srcFile = srcUnzip.GetUnzipFile();
    zipFile destFile = destZip.GetZipFile();
    do {
        unz_file_info64 fileInfo;
        if (unzGetCurrentFileInfo64(srcFile, &fileInfo, nullptr, 0,
            nullptr, 0, nullptr, 0) != UNZ_OK) {
            LOG(ERROR) << "Failed to get file info for entry";
            return false;
        }
        std::vector<char> fileName(fileInfo.size_filename + 1, '\0');
        if (unzGetCurrentFileInfo64(srcFile, &fileInfo, fileName.data(), fileName.size(),
            nullptr, 0, nullptr, 0) != UNZ_OK) {
            LOG(ERROR) << "Failed to get file name for entry";
            return false;
        }
        std::string entryName(fileName.data(), fileInfo.size_filename);
        std::replace(entryName.begin(), entryName.end(), '\\', '/');
        if (excludedPaths.find(entryName) != excludedPaths.end()) {
            LOG(DEBUG) << "Skipping removed SO: " << entryName;
            skippedCount++;
            continue;
        }
        if (destZip.AddRawEntryToZip(destFile, srcFile, entryName) != ZIP_ERR_SUCCESS) {
            LOG(ERROR) << "Failed to copy entry: " << entryName;
            return false;
        }
        copiedCount++;
    } while (unzGoToNextFile(srcFile) == UNZ_OK);
    return true;
}

// 流式读取文件并更新 MD5（可选 SHA-256）上下文
void UpdateDigests(std::ifstream& file, MD5_CTX& md5Context, SHA256_CTX* sha256Context)
{
    char buffer[DIGEST_BUFFER_SIZE];
    while (file.read(buffer, DIGEST_BUFFER_SIZE)) {
        MD5_Update(&md5Context, buffer, file.gcount());
        if (sha256Context != nullptr) {
            SHA256_Update(sha256Context, buffer, file.gcount());
        }
    }
    if (file.gcount() > 0) {
        MD5_Update(&md5Context, buffer, file.gcount());
        if (sha256Context != nullptr) {
            SHA256_Update(sha256Context, buffer, file.gcount());
        }
    }
}

// 将摘要转换为小写十六进制字符串，与 Utils::GetSha256File 输出一致
std::string DigestToHex(const unsigned char* digest, size_t length)
{
    std::stringstream stream;
    stream << std::hex << std::setfill('0');
    for (size_t index = 0; index < length; ++index) {
        stream << std::setw(MD5_HEX_WIDTH) << static_cast<int>(digest[index]);
    }
    return stream.str();
}

}  // namespace

SODeduplicator::SODeduplicator()
    : dedupSuccess_(false) {}

SODeduplicator::~SODeduplicator()
{}

void SODeduplicator::SetError(const std::string& error)
{
    errorMessage_ = error;
    dedupSuccess_ = false;
    interAppFailure_ = false;
    LOG(ERROR) << FormatDedupError(error);
}

void SODeduplicator::SetInterAppError(const std::string& error)
{
    errorMessage_ = error;
    dedupSuccess_ = false;
    interAppFailure_ = true;
    LOG(ERROR) << FormatInterAppDedupError(error);
}

bool SODeduplicator::IsSoFile(const std::string& fileName)
{
    try {
        // Match all .so files and versioned .so files (e.g., libtest.so.1.2.3)
        static const std::regex SO_PATTERN(".*\\.so(\\.\\d+)*", std::regex_constants::icase);
        return std::regex_match(fileName, SO_PATTERN);
    } catch (const std::exception& e) {
        LOG(ERROR) << FormatDedupError("Failed to match SO file name: " + std::string(e.what()));
        throw;
    }
}

bool SODeduplicator::CalculateFileDigests(const std::string& filePath, std::string& md5, std::string& sha256,
    bool collectSha256)
{
    md5.clear();
    sha256.clear();
    std::string realFilePath;
    if (!Utils::GetRealPath(filePath, realFilePath) || !std::filesystem::is_regular_file(realFilePath)) {
        LOG(ERROR) << FormatDedupError("Invalid file path for digest calculation: " + filePath);
        return false;
    }
    std::ifstream file(realFilePath, std::ios::binary);
    if (!file.is_open()) {
        LOG(ERROR) << FormatDedupError("Failed to open file for digest calculation: " + filePath);
        return false;
    }

    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    SHA256_CTX sha256Context;
    if (collectSha256) {
        SHA256_Init(&sha256Context);
    }
    UpdateDigests(file, md5Context, collectSha256 ? &sha256Context : nullptr);

    unsigned char md5Digest[MD5_DIGEST_LENGTH];
    MD5_Final(md5Digest, &md5Context);
    md5 = DigestToHex(md5Digest, MD5_DIGEST_LENGTH);
    if (collectSha256) {
        unsigned char sha256Digest[SHA256_DIGEST_LENGTH];
        SHA256_Final(sha256Digest, &sha256Context);
        sha256 = DigestToHex(sha256Digest, SHA256_DIGEST_LENGTH);
    }
    LOG(DEBUG) << "Calculated digests for file: " << filePath << " (MD5: "
               << md5.substr(0, MD5_DISPLAY_LENGTH) << "...)";
    return !md5.empty() && (!collectSha256 || !sha256.empty());
}

std::vector<const SODeduplicator::ModuleDedupContext*> SODeduplicator::FilterEntryModules(
    const std::vector<ModuleDedupContext>& allModules)
{
    std::vector<const ModuleDedupContext*> entryModules;

    for (const auto& context : allModules) {
        if (!context.module) {
            continue;
        }

        if (ModuleCalculator::IsValidForDedup(context.config) && context.config.moduleType == "entry") {
            entryModules.push_back(&context);
        }
    }

    LOG(DEBUG) << "Filtered " << entryModules.size() << " entry modules from "
              << allModules.size() << " total modules";

    return entryModules;
}

std::vector<const SODeduplicator::ModuleDedupContext*> SODeduplicator::FilterDedupEligibleModules(
    const std::vector<ModuleDedupContext>& allModules)
{
    std::vector<const ModuleDedupContext*> eligibleModules;

    for (const auto& context : allModules) {
        if (!context.module) {
            continue;
        }

        try {
            const ModuleConfig& config = context.config;
            if (!config.stageModel) {
                continue;
            }
            if (!ModuleCalculator::IsValidForDedup(config)) {
                continue;
            }

            // Condition 1: Will extract SO (compressNativeLibs or extractNativeLibs is true)
            if (!config.compressNativeLibs && !config.extractNativeLibs) {
                LOG(DEBUG) << "Module " << config.moduleName << " not eligible: SO not extracted";
                continue;
            }

            // Condition 2: libIsolation is false
            if (config.libIsolation) {
                LOG(DEBUG) << "Module " << config.moduleName << " not eligible: libIsolation enabled";
                continue;
            }

            // Condition 3: compileSdkType cannot be "OpenHarmony"
            if (config.compileSdkType == "OpenHarmony") {
                continue;
            }

            eligibleModules.push_back(&context);
        } catch (const std::exception& e) {
            LOG(WARNING) << "Failed to check module eligibility: " << e.what();
        }
    }

    LOG(DEBUG) << "Found " << eligibleModules.size() << " dedup-eligible modules from "
              << allModules.size() << " total modules";

    return eligibleModules;
}

std::map<std::string, std::vector<SoInfo>> SODeduplicator::CollectSoFiles(
    const std::vector<const ModuleDedupContext*>& modules, bool collectSha256)
{
    std::map<std::string, std::vector<SoInfo>> moduleSoMap;

    LOG(DEBUG) << "Collecting SO files from " << modules.size() << " modules";

    for (const auto* context : modules) {
        if (context == nullptr || !context->module) {
            continue;
        }

        try {
            const std::filesystem::path& modulePath = context->extractedPath;

            // Check if module directory exists
            if (!std::filesystem::exists(modulePath)) {
                SetError("Module directory does not exist: " + modulePath.string());
                return {};
            }

            // Traverse module directory to find .so files
            std::vector<SoInfo> soFiles;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(modulePath)) {
                if (!entry.is_regular_file()) {
                    continue;
                }
                std::string filePath = entry.path().string();
                std::string fileName = entry.path().filename().string();
                if (!IsSoFile(fileName)) {
                    continue;
                }
                std::string md5;
                std::string sha256;
                if (!CalculateFileDigests(filePath, md5, sha256, collectSha256)) {
                    SetError("Failed to calculate digests for file: " + filePath);
                    return {};
                }

                std::string relativePath = std::filesystem::relative(entry.path(), modulePath).generic_string();
                uintmax_t rawFileSize = std::filesystem::file_size(entry.path());
                if (rawFileSize > static_cast<uintmax_t>(std::numeric_limits<int64_t>::max())) {
                    SetError("SO file size exceeds the supported range: " + filePath);
                    return {};
                }
                SoInfo soInfo;
                soInfo.relativePath = relativePath;
                soInfo.md5 = md5;
                soInfo.sha256 = sha256;
                soInfo.sourceModule = context->moduleId;
                soInfo.fileSize = static_cast<int64_t>(rawFileSize);

                soFiles.push_back(soInfo);
                LOG(DEBUG) << "Found SO file: " << relativePath << " (MD5: "
                          << md5.substr(0, MD5_DISPLAY_LENGTH) << "...)";
            }

            if (!soFiles.empty()) {
                moduleSoMap[context->moduleId] = soFiles;
                LOG(DEBUG) << "Module " << context->config.moduleName << " (" << context->moduleId
                           << ") contains " << soFiles.size() << " SO files";
            }
        } catch (const std::exception& e) {
            SetError("Failed to collect SO files from module: " + std::string(e.what()));
            return {};
        }
    }

    size_t totalSoCount = 0;
    for (const auto& [moduleName, soList] : moduleSoMap) {
        totalSoCount += soList.size();
    }

    LOG(DEBUG) << "Collected " << totalSoCount << " SO files from "
              << moduleSoMap.size() << " modules";

    return moduleSoMap;
}

std::vector<DuplicateSoGroup> SODeduplicator::GroupDuplicateSos(
    const std::map<std::string, std::vector<SoInfo>>& moduleSoMap)
{

    std::vector<DuplicateSoGroup> duplicateGroups;
    // Group by module-relative path and MD5.
    // SOs with same content but different install paths cannot replace each other.
    std::map<std::pair<std::string, std::string>, std::vector<SoInfo>> duplicateMap;

    for (const auto& [moduleName, soList] : moduleSoMap) {
        for (const auto& so : soList) {
            duplicateMap[{so.relativePath, so.md5}].push_back(so);
        }
    }

    // Create duplicate SO groups
    for (const auto& [identity, soList] : duplicateMap) {
        if (soList.size() > 1) { // Only process duplicate SOs
            DuplicateSoGroup group;
            group.relativePath = identity.first;
            group.md5 = identity.second;
            group.soList = soList;
            duplicateGroups.push_back(group);

            LOG(DEBUG) << "Found duplicate SO group " << group.relativePath << " with MD5 "
                      << group.md5.substr(0, MD5_DISPLAY_LENGTH)
                      << "... (" << soList.size() << " SOs)";
        }
    }

    LOG(DEBUG) << "Found " << duplicateGroups.size() << " duplicate SO groups";

    return duplicateGroups;
}

bool SODeduplicator::ApplyDedupPlan(
    const DedupPlan& plan,
    const std::vector<ModuleDedupContext>& modules)
{
    LOG(DEBUG) << "Applying deduplication plan";

    std::map<std::string, std::filesystem::path> extractedPaths;
    for (const auto& context : modules) {
        extractedPaths[context.moduleId] = context.extractedPath;
    }

    int32_t removedCount = 0;

    for (const auto& [moduleId, soPaths] : plan.removedSoMap) {
        auto moduleIt = extractedPaths.find(moduleId);
        if (moduleIt == extractedPaths.end()) {
            SetError("Module instance does not exist for deduplication plan: " + moduleId);
            return false;
        }
        for (const auto& soPath : soPaths) {
            std::filesystem::path fullFilePath = moduleIt->second / soPath;

            try {
                if (std::filesystem::exists(fullFilePath)) {
                    removedCount++;
                    LOG(DEBUG) << "SO file scheduled for exclusion: " << fullFilePath.string();
                } else {
                    LOG(WARNING) << "SO file does not exist, cannot exclude: " << fullFilePath.string();
                }
            } catch (const std::exception& e) {
                LOG(ERROR) << FormatDedupError(
                    "Failed to check SO file: " + fullFilePath.string() + " - " + e.what());
                SetError("Failed to check SO file: " + fullFilePath.string());
                return false;
            }
        }
    }

    LOG(DEBUG) << "Successfully applied deduplication plan: scheduled " << removedCount
               << " SO files for exclusion";

    return true;
}

std::set<std::string> SODeduplicator::CollectExcludedSoPaths(const std::string& moduleId,
    const DedupPlan& plan) const
{
    std::set<std::string> excludedPaths;
    auto it = plan.removedSoMap.find(moduleId);
    if (it != plan.removedSoMap.end()) {
        for (const auto& soPath : it->second) {
            excludedPaths.insert(NormalizeSoPath(soPath));
        }
    }
    auto interAppIt = interAppRemovedSoMap_.find(moduleId);
    if (interAppIt != interAppRemovedSoMap_.end()) {
        for (const auto& soPath : interAppIt->second) {
            excludedPaths.insert(NormalizeSoPath(soPath));
        }
    }
    return excludedPaths;
}

bool SODeduplicator::OpenRepackZips(const std::string& sourceZip, const std::string& targetZip,
    UnzipWrapper& srcUnzip, ZipWrapper& destZip)
{
    std::string srcPath = sourceZip;
    if (srcUnzip.Open(srcPath) != UNZ_OK) {
        SetError("Failed to open source ZIP: " + sourceZip);
        return false;
    }
    std::string targetPath = targetZip;
    if (destZip.Open(targetPath, APPEND_STATUS_CREATE) != ZIP_OK) {
        srcUnzip.Close();
        SetError("Failed to create target ZIP: " + targetZip);
        return false;
    }
    if (unzGoToFirstFile(srcUnzip.GetUnzipFile()) != UNZ_OK) {
        srcUnzip.Close();
        destZip.Close();
        SetError("Failed to iterate source ZIP entries");
        return false;
    }
    return true;
}

bool SODeduplicator::RepackModuleExcludingSOs(
    const std::string& sourceZip,
    const std::string& targetZip,
    const std::string& moduleId,
    const DedupPlan& plan)
{
    LOG(DEBUG) << "Repacking module instance " << moduleId
              << " from " << sourceZip << " to " << targetZip;
    std::set<std::string> excludedPaths = CollectExcludedSoPaths(moduleId, plan);
    UnzipWrapper srcUnzip;
    ZipWrapper destZip;
    if (!OpenRepackZips(sourceZip, targetZip, srcUnzip, destZip)) {
        return false;
    }
    int32_t copiedCount = 0;
    int32_t skippedCount = 0;
    bool hasError = !CopyZipEntriesExcluding(srcUnzip, destZip, excludedPaths, copiedCount, skippedCount);
    // Inter-app dedup: inject dedupfiles.json at the package root for matched modules
    hasError = !InjectInterAppResult(destZip, moduleId) || hasError;
    srcUnzip.Close();
    destZip.Close();
    if (hasError) {
        CleanupIncompleteRepack(targetZip);
        SetError("Failed to repack module: error during copying entries");
        return false;
    }
    LOG(DEBUG) << "Repacking completed successfully: copied=" << copiedCount
              << ", skipped=" << skippedCount;
    return true;
}

bool SODeduplicator::InjectInterAppResult(ZipWrapper& destZip, const std::string& moduleId)
{
    auto jsonIt = interAppResultJsonMap_.find(moduleId);
    if (jsonIt == interAppResultJsonMap_.end() || jsonIt->second.empty()) {
        return true;
    }
    if (destZip.WriteStringToZip(jsonIt->second, InterAppDeduplicator::RESULT_FILE_NAME) !=
        ZipErrCode::ZIP_ERR_SUCCESS) {
        LOG(ERROR) << "Failed to inject " << InterAppDeduplicator::RESULT_FILE_NAME
                   << " for module " << moduleId;
        return false;
    }
    return true;
}

void SODeduplicator::CleanupIncompleteRepack(const std::string& targetZip)
{
    LOG(ERROR) << "Repacking failed with errors, cleaning up incomplete file";
    try {
        std::filesystem::remove(targetZip);
    } catch (const std::exception& e) {
        LOG(WARNING) << "Failed to remove incomplete file: " << e.what();
    }
}

bool SODeduplicator::BuildInterAppPackContexts(const std::vector<ModuleDedupContext>& modules,
    InterAppGateContext& appGateContext, std::vector<InterAppPackContext>& packContexts,
    std::vector<const ModuleDedupContext*>& participatingModules)
{
    for (const auto& module : modules) {
        // FA models or modules whose configuration failed to parse do not participate
        if (!module.module) {
            continue;
        }
        // Reuse the Stage module.json already parsed once during module extraction
        InterAppPackContext packContext;
        if (!InterAppDeduplicator::BuildPackContext(module.module, packContext)) {
            SetInterAppError("Failed to build inter-app dedup context for module: " + module.reportPath);
            return false;
        }
        if (!packContext.applicable) {
            continue;
        }
        InterAppDedupSolver::MergeGateContext(appGateContext, packContext.gateContext);
        packContexts.push_back(std::move(packContext));
        participatingModules.push_back(&module);
    }
    return true;
}

bool SODeduplicator::SolveInterAppModule(const ModuleDedupContext& module,
    const InterAppPackContext& packContext, const DedupListSdkParser& dedupList, size_t& matchedModules)
{
    // A hap/hsp that already carries the dedup result file was deduplicated before: keep it as is
    if (ZipUtils::IsFileExistsInZip(module.sourcePath, InterAppDeduplicator::RESULT_FILE_NAME)) {
        LOG(INFO) << "So deduplication for sdk skipped for module " << module.sourcePath
                  << ": " << InterAppDeduplicator::RESULT_FILE_NAME << " already exists";
        return true;
    }
    static const std::map<std::string, std::string> emptyCache;
    auto cacheIt = soSha256Cache_.find(module.moduleId);
    const std::map<std::string, std::string>& moduleCache =
        (cacheIt == soSha256Cache_.end()) ? emptyCache : cacheIt->second;
    // Paths already removed by the intra-app stage are skipped by the inter-app stage
    std::set<std::string> removedByIntraApp;
    auto removedIt = dedupPlan_.removedSoMap.find(module.moduleId);
    if (removedIt != dedupPlan_.removedSoMap.end()) {
        for (const auto& soPath : removedIt->second) {
            removedByIntraApp.insert(NormalizeSoPath(soPath));
        }
    }
    InterAppDeduplicator deduplicator;
    InterAppDedupStageResult removal = deduplicator.ComputeRemovals(dedupList,
        (module.extractedPath / "libs").string(), packContext, moduleCache, removedByIntraApp);
    if (removal.failed) {
        SetInterAppError("Inter-app deduplication failed for module: " + module.reportPath + ", " +
            removal.errorMessage);
        return false;
    }
    if (!removal.applied) {
        return true;
    }
    std::vector<std::string>& excluded = interAppRemovedSoMap_[module.moduleId];
    for (const auto& path : removal.excludedPaths) {
        if (std::find(excluded.begin(), excluded.end(), path) == excluded.end()) {
            excluded.push_back(path);
        }
    }
    interAppResultJsonMap_[module.moduleId] = removal.resultJson;
    ++matchedModules;
    return true;
}

bool SODeduplicator::MergeInterAppDedup(const std::vector<ModuleDedupContext>& modules,
    const std::string& interAppDedupListPath)
{
    LOG(INFO) << "SO deduplication for sdk started.";
    // Stage one: build per module contexts and aggregate the application level gates
    std::vector<InterAppPackContext> packContexts;
    std::vector<const ModuleDedupContext*> participatingModules;
    InterAppGateContext appGateContext;
    if (!BuildInterAppPackContexts(modules, appGateContext, packContexts, participatingModules)) {
        return false;
    }

    // Stage two: application level gates (driver / version) decide for the whole package
    DedupListSdkParser dedupList;
    if (!dedupList.ParseFile(interAppDedupListPath)) {
        SetInterAppError("failed to parse deduplist sdk: " + dedupList.GetErrorMessage());
        return false;
    }
    InterAppDedupSolver solver;
    std::string skipReason;
    if (!solver.CheckGates(appGateContext, dedupList, skipReason)) {
        LOG(INFO) << "So deduplication for sdk skipped: " << skipReason;
        return true;
    }

    // Stage three: each passing module matches compatibility groups with its own compatibleSdkVersion,
    // reusing the intra-app sha256 cache and removal set
    size_t matchedModules = 0;
    for (size_t index = 0; index < packContexts.size(); ++index) {
        if (!SolveInterAppModule(*participatingModules[index], packContexts[index], dedupList, matchedModules)) {
            return false;
        }
    }
    LOG(INFO) << "Inter-app deduplication merged for " << matchedModules << " module(s).";
    return true;
}

bool SODeduplicator::DeduplicateModules(std::list<std::string>& modulePaths,
    bool deduplicateSo, const std::string& workDir, const std::string& reportDir)
{
    return DeduplicateModules(modulePaths, modulePaths, DedupSwitch{deduplicateSo, ""}, workDir, reportDir);
}

bool SODeduplicator::DeduplicateModules(std::list<std::string>& modulePaths,
    const std::list<std::string>& originalModulePaths, bool deduplicateSo,
    const std::string& workDir, const std::string& reportDir)
{
    return DeduplicateModules(modulePaths, originalModulePaths, DedupSwitch{deduplicateSo, ""},
        workDir, reportDir);
}

bool SODeduplicator::DeduplicateModules(std::list<std::string>& modulePaths,
    const std::list<std::string>& originalModulePaths, const DedupSwitch& dedupSwitch,
    const std::string& workDir, const std::string& reportDir)
{
    if (!modulePaths.empty() && modulePaths.size() != originalModulePaths.size()) {
        SetError("Module path count does not match original module path count");
        return false;
    }
    if (!dedupSwitch.deduplicateSo) {
        LOG(INFO) << "SO deduplication skipped: disabled.";
    }
    if (dedupSwitch.interAppDedupListPath.empty()) {
        LOG(INFO) << "SO deduplication for sdk skipped: disabled.";
    }
    if (!dedupSwitch.deduplicateSo && dedupSwitch.interAppDedupListPath.empty()) {
        return true;
    }
    if (modulePaths.empty()) {
        return true;
    }

    try {
        std::filesystem::path root = std::filesystem::path(workDir) /
            ("so_dedup_" + Utils::GenerateUUID());
        std::filesystem::create_directories(root / "modules");
        std::filesystem::create_directories(root / "repacked");
        ModuleSet moduleSet;
        if (!ExtractModuleContexts(modulePaths, originalModulePaths, root, moduleSet)) {
            return false;
        }

        if (!RunDedupStages(moduleSet.modules, dedupSwitch, reportDir)) {
            return false;
        }
        if (!RepackModules(moduleSet.files, modulePaths)) {
            return false;
        }
        // The completed summary describes the intra-app stage only
        if (dedupSwitch.deduplicateSo) {
            LogDedupSummary();
        }
        return true;
    } catch (const std::exception& exception) {
        SetError("Exception during deduplication: " + std::string(exception.what()));
        return false;
    }
}

bool SODeduplicator::ExtractModuleContexts(const std::list<std::string>& modulePaths,
    const std::list<std::string>& originalModulePaths, const std::filesystem::path& root, ModuleSet& moduleSet)
{
    std::filesystem::path modulesRoot = root / "modules";
    std::filesystem::path repackedRoot = root / "repacked";
    auto originalPathIt = originalModulePaths.begin();
    for (const auto& modulePath : modulePaths) {
        if (!ExtractModule(modulePath, *originalPathIt, modulesRoot, repackedRoot, moduleSet)) {
            return false;
        }
        ++originalPathIt;
    }
    return true;
}

bool SODeduplicator::ExtractModule(const std::string& modulePath, const std::string& originalPath,
    const std::filesystem::path& modulesRoot, const std::filesystem::path& repackedRoot, ModuleSet& moduleSet)
{
    std::filesystem::path source(modulePath);
    std::string sourcePathStr = source.string();
    std::string moduleId = std::to_string(moduleSet.modules.size());
    std::filesystem::path extractDir = modulesRoot / std::filesystem::path(moduleId);
    std::filesystem::create_directories(extractDir);
    if (ZipUtils::Unzip(sourcePathStr, extractDir.string()) != ZIP_ERR_SUCCESS) {
        SetError("Failed to unzip module: " + sourcePathStr);
        return false;
    }
    std::filesystem::path jsonPath = extractDir / "module.json";
    bool stageModel = std::filesystem::exists(jsonPath);
    if (!stageModel) {
        jsonPath = extractDir / "config.json";
    }
    auto module = std::make_shared<ModuleJson>();
    if (!std::filesystem::exists(jsonPath) || !module->ParseFromFile(jsonPath.string())) {
        LOG(WARNING) << "SO deduplication skipped for module: failed to parse configuration, module="
                     << sourcePathStr;
        module.reset();
    }
    if (!stageModel) {
        module.reset();
    }
    ModuleConfig config;
    std::map<std::string, std::string> resourceMap;
    if (module) {
        if (!ZipUtils::GetResourceMapFromZip(sourcePathStr, resourceMap)) {
            SetError("Failed to read profile resources from module: " + sourcePathStr);
            return false;
        }
        config = ModuleCalculator::ExtractModuleConfig(module, stageModel, resourceMap);
        if (!config.distributionFilterValid) {
            SetError("Failed to parse distributionFilter from module: " + sourcePathStr);
            return false;
        }
    }
    moduleSet.modules.push_back({module, config, moduleId, extractDir, sourcePathStr, originalPath});
    std::filesystem::path repackedPath = repackedRoot / std::filesystem::path(moduleId) / source.filename();
    std::filesystem::create_directories(repackedPath.parent_path());
    moduleSet.files.push_back({sourcePathStr, repackedPath, moduleId});
    return true;
}

bool SODeduplicator::RunDedupStages(const std::vector<ModuleDedupContext>& modules,
    const DedupSwitch& dedupSwitch, const std::string& reportDir)
{
    // Stage one: intra-app deduplication runs only when its switch is on
    if (dedupSwitch.deduplicateSo) {
        std::string effectiveReportDir = reportDir.empty() ? "." : reportDir;
        if (!ExecuteDeduplication(modules, effectiveReportDir, !dedupSwitch.interAppDedupListPath.empty())) {
            return false;
        }
    } else {
        ResetIntraAppState();
    }
    // Stage two: inter-app dedup (merged with stage one results at repack, each .so is removed once)
    interAppRemovedSoMap_.clear();
    interAppResultJsonMap_.clear();
    if (!dedupSwitch.interAppDedupListPath.empty() &&
        !MergeInterAppDedup(modules, dedupSwitch.interAppDedupListPath)) {
        return false;
    }
    return true;
}

bool SODeduplicator::RepackModules(const std::vector<ModuleFile>& moduleFiles,
    std::list<std::string>& modulePaths)
{
    std::list<std::string> repackedPaths;
    for (const auto& [sourcePath, repackedPath, moduleId] : moduleFiles) {
        // Directly copy ZIP entries, excluding removed SOs
        if (!RepackModuleExcludingSOs(sourcePath, repackedPath.string(), moduleId, dedupPlan_)) {
            SetError("Failed to repack module: " + repackedPath.string());
            return false;
        }
        repackedPaths.push_back(repackedPath.string());
    }
    modulePaths.swap(repackedPaths);
    return true;
}

void SODeduplicator::ResetIntraAppState()
{
    dedupPlan_ = DedupPlan();
    soSha256Cache_.clear();
    dedupSuccess_ = true;
    errorMessage_ = "";
    interAppFailure_ = false;
}

void SODeduplicator::LogDedupSummary()
{
    size_t removedCount = 0;
    for (const auto& [moduleName, paths] : dedupPlan_.removedSoMap) {
        removedCount += paths.size();
    }
    LOG(WARNING) << "SO deduplication completed: removed=" << removedCount
                 << ", savedBytes=" << dedupPlan_.totalSavedSize << ", report=" << reportFilePath_;
}

bool SODeduplicator::ExecuteDeduplication(
    const std::vector<ModuleDedupContext>& allModules,
    const std::string& reportOutputPath,
    bool collectSha256)
{
    LOG(INFO) << "SO deduplication started.";
    LOG(DEBUG) << "Total modules: " << allModules.size();

    // 重置状态
    dedupSuccess_ = false;
    errorMessage_ = "";
    interAppFailure_ = false;
    dedupPlan_ = DedupPlan();
    soSha256Cache_.clear();

    if (allModules.empty()) {
        SetError("No modules provided for deduplication");
        return false;
    }

    try {
        // Step 1: Filter entry modules
        std::vector<const ModuleDedupContext*> entryModules = FilterEntryModules(allModules);

        // Step 2: Calculate device set
        DeviceCalculator deviceCalculator;
        std::vector<ModuleConfig> entryModuleConfigs;
        for (const auto* context : entryModules) {
            entryModuleConfigs.push_back(context->config);
        }
        std::vector<DeviceInstance> devices = deviceCalculator.CalculateDevices(entryModuleConfigs);

        if (devices.empty()) {
            LOG(INFO) << "SO deduplication skipped: no valid entry devices.";
            ReportGenerator reportGenerator;
            reportFilePath_ = reportGenerator.GenerateReport(
                dedupPlan_, DedupStrategy::NONE, reportOutputPath);
            dedupSuccess_ = !reportFilePath_.empty();
            return dedupSuccess_;
        }

        // Step 3: Filter modules for deduplication
        std::vector<const ModuleDedupContext*> eligibleModules = FilterDedupEligibleModules(allModules);
        if (!errorMessage_.empty()) {
            return false;
        }

        if (eligibleModules.empty()) {
            LOG(DEBUG) << "No modules eligible for deduplication";
            ReportGenerator reportGenerator;
            reportFilePath_ = reportGenerator.GenerateReport(dedupPlan_, DedupStrategy::NONE, reportOutputPath);
            if (reportFilePath_.empty()) {
                SetError("Failed to generate deduplication report");
                return false;
            }
            dedupSuccess_ = true;
            return true;
        }

        // Step 4: Calculate mandatory module set
        std::map<DeviceInstance, std::vector<std::string>> mandatoryModuleMap;
        for (const auto& device : devices) {
            auto& mandatoryModules = mandatoryModuleMap[device];
            for (const auto* context : eligibleModules) {
                if (ModuleCalculator::IsMandatoryModule(context->config, device)) {
                    mandatoryModules.push_back(context->moduleId);
                }
            }
        }
        std::map<std::string, std::vector<DeviceInstance>> moduleSupportMap;
        for (const auto* context : eligibleModules) {
            const ModuleConfig& config = context->config;
            for (const auto& device : devices) {
                bool supportsType = std::find(config.deviceTypes.begin(), config.deviceTypes.end(), device.type) !=
                    config.deviceTypes.end();
                bool supportsFilter = config.distributionFilter.empty() ||
                    config.distributionFilter == device.distributionFilter;
                if (supportsType && supportsFilter) {
                    moduleSupportMap[context->moduleId].push_back(device);
                }
            }
        }

        // Step 5: Collect SO file information
        std::map<std::string, std::vector<SoInfo>> moduleSoMap = CollectSoFiles(eligibleModules, collectSha256);
        if (!errorMessage_.empty()) {
            return false;
        }

        // Fill the so sha256 cache for the inter-app stage (module id -> package relative path -> sha256);
        // SHA-256 is only computed when that stage will run
        soSha256Cache_.clear();
        if (collectSha256) {
            for (const auto& [moduleId, soList] : moduleSoMap) {
                std::map<std::string, std::string>& moduleCache = soSha256Cache_[moduleId];
                for (const auto& soInfo : soList) {
                    moduleCache[soInfo.relativePath] = soInfo.sha256;
                }
            }
        }

        if (moduleSoMap.empty()) {
            LOG(DEBUG) << "No SO files found for deduplication";
            ReportGenerator reportGenerator;
            reportFilePath_ = reportGenerator.GenerateReport(dedupPlan_, DedupStrategy::NONE, reportOutputPath);
            if (reportFilePath_.empty()) {
                SetError("Failed to generate deduplication report");
                return false;
            }
            dedupSuccess_ = true;
            return true;
        }

        // Step 6: Group duplicate SOs
        std::vector<DuplicateSoGroup> duplicateGroups = GroupDuplicateSos(moduleSoMap);

        if (duplicateGroups.empty()) {
            LOG(DEBUG) << "No duplicate SO groups found";
            ReportGenerator reportGenerator;
            reportFilePath_ = reportGenerator.GenerateReport(dedupPlan_, DedupStrategy::NONE, reportOutputPath);
            if (reportFilePath_.empty()) {
                SetError("Failed to generate deduplication report");
                return false;
            }
            dedupSuccess_ = true;
            return true;
        }

        // Step 7: Select algorithm based on SO duplicate count and solve
        std::vector<DuplicateSoGroup> smallGroups;  // Groups with <=20 duplicates
        std::vector<DuplicateSoGroup> largeGroups;  // Groups with >20 duplicates

        for (const auto& group : duplicateGroups) {
            if (ExactSolver::CanUseExactAlgorithm(group.soList.size())) {
                smallGroups.push_back(group);
            } else if (GreedySolver::CanUseGreedyAlgorithm(group.soList.size())) {
                largeGroups.push_back(group);
            }
        }

        LOG(DEBUG) << "Using EXACT algorithm for " << smallGroups.size() << " small groups (size <= 20)";
        LOG(DEBUG) << "Using GREEDY algorithm for " << largeGroups.size() << " large groups (size > 20)";

        // Use exact algorithm for small groups
        DedupPlan smallGroupsPlan;
        if (!smallGroups.empty()) {
            ExactSolver exactSolver;
            smallGroupsPlan = exactSolver.Solve(smallGroups, mandatoryModuleMap, moduleSupportMap);
        }

        // Use greedy algorithm for large groups
        DedupPlan largeGroupsPlan;
        if (!largeGroups.empty()) {
            GreedySolver greedySolver;
            largeGroupsPlan = greedySolver.Solve(largeGroups, mandatoryModuleMap, moduleSupportMap);
        }

        // Merge results
        dedupPlan_.Merge(smallGroupsPlan);
        dedupPlan_.Merge(largeGroupsPlan);
        for (const auto& context : allModules) {
            dedupPlan_.moduleNames[context.moduleId] = context.config.moduleName;
            dedupPlan_.modulePaths[context.moduleId] = context.reportPath;
        }

        if (dedupPlan_.keptSoMap.empty() && dedupPlan_.removedSoMap.empty()) {
            SetError("Failed to generate deduplication plan");
            return false;
        }

        // Step 8: Apply deduplication plan
        if (!ApplyDedupPlan(dedupPlan_, allModules)) {
            return false;
        }

        // Step 9: Generate deduplication report
        ReportGenerator reportGenerator;
        DedupStrategy actualStrategy = ResolveDedupStrategy(!smallGroups.empty(), !largeGroups.empty());
        std::string generatedReportPath = reportGenerator.GenerateReport(dedupPlan_, actualStrategy, reportOutputPath);
        if (generatedReportPath.empty()) {
            SetError("Failed to generate deduplication report");
            return false;
        }

        reportFilePath_ = generatedReportPath;
        dedupSuccess_ = true;
        return true;
    } catch (const std::exception& e) {
        SetError("Exception during deduplication: " + std::string(e.what()));
        return false;
    }
}

}  // namespace AppPackingTool
}  // namespace OHOS
