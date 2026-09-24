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

#include "installer_packager.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <limits>
#include <set>
#include <stdexcept>
#include "hap_verify_utils.h"
#include "log.h"
#include "utils.h"
#include "module_json_utils.h"
#include "packing_tool_err_msg.h"
#include "zip_wrapper.h"
#include <unistd.h>

namespace OHOS {
namespace AppPackingTool {
using packing_tool::error::PackingToolErrMsg;
namespace {
using Json = InstallerManifest::Json;
using Manifest = InstallerManifest;
constexpr ZipLevel COMPRESSION_LEVEL = ZipLevel::ZIP_LEVEL_5;
constexpr size_t IO_BUFFER_SIZE = 8192;
constexpr char APP_SUFFIX[] = ".app";

void ValidatePathComponent(const std::string& name)
{
    Manifest::Require(!name.empty() && name != "." && name != ".." &&
        name.find_first_of("/\\:") == std::string::npos && name.find('\0') == std::string::npos,
        PackingToolErrMsg::INSTALLER_PATH_INVALID,
        "Invalid archive path component: " + name);
    Json(name).dump();
}

std::vector<fs::path> ListChildren(const fs::path& directory)
{
    std::vector<fs::path> result;
    for (const auto& entry : fs::directory_iterator(directory)) {
        result.push_back(entry.path());
    }
    Manifest::Require(!result.empty(),
        PackingToolErrMsg::INSTALLER_CONTENTS_INVALID, "Empty directory: " + directory.u8string());
    std::sort(result.begin(), result.end(), [](const fs::path& left, const fs::path& right) {
        return left.filename().u8string() < right.filename().u8string();
    });
    return result;
}

class TempDirGuard final {
public:
    TempDirGuard(const fs::path& directory, const fs::path& file) : directory_(directory), file_(file) {}

    ~TempDirGuard()
    {
        try {
            std::error_code error;
            fs::remove(file_, error);
            if (error) {
                LOGE("%s", PackingToolErrMsg::INSTALLER_CLEANUP_FAILED.toStringWithArgs(
                    "Cannot clean temporary output: " + file_.u8string()).c_str());
                LOGE("[installer] Details: error=%d", error.value());
            }
            fs::remove(directory_, error);
            if (error) {
                LOGE("%s", PackingToolErrMsg::INSTALLER_CLEANUP_FAILED.toStringWithArgs(
                    "Cannot clean temporary directory: " + directory_.u8string()).c_str());
                LOGE("[installer] Details: error=%d", error.value());
            }
        } catch (...) {
            LOGE("%s", PackingToolErrMsg::INSTALLER_CLEANUP_FAILED.toStringWithArgs(
                "Unexpected error while cleaning temporary output").c_str());
        }
    }

