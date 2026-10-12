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

#ifndef DEVELOPTOOLS_PACKING_TOOL_INSTALLER_PACKAGER_H
#define DEVELOPTOOLS_PACKING_TOOL_INSTALLER_PACKAGER_H

#include <exception>
#include <memory>
#include "packager.h"
#include "installer_manifest.h"
#include "hap_verify_info.h"

namespace OHOS {
namespace AppPackingTool {
class InstallerPackager final : public Packager {
public:
    InstallerPackager(const std::map<std::string, std::string>& parameters, std::string& result);
    ~InstallerPackager() override = default;
    InstallerPackager(const InstallerPackager&) = delete;
    InstallerPackager& operator=(const InstallerPackager&) = delete;
    int32_t InitAllowedParam() override;
    int32_t PreProcess() override;
    int32_t Process() override;
    int32_t PostProcess() override;

private:
    struct Input {
        fs::path path;
        std::string entry;
        Input(const fs::path& path, const std::string& entry);
    };

    fs::path contents_;
    fs::path output_;
    std::vector<Input> modules_;
    std::vector<InstallerManifest::Json> apps_;
    fs::path external_;
    std::string manifest_;
    bool force_ = false;

    void ValidateArguments();
    void Discover();
    static HapVerifyInfo ReadModule(const Input& input);
    void Write();
    int32_t Fail(const std::exception& exception);
    int32_t Fail(const InstallerManifest::Failure& exception);
};
}
}
#endif
