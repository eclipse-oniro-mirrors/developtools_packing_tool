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

#include "installer_manifest.h"

#include <cmath>
#include <set>
#include <stdexcept>

namespace OHOS {
namespace AppPackingTool {
void InstallerManifest::Require(bool condition, const std::string& message)
{
    Require(condition, packing_tool::error::PackingToolErrMsg::INSTALLER_MANIFEST_INVALID, message);
}

void InstallerManifest::Require(bool condition, const packing_tool::error::ErrorMsg& error,
    const std::string& message)
{
    if (!condition) {
        throw Failure(error, message);
    }
}

std::string InstallerManifest::ReadRequiredString(const PtJson& object, const std::string& field)
{
    std::string value;
    Require(object.GetString(field.c_str(), &value) == Result::SUCCESS && !value.empty(),
        field + " must be a nonempty string");
    return value;
}

int32_t InstallerManifest::ReadNonNegativeInt32(const PtJson& object, const std::string& field)
{
    double number = 0;
    Require(object.GetDouble(field.c_str(), &number) == Result::SUCCESS && std::isfinite(number) &&
        number >= 0 && number <= INT32_MAX && std::trunc(number) == number,
        field + " must be a nonnegative int32");
    return static_cast<int32_t>(number);
}

std::string InstallerManifest::Generate(const std::vector<Json>& apps)
{
    Json entries = apps;
    Require(!entries.empty(), "Empty application list");
    entries.front()["coreApp"] = true;
    return nlohmann::ordered_json{{"version", 1}, {"distributionType", "developerID"}, {"apps", entries}}.dump();
}

void InstallerManifest::Validate(const std::string& bytes, const std::vector<Json>& apps)
{
    auto manifest = PtJson::Parse(bytes);
    Require(manifest != nullptr, "Cannot parse manifest JSON");
    std::unique_ptr<cJSON, CJsonDeleter> owner(manifest->GetJson());
    Require(manifest->IsObject(), "JSON root must be an object");
    std::unique_ptr<PtJson> entries;
    Require(manifest->GetArray("apps", &entries) == Result::SUCCESS && entries->GetSize() > 0 &&
        static_cast<size_t>(entries->GetSize()) == apps.size(),
        "apps must correspond one-to-one with application directories");
    std::set<std::string> seen;
    int32_t coreCount = 0;
    for (int32_t index = 0; index < entries->GetSize(); ++index) {
        auto app = entries->Get(index);
        Require(app->IsObject(), "apps item must be an object");
        std::string path = ReadRequiredString(*app, "path");
        Require(seen.insert(path).second, "Duplicate application path");
        const Json* match = nullptr;
        for (const auto& candidate : apps) {
            if (candidate.at("path") == path) {
                match = &candidate;
                break;
            }
        }
        Require(match != nullptr, "Unknown application path: " + path);
        Require(ReadRequiredString(*app, "bundleName") == match->at("bundleName") &&
            ReadRequiredString(*app, "versionName") == match->at("versionName") &&
            ReadNonNegativeInt32(*app, "versionCode") == match->at("versionCode").get<int32_t>(),
            "Manifest application identity differs: " + path);
        Require(ReadNonNegativeInt32(*app, "minAPIVersion") >= match->at("minAPIVersion").get<int32_t>(),
            "Manifest minAPIVersion is below a module: " + path);
        if (app->Contains("coreApp")) {
            bool value = false;
            Require(app->GetBool("coreApp", &value) == Result::SUCCESS, "coreApp must be boolean");
            if (value) {
                ++coreCount;
            }
        }
    }
    Require(coreCount == 1, "Exactly one coreApp=true is required");
}
}
}
