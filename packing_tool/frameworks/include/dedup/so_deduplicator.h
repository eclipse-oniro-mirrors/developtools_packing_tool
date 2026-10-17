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

#ifndef DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_SO_DEDUPLICATOR_H
#define DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_SO_DEDUPLICATOR_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <memory>
#include <tuple>
#include <filesystem>
#include "json/module_json.h"
#include "dedup/so_info.h"
#include "dedup/device_calculator.h"
#include "dedup/module_calculator.h"
#include "dedup/deduplist_sdk_parser.h"
#include "dedup/inter_app_deduplicator.h"

namespace OHOS {
namespace AppPackingTool {

class UnzipWrapper;
class ZipWrapper;

// 去重算法选择阈值：重复副本数量超过此值时使用贪心算法，否则使用精确算法
constexpr size_t DEDUP_ALGORITHM_THRESHOLD = 20;

// MD5哈希显示相关常量
constexpr size_t MD5_HEX_WIDTH = 2;           // MD5十六进制宽度
constexpr size_t MD5_DISPLAY_LENGTH = 8;      // MD5哈希显示长度（前N个字符）

struct DedupSwitch {
    bool deduplicateSo = false;
    std::string interAppDedupListPath;  // deduplist_sdk.json 路径；为空表示关闭应用间去重
};

/**
 * @brief SO去重器
 *
 * 串联应用内与应用间去重流程，接收IDE去重开关参数，执行去重操作
 */
class SODeduplicator {
public:
    SODeduplicator();
    ~SODeduplicator();

    bool DeduplicateModules(std::list<std::string>& modulePaths,
                            bool deduplicateSo,
                            const std::string& workDir,
                            const std::string& reportDir);

    bool DeduplicateModules(std::list<std::string>& modulePaths,
                            const std::list<std::string>& originalModulePaths,
                            bool deduplicateSo,
                            const std::string& workDir,
                            const std::string& reportDir);

    /**
     * @brief 应用内 / 应用间两阶段去重（统一重新打包）
     *
     * 应用内阶段在 deduplicateSo 开启时运行，应用间阶段在 interAppDedupListPath 非空时运行；
     * 两个排除集合在重新打包时合并，同一个 .so 只删除一次。
     */
    bool DeduplicateModules(std::list<std::string>& modulePaths,
                            const std::list<std::string>& originalModulePaths,
                            const DedupSwitch& dedupSwitch,
                            const std::string& workDir,
                            const std::string& reportDir);

    /**
     * @brief 获取最终的去重方案
     * @return 去重方案
     */
    DedupPlan GetDedupPlan() const { return dedupPlan_; }

    /**
     * @brief 获取去重是否成功
     * @return 是否成功
     */
    bool IsDedupSuccess() const { return dedupSuccess_; }

    /**
     * @brief 获取错误信息
     * @return 错误信息
     */
    std::string GetErrorMessage() const { return errorMessage_; }

    /**
     * @brief 获取最近一次失败是否来自应用间去重阶段
     * @return true 表示错误来自应用间去重阶段，调用方可据此选择对应错误码
     */
    bool IsInterAppFailure() const { return interAppFailure_; }

private:
    struct ModuleDedupContext {
        std::shared_ptr<ModuleJson> module;
        ModuleConfig config;
        std::string moduleId;
        std::filesystem::path extractedPath;
        std::string sourcePath;
        std::string reportPath;
    };

    using ModuleFile = std::tuple<std::string, std::filesystem::path, std::string>;

    struct ModuleSet {
        std::vector<ModuleDedupContext> modules;
        std::vector<ModuleFile> files;
    };

    /**
     * @brief 过滤entry模块
     * @param allModules 所有模块列表
     * @return entry模块列表
     */
    std::vector<const ModuleDedupContext*> FilterEntryModules(
        const std::vector<ModuleDedupContext>& allModules);

    /**
     * @brief 过滤参与去重的模块
     * @param allModules 所有模块列表
     * @return 参与去重的模块列表
     */
    std::vector<const ModuleDedupContext*> FilterDedupEligibleModules(
        const std::vector<ModuleDedupContext>& allModules);

    /**
     * @brief 收集SO文件信息
     * @param modules 模块实例上下文列表
     * @param collectSha256 是否同时计算 SHA-256（仅应用间去重启用时需要）
     * @return SO文件信息列表（按模块实例ID -> SO列表）
     */
    std::map<std::string, std::vector<SoInfo>> CollectSoFiles(
        const std::vector<const ModuleDedupContext*>& modules, bool collectSha256);

    /**
     * @brief 按MD5分组重复的SO
     * @param moduleSoMap 模块SO映射
     * @return 重复SO组列表
     */
    std::vector<DuplicateSoGroup> GroupDuplicateSos(
        const std::map<std::string, std::vector<SoInfo>>& moduleSoMap);

    /**
     * @brief 计算文件的MD5和SHA256值
     * @param filePath 文件路径
     * @param md5 输出的MD5值
     * @param sha256 输出的SHA256值
     * @param collectSha256 是否同时计算SHA256
     * @return 是否成功计算
     */
    bool CalculateFileDigests(const std::string& filePath, std::string& md5, std::string& sha256,
        bool collectSha256);