    TempDirGuard(const TempDirGuard&) = delete;
    TempDirGuard& operator=(const TempDirGuard&) = delete;

private:
    const fs::path& directory_;
    const fs::path& file_;
};

std::string ReadFile(const fs::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    Manifest::Require(stream.is_open(), PackingToolErrMsg::INSTALLER_IO_FAILED,
        "Cannot read manifest: " + path.u8string());
    std::string result;
    std::array<char, IO_BUFFER_SIZE> buffer{};
    while (stream.read(buffer.data(), buffer.size()) || stream.gcount() > 0) {
        result.append(buffer.data(), static_cast<size_t>(stream.gcount()));
    }
    Manifest::Require(stream.eof() && !stream.bad(),
        PackingToolErrMsg::INSTALLER_IO_FAILED, "Cannot read manifest: " + path.u8string());
    stream.clear();
    stream.close();
    Manifest::Require(!stream.fail(), PackingToolErrMsg::INSTALLER_IO_FAILED,
        "Cannot read manifest: " + path.u8string());
    return result;
}
}

InstallerPackager::InstallerPackager(const std::map<std::string, std::string>& parameters, std::string& result)
    : Packager(parameters, result)
{}

InstallerPackager::Input::Input(const fs::path& source, const std::string& name) : path(source), entry(name)
{
    Manifest::Require(fs::is_regular_file(path),
        PackingToolErrMsg::INSTALLER_IO_FAILED, "Input must be a regular file: " + path.u8string());
}

int32_t InstallerPackager::Fail(const std::exception& exception)
{
    LOGE("%s", PackingToolErrMsg::INSTALLER_IO_FAILED.toStringWithArgs(
        "Cannot access installer files").c_str());
    LOGE("[installer] Details: %s", exception.what());
    return ERR_INVALID_VALUE;
}

int32_t InstallerPackager::Fail(const Manifest::Failure& exception)
{
    LOGE("%s", exception.GetError().toStringWithArgs(exception.what()).c_str());
    if (!exception.GetDetails().empty()) {
        LOGE("[installer] Details: %s", exception.GetDetails().c_str());
    }
    return ERR_INVALID_VALUE;
}

int32_t InstallerPackager::InitAllowedParam()
{
    try {
        for (const std::string& required : {"mode", "contents", "out-path"}) {
            Manifest::Require(parameterMap_.count(required) != 0 && !parameterMap_.at(required).empty(),
                PackingToolErrMsg::INSTALLER_ARGUMENT_INVALID,
                "--" + required + " is required");
        }
        Manifest::Require(parameterMap_.at("mode") == "installer",
            PackingToolErrMsg::INSTALLER_ARGUMENT_INVALID, "--mode must be installer");
        return ERR_OK;
    } catch (const Manifest::Failure& exception) {
        return Fail(exception);
    } catch (const std::exception& exception) {
        return Fail(exception);
    }
}

void InstallerPackager::ValidateArguments()
{
    const auto& out = parameterMap_.at("out-path");
    Manifest::Require(Utils::EndsWith(out, APP_SUFFIX), PackingToolErrMsg::INSTALLER_ARGUMENT_INVALID,
        "--out-path must end with lowercase .app");
    auto force = parameterMap_.find("force");
    force_ = force != parameterMap_.end() && force->second == "true";
    contents_ = fs::absolute(fs::u8path(parameterMap_.at("contents")));
    output_ = fs::absolute(fs::u8path(out));
    contents_ = fs::canonical(contents_);
    output_ = fs::weakly_canonical(output_);
    Manifest::Require(fs::is_directory(contents_),
        PackingToolErrMsg::INSTALLER_CONTENTS_INVALID, "--contents must be a directory");
    for (fs::path ancestor = output_; !ancestor.empty();) {
        Manifest::Require(!fs::exists(ancestor) || !fs::equivalent(ancestor, contents_),
            PackingToolErrMsg::INSTALLER_PATH_INVALID,
            "Output must be outside contents");
        fs::path parent = ancestor.parent_path();
        if (parent == ancestor) {
            break;
        }
        ancestor = parent;
    }
    if (parameterMap_.count("manifest") != 0) {
        Manifest::Require(!parameterMap_.at("manifest").empty(), PackingToolErrMsg::INSTALLER_ARGUMENT_INVALID,
            "--manifest must be a file");
        external_ = fs::absolute(fs::u8path(parameterMap_.at("manifest")));
        Manifest::Require(fs::is_regular_file(external_),
            PackingToolErrMsg::INSTALLER_IO_FAILED, "Input must be a regular file: " + external_.u8string());
        Manifest::Require(output_ != external_ &&
            (!fs::exists(output_) || !fs::equivalent(output_, external_)), PackingToolErrMsg::INSTALLER_PATH_INVALID,
            "Output aliases manifest");
    }
    if (fs::exists(output_)) {
        Manifest::Require(fs::is_regular_file(output_) && force_,
            PackingToolErrMsg::INSTALLER_ARGUMENT_INVALID, "Output exists; use --force true for a regular file");
    }
}

HapVerifyInfo InstallerPackager::ReadModule(const Input& input)
try {
    HapVerifyInfo info;
    Manifest::Require(ModuleJsonUtils::GetStageHapVerifyInfo(input.path.string(), info),
        PackingToolErrMsg::INSTALLER_MODULE_INVALID,
        "Invalid installer module: " + input.path.u8string());
    Manifest::Require(info.GetBundleType() == "app",
        PackingToolErrMsg::INSTALLER_MODULE_INVALID, "bundleType must be app: " + input.path.u8string());
    const auto& types = info.GetDeviceTypes();
    Manifest::Require(std::find(types.begin(), types.end(), "2in1") != types.end(),
        PackingToolErrMsg::INSTALLER_MODULE_INVALID, "deviceTypes must contain 2in1: " + input.path.u8string());
    return info;
}
catch (const Manifest::Failure&) {
    throw;
}
catch (const std::exception& exception) {
    throw Manifest::Failure(PackingToolErrMsg::INSTALLER_MODULE_INVALID,
        "Invalid installer module: " + input.path.u8string(), exception.what());
}

void InstallerPackager::Discover()
{
    for (const auto& directory : ListChildren(contents_)) {
        Manifest::Require(fs::is_directory(directory), PackingToolErrMsg::INSTALLER_CONTENTS_INVALID,
            "Contents may only contain application directories: " + directory.u8string());
        std::string name = directory.filename().u8string();
        ValidatePathComponent(name);
        std::list<HapVerifyInfo> infos;
        int32_t minApi = std::numeric_limits<int32_t>::min();
        for (const auto& path : ListChildren(directory)) {
            std::string filename = path.filename().u8string();
            ValidatePathComponent(filename);
            Manifest::Require(path.extension() == ".hap" || path.extension() == ".hsp",
                PackingToolErrMsg::INSTALLER_CONTENTS_INVALID,
                "Application directories only accept lowercase .hap/.hsp: " + path.u8string());
            Input input(path, "Contents/" + name + "/" + filename);
            Manifest::Require(!fs::exists(output_) || !fs::equivalent(path, output_),
                PackingToolErrMsg::INSTALLER_PATH_INVALID, "Output aliases an input module");
            HapVerifyInfo info = ReadModule(input);
            minApi = std::max(minApi, info.GetApiVersion().compatibleApiVersion);
            infos.push_back(info);
            modules_.push_back(input);
        }
        Manifest::Require(HapVerifyUtils::CheckInstallerAppFields(infos), PackingToolErrMsg::INSTALLER_APP_INVALID,
            "Application module fields are inconsistent: " + directory.u8string());
        const auto& first = infos.front();
        apps_.push_back(Json{{"bundleName", first.GetBundleName()}, {"versionCode", first.GetVersion().versionCode},
            {"versionName", first.GetVersion().versionName}, {"minAPIVersion", minApi}, {"path", "Contents/" + name}});
    }
}

int32_t InstallerPackager::PreProcess()
{
    modules_.clear();
    apps_.clear();
    external_.clear();
    if (!CheckForceFlag()) {
        return ERR_INVALID_VALUE;
    }
    try {
        ValidateArguments();
        Discover();
        if (!external_.empty()) {
            manifest_ = ReadFile(external_);
            Manifest::Validate(manifest_, apps_);
        } else {
            manifest_ = Manifest::Generate(apps_);
        }
        return ERR_OK;
    } catch (const Manifest::Failure& exception) {
        return Fail(exception);
    } catch (const std::exception& exception) {
        return Fail(exception);
    }
}

void InstallerPackager::Write()
try {
    fs::create_directories(output_.parent_path());
    const fs::path temporaryDirectory = output_.parent_path() / (".installer-" + Utils::GenerateUUID());
    const fs::path temporaryFile = temporaryDirectory / "output.tmp";
    std::error_code error;
    if (!fs::create_directory(temporaryDirectory, error)) {
        throw Manifest::Failure(PackingToolErrMsg::INSTALLER_WRITE_FAILED,
            "Cannot create temporary output directory: " + temporaryDirectory.u8string());
    }
    TempDirGuard temporaryGuard(temporaryDirectory, temporaryFile);
    ZipWrapper writer(temporaryFile.string());
    writer.SetZipLevel(COMPRESSION_LEVEL);
    writer.SetUtf8FileNames(true);
    Manifest::Require(writer.Open() == ZIP_ERR_SUCCESS,
        PackingToolErrMsg::INSTALLER_WRITE_FAILED, "Cannot open temporary output");
    Manifest::Require(writer.WriteStringToZip(manifest_, "manifest.json", true) == ZIP_ERR_SUCCESS,
        PackingToolErrMsg::INSTALLER_WRITE_FAILED,
        "Cannot write manifest.json");
    for (const auto& input : modules_) {
        Manifest::Require(writer.AddFileOrDirectoryToZip(input.path, fs::u8path(input.entry),
            true, COMPRESSION_LEVEL) == ZIP_ERR_SUCCESS,
            PackingToolErrMsg::INSTALLER_WRITE_FAILED, "Cannot write module: " + input.path.u8string());
    }
    writer.Close();
    if (force_) {
        Manifest::Require(rename(temporaryFile.c_str(), output_.c_str()) == 0,
            PackingToolErrMsg::INSTALLER_COMMIT_FAILED, "Cannot commit output: " + output_.u8string());
    } else {
        Manifest::Require(link(temporaryFile.c_str(), output_.c_str()) == 0,
            PackingToolErrMsg::INSTALLER_COMMIT_FAILED, "Cannot commit output: " + output_.u8string());
    }
}
catch (const Manifest::Failure&) {
    throw;
}
catch (const std::exception& exception) {
    throw Manifest::Failure(PackingToolErrMsg::INSTALLER_WRITE_FAILED,
        "Cannot write installer archive: " + output_.u8string(), exception.what());
}

int32_t InstallerPackager::Process()
{
    try {
        Write();
        return ERR_OK;
    } catch (const Manifest::Failure& exception) {
        return Fail(exception);
    } catch (const std::exception& exception) {
        return Fail(exception);
    }
}

int32_t InstallerPackager::PostProcess()
{
    return ERR_OK;
}
}
}
