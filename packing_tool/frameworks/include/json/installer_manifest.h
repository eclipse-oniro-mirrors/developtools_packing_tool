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

#ifndef DEVELOPTOOLS_PACKING_TOOL_INSTALLER_MANIFEST_H
#define DEVELOPTOOLS_PACKING_TOOL_INSTALLER_MANIFEST_H

#include <cstdint>
#include <string>
#include <stdexcept>
#include "packing_tool_err_msg.h"
#include <vector>
#include "nlohmann/json.hpp"
#include "pt_json.h"

namespace OHOS {
namespace AppPackingTool {
class InstallerManifest final {
public:
    using Json = nlohmann::json;

    class Failure final : public std::runtime_error {
    public:
        Failure(const packing_tool::error::ErrorMsg& error, const std::string& message,
            const std::string& details = "")
            : std::runtime_error(message), error_(error), details_(details) {}
        const packing_tool::error::ErrorMsg& GetError() const { return error_; }
        const std::string& GetDetails() const { return details_; }

    private:
        const packing_tool::error::ErrorMsg& error_;
        const std::string details_;
    };

    struct CJsonDeleter {
        void operator()(cJSON* value) const noexcept
        {
            // Use a direct call to avoid CFI checks on a function pointer deleter.
            cJSON_Delete(value);
        }
    };

    static std::string ReadRequiredString(const PtJson& object, const std::string& field);
    static int32_t ReadNonNegativeInt32(const PtJson& object, const std::string& field);
    static void Require(bool condition, const std::string& message);
    static void Require(bool condition, const packing_tool::error::ErrorMsg& error, const std::string& message);
    static std::string Generate(const std::vector<Json>& apps);
    static void Validate(const std::string& bytes, const std::vector<Json>& apps);
};
}
}
#endif