    /**
     * @brief 应用去重方案到文件系统
     * @param plan 去重方案
     * @param modules 模块实例上下文列表
     * @return 是否成功应用
     */
    bool ApplyDedupPlan(
        const DedupPlan& plan,
        const std::vector<ModuleDedupContext>& modules);

    // 合并应用内与应用间排除集合（同一模块内去重后，每个 .so 只删除一次）
    std::set<std::string> CollectExcludedSoPaths(const std::string& moduleId, const DedupPlan& plan) const;

    /**
     * @brief 重新打包模块，排除指定的SO文件
     * @return 是否成功
     */
    bool RepackModuleExcludingSOs(
        const std::string& sourceZip,
        const std::string& targetZip,
        const std::string& moduleId,
        const DedupPlan& plan);

    // 打开源/目标 ZIP 并定位到首个条目
    bool OpenRepackZips(const std::string& sourceZip, const std::string& targetZip,
        UnzipWrapper& srcUnzip, ZipWrapper& destZip);

    bool RepackModules(const std::vector<ModuleFile>& moduleFiles, std::list<std::string>& modulePaths);

    // 将应用间去重结果文件写入目标包；无结果内容时直接成功
    bool InjectInterAppResult(ZipWrapper& destZip, const std::string& moduleId);

    // 重新打包失败时删除不完整的目标包
    void CleanupIncompleteRepack(const std::string& targetZip);

    // 运行应用内与应用间两个阶段，任一失败即返回 false
    bool RunDedupStages(const std::vector<ModuleDedupContext>& modules,
        const DedupSwitch& dedupSwitch, const std::string& reportDir);

    /**
     * @brief 解压各模块并解析模块配置
     * @param root 本次去重的临时根目录
     * @param moduleSet 输出的模块上下文与待重新打包文件
     * @return 是否成功
     */
    bool ExtractModuleContexts(const std::list<std::string>& modulePaths,
        const std::list<std::string>& originalModulePaths, const std::filesystem::path& root,
        ModuleSet& moduleSet);

    // 解压单个模块并解析其模块配置：modulePath 作为处理路径，originalPath 作为报告路径
    bool ExtractModule(const std::string& modulePath, const std::string& originalPath,
        const std::filesystem::path& modulesRoot, const std::filesystem::path& repackedRoot,
        ModuleSet& moduleSet);

    // 应用内阶段关闭时重置其状态，使重新打包只反映应用间排除结果
    void ResetIntraAppState();

    // 应用内阶段完成后的汇总日志
    void LogDedupSummary();

    bool ExecuteDeduplication(
        const std::vector<ModuleDedupContext>& allModules,
        const std::string& reportOutputPath,
        bool collectSha256);

    /**
     * @brief 对整个包执行应用间去重，并将结果合并到排除集合
     *
     * 应用级门禁（driver / 版本）基于全部模块聚合后统一判定，门禁不通过时整个包跳过应用间去重；
     * 每个通过门禁的模块使用自身的 compatibleSdkVersion 求解，命中的条目记录为模块级排除路径和
     * 重新打包阶段的结果文件内容。
     * @param modules 模块实例上下文列表
     * @param interAppDedupListPath deduplist_sdk.json 路径
     * @return 是否成功（清单无效等错误时返回 false）
     */
    bool MergeInterAppDedup(const std::vector<ModuleDedupContext>& modules,
        const std::string& interAppDedupListPath);

    // 收集参与应用间去重的模块上下文，并聚合应用级门禁输入
    bool BuildInterAppPackContexts(const std::vector<ModuleDedupContext>& modules,
        InterAppGateContext& appGateContext, std::vector<InterAppPackContext>& packContexts,
        std::vector<const ModuleDedupContext*>& participatingModules);

    // 求解单个模块的应用间去重结果并合并到排除集合
    bool SolveInterAppModule(const ModuleDedupContext& module, const InterAppPackContext& packContext,
        const DedupListSdkParser& dedupList, size_t& matchedModules);

    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void SetError(const std::string& error);

    /**
     * @brief 设置应用间去重阶段的错误信息（使用应用间去重专属错误码）
     * @param error 错误信息
     */
    void SetInterAppError(const std::string& error);

    /**
     * @brief 判断文件是否为.so文件（包括版本号文件如.so.1）
     * @param fileName 文件名
     * @return 是否为.so文件
     */
    static bool IsSoFile(const std::string& fileName);

private:
    DedupPlan dedupPlan_;
    bool dedupSuccess_;
    std::string errorMessage_;
    bool interAppFailure_ = false;
    std::string reportFilePath_;
    // 应用间去重结果：模块实例ID -> 待排除的包内相对路径；模块实例ID -> dedupfiles.json 内容
    std::map<std::string, std::vector<std::string>> interAppRemovedSoMap_;
    std::map<std::string, std::string> interAppResultJsonMap_;
    // 应用内阶段 so 的 sha256 缓存：模块实例ID -> 包内相对路径 -> sha256，供应用间阶段复用
    std::map<std::string, std::map<std::string, std::string>> soSha256Cache_;
};

}  // namespace AppPackingTool
}  // namespace OHOS

#endif  // DEVELOPTOOLS_PACKING_TOOL_APT_FRAMEWORKS_INCLUDE_DEDUP_SO_DEDUPLICATOR_H
