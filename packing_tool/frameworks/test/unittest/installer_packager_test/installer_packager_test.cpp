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
#include <fstream>
#include <random>
#include <sstream>
#include "installer_packager.h"
#include "module_json_utils.h"
#include "shell_command.h"
#include "zip_constants.h"
#include "zip_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace AppPackingTool {
namespace {
using Json = InstallerManifest::Json;
constexpr uLong UTF8_FILE_NAME_FLAG = 1u << 11;

Json Module(const std::string& name = "entry", int32_t minApi = 20, bool debug = false)
{
    return Json{{"app", {{"bundleName", "com.example.installer"}, {"bundleType", "app"},
        {"versionCode", 1}, {"versionName", "1.0"}, {"minAPIVersion", minApi}, {"targetAPIVersion", 30},
        {"debug", debug}}}, {"module", {{"name", name}, {"type", "feature"}, {"deviceTypes", {"2in1"}}}}};
}

void WriteText(const fs::path& path, const std::string& content)
{
    std::ofstream stream(path, std::ios::binary);
    ASSERT_TRUE(stream.is_open());
    stream << content;
    stream.close();
    ASSERT_FALSE(stream.fail());
}

std::string ReadText(const fs::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

void WriteZip(const fs::path& path, const std::vector<std::pair<std::string, std::string>>& entries)
{
    zipFile archive = zipOpen64(path.string().c_str(), APPEND_STATUS_CREATE);
    ASSERT_NE(archive, nullptr);
    for (const auto& entry : entries) {
        ASSERT_EQ(zipOpenNewFileInZip(archive, entry.first.c_str(), nullptr, nullptr, 0, nullptr, 0,
            nullptr, 0, 0), ZIP_OK);
        EXPECT_EQ(zipWriteInFileInZip(archive, entry.second.data(), entry.second.size()), ZIP_OK);
        EXPECT_EQ(zipCloseFileInZip(archive), ZIP_OK);
    }
    EXPECT_EQ(zipClose(archive, nullptr), ZIP_OK);
}

std::map<std::string, std::string> ReadZip(const fs::path& path)
{
    std::map<std::string, std::string> entries;
    unzFile archive = unzOpen64(path.string().c_str());
    if (archive == nullptr) {
        ADD_FAILURE() << "Cannot open result";
        return entries;
    }
    int status = unzGoToFirstFile(archive);
    while (status == UNZ_OK) {
        unz_file_info64 info{};
        char name[1024]{};
        EXPECT_EQ(unzGetCurrentFileInfo64(archive, &info, name, sizeof(name), nullptr, 0, nullptr, 0), UNZ_OK);
        EXPECT_EQ(info.compression_method, static_cast<uLong>(Z_DEFLATED));
        EXPECT_NE(info.flag & UTF8_FILE_NAME_FLAG, 0U);
        EXPECT_EQ(unzOpenCurrentFile(archive), UNZ_OK);
        std::string content;
        char buffer[4096];
        int count;
        while ((count = unzReadCurrentFile(archive, buffer, sizeof(buffer))) > 0) {
            content.append(buffer, count);
        }
        EXPECT_EQ(count, 0);
        EXPECT_EQ(unzCloseCurrentFile(archive), UNZ_OK);
        EXPECT_TRUE(entries.emplace(name, content).second);
        status = unzGoToNextFile(archive);
    }
    EXPECT_EQ(status, UNZ_END_OF_LIST_OF_FILE);
    EXPECT_EQ(unzClose(archive), UNZ_OK);
    return entries;
}
}

class InstallerPackagerTest : public testing::Test {
protected:
    fs::path root_;
    fs::path contents_;
    fs::path output_;
    std::map<std::string, std::string> parameters_;
    std::string result_;
    bool owned_ = false;

    void SetUp() override
    {
        root_ = fs::absolute("installer-test-" + std::to_string(std::random_device{}()));
        ASSERT_TRUE(fs::create_directory(root_));
        owned_ = true;
        contents_ = root_ / "payload";
        output_ = root_ / "result.app";
        ASSERT_TRUE(fs::create_directory(contents_));
        parameters_ = {{"mode", "installer"}, {"contents", contents_.string()}, {"out-path", output_.string()}};
        Add("app1", "entry.hap", Module());
    }

    void TearDown() override
    {
        const std::string testDirectoryPrefix = "installer-test-";
        if (owned_ && !root_.empty() && root_.parent_path() == fs::current_path() &&
            root_.filename().string().compare(0, testDirectoryPrefix.size(), testDirectoryPrefix) == 0) {
            std::error_code error;
            fs::remove_all(root_, error);
            EXPECT_FALSE(error);
        }
    }

    void Add(const std::string& app, const std::string& file, const Json& module)
    {
        fs::path appPath = contents_;
        appPath /= fs::u8path(app);
        fs::create_directories(appPath);
        appPath.append(file);
        WriteZip(appPath, {{"module.json", module.dump()}, {"payload.bin", "original bytes"}});
    }

    int32_t Pack()
    {
        InstallerPackager packager(parameters_, result_);
        return packager.MakePackage();
    }

    Json App(const std::string& path = "Contents/app1")
    {
        return Json{{"bundleName", "com.example.installer"}, {"versionCode", 1}, {"versionName", "1.0"},
            {"minAPIVersion", 20}, {"path", path}, {"coreApp", true}};
    }

    void External(const std::string& bytes)
    {
        fs::path manifest = root_ / "manifest.json";
        WriteText(manifest, bytes);
        parameters_["manifest"] = manifest.string();
    }
};


HWTEST_F(InstallerPackagerTest, ReuseAppConfigurationChecks, TestSize.Level2)
{
    for (const auto& field : {"apiReleaseType", "compileSdkType", "targetBundleName"}) {
        Json module = Module("feature");
        module["app"][field] = "different";
        Add("app1", "feature.hap", module);
        EXPECT_NE(Pack(), ERR_OK) << field;
        EXPECT_FALSE(fs::exists(output_));
    }
}

HWTEST_F(InstallerPackagerTest, ReuseAppEntryConflictChecks, TestSize.Level2)
{
    Json entry = Module("entry");
    entry["module"]["type"] = "entry";
    Add("app1", "entry.hap", entry);
    Json other = Module("other");
    other["module"]["type"] = "entry";
    Add("app1", "other.hap", other);
    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_FALSE(fs::exists(output_));
}

HWTEST_F(InstallerPackagerTest, RejectEmptyModuleNames, TestSize.Level2)
{
    Add("app1", "entry.hap", Module(""));
    EXPECT_NE(Pack(), ERR_OK);
    Add("app1", "feature.hap", Module("feature"));
    EXPECT_NE(Pack(), ERR_OK);
    Add("app1", "entry.hap", Module("entry"));
    Add("app1", "feature.hap", Module(""));
    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_FALSE(fs::exists(output_));
}

HWTEST_F(InstallerPackagerTest, MakePackage_001, TestSize.Level1)
{
    const auto original = ReadText(contents_ / "app1/entry.hap");
    ASSERT_EQ(Pack(), ERR_OK);
    auto entries = ReadZip(output_);
    ASSERT_EQ(entries.size(), 2U);
    EXPECT_EQ(entries.at("Contents/app1/entry.hap"), original);
    Json manifest = Json::parse(entries.at("manifest.json"));
    EXPECT_EQ(manifest, (Json{{"version", 1}, {"distributionType", "developer_id"}, {"apps", {App()}}}));
    EXPECT_EQ(ReadText(contents_ / "app1/entry.hap"), original);
}

HWTEST_F(InstallerPackagerTest, MakePackage_002, TestSize.Level1)
{
    fs::remove(contents_ / "app1/entry.hap");
    Add("app1", "first.hsp", Module("first", 20, true));
    Add("app1", "second.hsp", Module("second", 25, false));
    ASSERT_EQ(Pack(), ERR_OK);
    Json manifest = Json::parse(ReadZip(output_).at("manifest.json"));
    EXPECT_EQ(manifest.at("apps").at(0).at("minAPIVersion"), 25);
}

HWTEST_F(InstallerPackagerTest, MakePackage_003, TestSize.Level1)
{
    for (const auto& name : {"a2", "A1", "app10", "app2"}) {
        Add(name, "feature.hap", Module("feature"));
    }
    ASSERT_EQ(Pack(), ERR_OK);
    Json apps = Json::parse(ReadZip(output_).at("manifest.json")).at("apps");
    ASSERT_EQ(apps.size(), 5U);
    EXPECT_EQ(apps.at(0).at("path"), "Contents/A1");
    EXPECT_EQ(apps.at(1).at("path"), "Contents/a2");
    EXPECT_EQ(apps.at(3).at("path"), "Contents/app10");
    for (size_t index = 1; index < apps.size(); ++index) {
        EXPECT_FALSE(apps.at(index).contains("coreApp"));
    }
}

HWTEST_F(InstallerPackagerTest, MakePackage_004, TestSize.Level1)
{
    Json module = Module("shared", 19);
    module["module"]["type"] = "shared";
    module["app"]["vendor"] = "different";
    module["app"]["apiReleaseType"] = "different";
    module["app"]["targetAPIVersion"] = 29;
    module["app"]["minCompatibleVersionCode"] = 0;
    Add("app1", "shared.hsp", module);
    EXPECT_EQ(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_005, TestSize.Level2)
{
    WriteText(output_, "old output");
    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_EQ(ReadText(output_), "old output");
    parameters_["force"] = "true";
    WriteText(contents_ / "app1/unexpected.txt", "input");
    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_EQ(ReadText(output_), "old output");
    fs::remove(contents_ / "app1/unexpected.txt");
    EXPECT_EQ(Pack(), ERR_OK);
    EXPECT_NE(ReadText(output_), "old output");
}

HWTEST_F(InstallerPackagerTest, MakePackage_006, TestSize.Level2)
{
    parameters_["out-path"] = (contents_ / "forbidden.app").string();
    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_FALSE(fs::exists(contents_ / "forbidden.app"));
    parameters_["out-path"] = (root_ / "result.APP").string();
    EXPECT_NE(Pack(), ERR_OK);
    parameters_["out-path"] = output_.string();
    fs::rename(contents_ / "app1/entry.hap", contents_ / "app1/entry.HAP");
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_007, TestSize.Level2)
{
    for (const auto& field : {"bundleName", "versionName", "versionCode", "buildVersion"}) {
        Json module = Module("second");
        module["app"][field] = std::string(field) == "versionCode" ? Json(2) : Json("different");
        Add("app1", "second.hsp", module);
        EXPECT_NE(Pack(), ERR_OK) << field;
        EXPECT_FALSE(fs::exists(output_));
    }
}

HWTEST_F(InstallerPackagerTest, MakePackage_008, TestSize.Level2)
{
    for (const auto& field : {"minAPIVersion", "targetAPIVersion", "minCompatibleVersionCode"}) {
        Json module = Module("second");
        module["app"][field] = 100;
        Add("app1", "second.hsp", module);
        EXPECT_NE(Pack(), ERR_OK) << field;
    }
}

HWTEST_F(InstallerPackagerTest, MakePackage_009, TestSize.Level2)
{
    Add("app1", "shared.hsp", Module("shared", 20, true));
    EXPECT_NE(Pack(), ERR_OK);
    Add("app1", "entry.hap", Module("entry", 20, true));
    EXPECT_EQ(Pack(), ERR_OK);
    fs::remove(output_);
    Add("app1", "feature.hap", Module("feature", 20, false));
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_010, TestSize.Level2)
{
    Add("app1", "duplicate.hsp", Module());
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_011, TestSize.Level2)
{
    Json module = Module();
    module["module"]["deviceTypes"] = {"phone"};
    Add("app1", "entry.hap", module);
    EXPECT_NE(Pack(), ERR_OK);
    module = Module();
    module["app"]["bundleType"] = "shared";
    Add("app1", "entry.hap", module);
    EXPECT_NE(Pack(), ERR_OK);
    WriteZip(contents_ / "app1/entry.hap", {{"config.json", "{}"}});
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_012, TestSize.Level2)
{
    WriteZip(contents_ / "app1/entry.hap", {{"module.json", Module().dump()}, {"module.json", Module().dump()}});
    ASSERT_EQ(Pack(), ERR_OK);
    ASSERT_TRUE(fs::remove(output_));
    WriteText(contents_ / "app1/entry.hap", "not a ZIP");
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_013, TestSize.Level1)
{
    std::string bytes = "\r\n {\"version\":null,\"distributionType\":{},"
        "\"unknown\":[1,true],\"apps\":[" + App().dump() + "]} \n";
    External(bytes);
    ASSERT_EQ(Pack(), ERR_OK);
    EXPECT_EQ(ReadZip(output_).at("manifest.json"), bytes);
}

HWTEST_F(InstallerPackagerTest, MakePackage_014, TestSize.Level2)
{
    for (const auto& field : {"bundleName", "versionCode", "versionName", "minAPIVersion", "path"}) {
        Json app = App();
        app.erase(field);
        External(Json{{"apps", {app}}}.dump());
        EXPECT_NE(Pack(), ERR_OK) << field;
    }
    Json app = App();
    app["coreApp"] = "true";
    External(Json{{"apps", {app}}}.dump());
    EXPECT_NE(Pack(), ERR_OK);
    app["coreApp"] = false;
    External(Json{{"apps", {app}}}.dump());
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, RequireExactlyOneCoreApp, TestSize.Level2)
{
    Add("app2", "entry.hap", Module());
    Json first = App();
    Json second = App("Contents/app2");
    External(Json{{"apps", {first, second}}}.dump());
    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_FALSE(fs::exists(output_));

    first["coreApp"] = false;
    std::string bytes = Json{{"apps", {first, second}}}.dump();
    External(bytes);
    ASSERT_EQ(Pack(), ERR_OK);
    EXPECT_EQ(ReadZip(output_).at("manifest.json"), bytes);
    ASSERT_TRUE(fs::remove(output_));

    first["coreApp"] = true;
    second.erase("coreApp");
    External(Json{{"apps", {first, second}}}.dump());
    ASSERT_EQ(Pack(), ERR_OK);
    ASSERT_TRUE(fs::remove(output_));

    first["coreApp"] = false;
    External(Json{{"apps", {first, second}}}.dump());
    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_FALSE(fs::exists(output_));
}

HWTEST_F(InstallerPackagerTest, MakePackage_015, TestSize.Level2)
{
    for (const auto& path : {"Contents/../app1", "Contents/app1/", "Contents\\app1", "/Contents/app1",
        "contents/app1", "Contents/app1/sub", "C:/app1", "Contents/unknown"}) {
        External(Json{{"apps", {App(path)}}}.dump());
        EXPECT_NE(Pack(), ERR_OK) << path;
    }
    Add("app2", "entry.hap", Module());
    External(Json{{"apps", {App(), App()}}}.dump());
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_016, TestSize.Level2)
{
    for (const Json& value : {Json(-1), Json(2147483648LL), Json(1.5), Json("1"), Json(true), Json(nullptr),
        Json(INT32_MAX)}) {
        Json module = Module();
        module["app"]["versionCode"] = value;
        Add("app1", "entry.hap", module);
        HapVerifyInfo info;
        bool parsed = ModuleJsonUtils::GetStageHapVerifyInfo(
            (contents_ / "app1/entry.hap").string(), info);
        ASSERT_EQ(Pack() == ERR_OK, parsed) << value;
        if (parsed) {
            Json manifest = Json::parse(ReadZip(output_).at("manifest.json"));
            EXPECT_EQ(manifest.at("apps").at(0).at("versionCode"), info.GetVersion().versionCode);
            ASSERT_TRUE(fs::remove(output_));
        } else {
            EXPECT_FALSE(fs::exists(output_));
        }
    }
}

HWTEST_F(InstallerPackagerTest, MakePackage_017, TestSize.Level2)
{
    WriteText(contents_ / "unexpected.txt", "input");
    EXPECT_NE(Pack(), ERR_OK);
    fs::remove(contents_ / "unexpected.txt");
    fs::create_directory(contents_ / "app1/nested");
    EXPECT_NE(Pack(), ERR_OK);
    fs::remove(contents_ / "app1/nested");
    fs::remove(contents_ / "app1/entry.hap");
    EXPECT_NE(Pack(), ERR_OK);
    fs::remove(contents_ / "app1");
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_018, TestSize.Level2)
{
    std::error_code error;
    fs::create_hard_link(contents_ / "app1/entry.hap", output_, error);
    ASSERT_FALSE(error);
    const auto before = ReadText(output_);
    parameters_["force"] = "true";
    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_EQ(ReadText(output_), before);
}

HWTEST_F(InstallerPackagerTest, MakePackage_019, TestSize.Level2)
{
    fs::create_directory_symlink(contents_, root_ / "linked");
    parameters_["contents"] = (root_ / "linked").string();
    ASSERT_EQ(Pack(), ERR_OK);
    ASSERT_TRUE(fs::remove(output_));
    parameters_["contents"] = contents_.string();
    fs::rename(contents_ / "app1/entry.hap", root_ / "entry.hap");
    fs::create_symlink(root_ / "entry.hap", contents_ / "app1/entry.hap");
    EXPECT_EQ(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, Process_002, TestSize.Level2)
{
    InstallerPackager packager(parameters_, result_);
    ASSERT_EQ(packager.InitAllowedParam(), ERR_OK);
    ASSERT_EQ(packager.PreProcess(), ERR_OK);
    WriteText(output_, "concurrent output");
    EXPECT_NE(packager.Process(), ERR_OK);
    EXPECT_EQ(ReadText(output_), "concurrent output");
    for (const auto& entry : fs::directory_iterator(root_)) {
        EXPECT_NE(entry.path().filename().string().compare(0, 11, ".installer-"), 0);
    }
}

HWTEST_F(InstallerPackagerTest, InitAllowedParam_001, TestSize.Level2)
{
    for (const auto& required : {"mode", "contents", "out-path"}) {
        auto parameters = parameters_;
        parameters.erase(required);
        InstallerPackager packager(parameters, result_);
        EXPECT_NE(packager.MakePackage(), ERR_OK);
    }
    parameters_["hap-path"] = "legacy.hap";
    EXPECT_EQ(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_020, TestSize.Level1)
{
    Json module = Module();
    module["module"]["metadata"] = {{{"resource", "$profile:distribution"}}};
    Json filter = {{"distributionFilter", {{"countryCode", {{"policy", "include"}, {"value", {"CN"}}}}}}};
    WriteZip(contents_ / "app1/entry.hap", {{"module.json", module.dump()},
        {"resources/base/profile/distribution.json", filter.dump()}});
    filter["distributionFilter"]["countryCode"]["value"] = {"US"};
    WriteZip(contents_ / "app1/variant.hap", {{"module.json", module.dump()},
        {"resources/base/profile/distribution.json", filter.dump()}});
    EXPECT_EQ(Pack(), ERR_OK);
    fs::remove(output_);
    filter["distributionFilter"]["countryCode"]["value"] = {"CN"};
    WriteZip(contents_ / "app1/variant.hap", {{"module.json", module.dump()},
        {"resources/base/profile/distribution.json", filter.dump()}});
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, ReuseDistroFilterParser_001, TestSize.Level1)
{
    Json module = Module();
    module["module"]["metadata"] = {{{"resource", "$profile:unrelated"}},
        {{"resource", "$profile:distribution"}}};
    for (const std::string& key : {"distributionFilter", "distroFilter"}) {
        Json filter = {{key, {{"countryCode", {{"policy", "include"}, {"value", {"CN"}}}}}}};
        WriteZip(contents_ / "app1/entry.hap", {{"module.json", module.dump()},
            {"resources/base/profile/unrelated.json", "{}"},
            {"resources/base/profile/distribution.json", filter.dump()}});
        filter[key]["countryCode"]["value"] = {"US"};
        WriteZip(contents_ / "app1/variant.hap", {{"module.json", module.dump()},
            {"resources/base/profile/unrelated.json", "{}"},
            {"resources/base/profile/distribution.json", filter.dump()}});
        ASSERT_EQ(Pack(), ERR_OK);
        EXPECT_EQ(ReadZip(output_).count("Contents/app1/variant.hap"), 1U);
        fs::remove(output_);
    }
}

HWTEST_F(InstallerPackagerTest, RejectMissingMetadataResourceAfterFilter, TestSize.Level2)
{
    Json module = Module();
    module["module"]["metadata"] = {{{"resource", "$profile:distribution"}},
        {{"resource", "$profile:missing"}}};
    Json filter = {{"distributionFilter", {{"countryCode", {{"policy", "include"}, {"value", {"CN"}}}}}}};
    WriteZip(contents_ / "app1/entry.hap", {{"module.json", module.dump()},
        {"resources/base/profile/distribution.json", filter.dump()}});
    filter["distributionFilter"]["countryCode"]["value"] = {"US"};
    WriteZip(contents_ / "app1/variant.hap", {{"module.json", module.dump()},
        {"resources/base/profile/distribution.json", filter.dump()}});
    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_FALSE(fs::exists(output_));
}

HWTEST_F(InstallerPackagerTest, ReuseDistroFilterParser_002, TestSize.Level1)
{
    Json module = Module();
    module["module"]["metadata"] = {{{"resource", "$profile:distribution"}}};
    const std::vector<Json> values = {Json::array(), Json::array({""})};
    for (const auto& value : values) {
        Json filter = {{"distributionFilter", {{"countryCode", {{"policy", "include"}, {"value", value}}},
            {"apiVersion", {{"policy", "include"}, {"value", {-1}}}}}}};
        WriteZip(contents_ / "app1/entry.hap", {{"module.json", module.dump()},
            {"resources/base/profile/distribution.json", filter.dump()}});
        filter["distributionFilter"]["countryCode"]["value"] = {"US"};
        WriteZip(contents_ / "app1/variant.hap", {{"module.json", module.dump()},
            {"resources/base/profile/distribution.json", filter.dump()}});
        ASSERT_EQ(Pack(), ERR_OK);
        fs::remove(output_);
    }
}

HWTEST_F(InstallerPackagerTest, ReuseDistroFilterParser_003, TestSize.Level2)
{
    Json module = Module();
    module["module"]["metadata"] = {{{"resource", "$profile:distribution"}}};
    const std::vector<Json> filters = {
        {{"distributionFilter", {{"countryCode", {{"policy", "include"}, {"value", {1}}}}}}},
        {{"distributionFilter", {{"apiVersion", {{"policy", "include"}, {"value", {1.5}}}}}}},
        {{"distributionFilter", "invalid"}}
    };
    for (const auto& filter : filters) {
        for (const std::string& name : {"entry.hap", "variant.hap"}) {
            WriteZip(contents_ / "app1" / name, {{"module.json", module.dump()},
                {"resources/base/profile/distribution.json", filter.dump()}});
        }
        EXPECT_NE(Pack(), ERR_OK);
        EXPECT_FALSE(fs::exists(output_));
    }
}

HWTEST_F(InstallerPackagerTest, ReuseDistroFilterParser_004, TestSize.Level2)
{
    Json module = Module();
    for (const std::string& profile : {"../distribution", "distribution"}) {
        module["module"]["metadata"] = {{{"resource", "$profile:" + profile}}};
        for (const std::string& name : {"entry.hap", "variant.hap"}) {
            WriteZip(contents_ / "app1" / name, {{"module.json", module.dump()},
                {"resources/base/profile/distribution.json", "{\"distributionFilter\":{},}"}});
        }
        EXPECT_NE(Pack(), ERR_OK);
        EXPECT_FALSE(fs::exists(output_));
    }
}

HWTEST_F(InstallerPackagerTest, MakePackage_021, TestSize.Level1)
{
    Json module = Module();
    module["app"].erase("bundleType");
    module["app"].erase("debug");
    module["app"].erase("targetAPIVersion");
    module["module"]["metadata"] = {{{"resource", "$profile:unused"}}};
    WriteZip(contents_ / "app1/entry.hap", {{"module.json", module.dump()},
        {"resources/base/profile/unused.json", "{}"}});
    EXPECT_EQ(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, MakePackage_022, TestSize.Level2)
{
    const fs::path path = contents_ / "app1/entry.hap";
    const auto original = ReadText(path);
    size_t header = original.find(std::string("PK\1\2", 4));
    ASSERT_NE(header, std::string::npos);
    for (size_t offset : {16U, 24U}) {
        std::string damaged = original;
        damaged.at(header + offset) ^= 1;
        WriteText(path, damaged);
        EXPECT_NE(Pack(), ERR_OK);
        EXPECT_FALSE(fs::exists(output_));
    }
}

HWTEST_F(InstallerPackagerTest, MakePackage_023, TestSize.Level1)
{
    std::string bytes = "{\"unknown\":[-1,1.5,1e2,2147483648],\"apps\":[" + App().dump() + "]}";
    External(bytes);
    ASSERT_EQ(Pack(), ERR_OK);
    EXPECT_EQ(ReadZip(output_).at("manifest.json"), bytes);
}

HWTEST_F(InstallerPackagerTest, MakePackage_024, TestSize.Level1)
{
    for (const auto& name : {u8"\ue000", u8"\U00010000"}) {
        Add(name, "entry.hap", Module());
    }
    ASSERT_EQ(Pack(), ERR_OK);
    Json apps = Json::parse(ReadZip(output_).at("manifest.json")).at("apps");
    EXPECT_EQ(apps.at(1).at("path"), u8"Contents/\ue000");
    EXPECT_EQ(apps.at(2).at("path"), u8"Contents/\U00010000");
}

HWTEST_F(InstallerPackagerTest, Utf8FileNames, TestSize.Level1)
{
    Add(u8"中", u8"𠀀.hap", Module());
    Add(u8"𠀀", u8"中.hsp", Module("library"));
    ASSERT_EQ(Pack(), ERR_OK);
    auto entries = ReadZip(output_);
    EXPECT_EQ(entries.at(u8"Contents/中/𠀀.hap"),
        ReadText(contents_ / fs::u8path(u8"中/𠀀.hap")));
    EXPECT_EQ(entries.at(u8"Contents/𠀀/中.hsp"),
        ReadText(contents_ / fs::u8path(u8"𠀀/中.hsp")));
}

HWTEST_F(InstallerPackagerTest, ZipUtf8FileNamesOptIn, TestSize.Level1)
{
    for (bool enabled : {false, true}) {
        const fs::path archivePath = root_ / (enabled ? "utf8.zip" : "legacy.zip");
        ZipWrapper writer(archivePath.string());
        if (enabled) {
            writer.SetUtf8FileNames(true);
        }
        ASSERT_EQ(writer.Open(), ZIP_ERR_SUCCESS);
        ASSERT_EQ(writer.WriteStringToZip("original text", u8"中.json"), ZIP_ERR_SUCCESS);
        ASSERT_EQ(writer.AddFileOrDirectoryToZip(contents_ / "app1", fs::u8path(u8"𠀀")),
            ZIP_ERR_SUCCESS);
        writer.Close();
        unzFile archive = unzOpen64(archivePath.string().c_str());
        ASSERT_NE(archive, nullptr);
        int status = unzGoToFirstFile(archive);
        size_t count = 0;
        while (status == UNZ_OK) {
            unz_file_info64 info{};
            EXPECT_EQ(unzGetCurrentFileInfo64(archive, &info, nullptr, 0, nullptr, 0, nullptr, 0), UNZ_OK);
            if (enabled) {
                EXPECT_EQ(info.flag & UTF8_FILE_NAME_FLAG, UTF8_FILE_NAME_FLAG);
            }
            EXPECT_EQ(info.compression_method, 0U);
            ++count;
            status = unzGoToNextFile(archive);
        }
        EXPECT_EQ(status, UNZ_END_OF_LIST_OF_FILE);
        EXPECT_EQ(count, 2U);
        EXPECT_EQ(unzClose(archive), UNZ_OK);
    }
}

HWTEST_F(InstallerPackagerTest, Process_003, TestSize.Level2)
{
    parameters_["force"] = "true";
    InstallerPackager packager(parameters_, result_);
    ASSERT_EQ(packager.InitAllowedParam(), ERR_OK);
    ASSERT_EQ(packager.PreProcess(), ERR_OK);
    fs::create_directory(output_);
    WriteText(output_ / "sentinel", "preserve");
    EXPECT_NE(packager.Process(), ERR_OK);
    EXPECT_EQ(ReadText(output_ / "sentinel"), "preserve");
}

HWTEST_F(InstallerPackagerTest, MakePackage_025, TestSize.Level2)
{
    for (const auto& field : {"minAPIVersion", "targetAPIVersion", "minCompatibleVersionCode", "name"}) {
        Json module = Module("feature");
        bool moduleConflict = std::string(field) == "name";
        if (moduleConflict) {
            module["module"]["name"] = "entry";
        } else {
            module["app"][field] = 2;
        }
        Add("app1", "feature.hap", module);
        int32_t result = Pack();
        EXPECT_NE(result, ERR_OK);
        EXPECT_FALSE(fs::exists(output_));
    }
}

HWTEST_F(InstallerPackagerTest, MakePackage_026, TestSize.Level1)
{
    for (bool debug : {false, true}) {
        std::string payload;
        for (int index = 0; index < 10000; ++index) {
            payload += std::to_string(index % 137) + "-installer-compression-level-five-";
        }
        WriteZip(contents_ / "app1/entry.hap", {{"module.json", Module("entry", 20, debug).dump()},
            {"payload.bin", payload}});
        parameters_["force"] = "true";
        ASSERT_EQ(Pack(), ERR_OK);
        const std::string original = ReadText(contents_ / "app1/entry.hap");
        EXPECT_EQ(ReadZip(output_).at("Contents/app1/entry.hap"), original);
        z_stream stream{};
        ASSERT_EQ(deflateInit2(&stream, 5, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY), Z_OK);
        std::string expected(deflateBound(&stream, original.size()), '\0');
        stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(original.data()));
        stream.avail_in = original.size();
        stream.next_out = reinterpret_cast<Bytef*>(&expected[0]);
        stream.avail_out = expected.size();
        EXPECT_EQ(deflate(&stream, Z_FINISH), Z_STREAM_END);
        expected.resize(stream.total_out);
        EXPECT_EQ(deflateEnd(&stream), Z_OK);
        unzFile archive = unzOpen64(output_.string().c_str());
        ASSERT_NE(archive, nullptr);
        EXPECT_EQ(unzLocateFile(archive, "Contents/app1/entry.hap", 1), UNZ_OK);
        int method = 0;
        EXPECT_EQ(unzOpenCurrentFile2(archive, &method, nullptr, 1), UNZ_OK);
        std::string compressed;
        char buffer[4096];
        int count;
        while ((count = unzReadCurrentFile(archive, buffer, sizeof(buffer))) > 0) {
            compressed.append(buffer, count);
        }
        EXPECT_EQ(count, 0);
        EXPECT_EQ(unzCloseCurrentFile(archive), UNZ_OK);
        EXPECT_EQ(unzClose(archive), UNZ_OK);
        EXPECT_EQ(method, Z_DEFLATED);
        EXPECT_EQ(compressed, expected);
    }
}

HWTEST(InstallerShellTest, ParseParam_001, TestSize.Level2)
{
    const int savedOptind = optind;
    const std::vector<std::vector<std::string>> cases = {
        {"--mode", "installer", "--manifest"},
        {"--mode", "installer", "--unknown", "value"},
    };
    for (const auto& arguments : cases) {
        std::vector<std::string> storage = {"ohos_packing_tool", "pack"};
        storage.insert(storage.end(), arguments.begin(), arguments.end());
        std::vector<char*> pointers;
        for (auto& item : storage) {
            pointers.push_back(item.data());
        }
        ShellCommand command(pointers.size(), pointers.data(), "ohos_packing_tool");
        optind = 2;
        EXPECT_NE(command.ParseParam(), ERR_OK);
    }
    optind = savedOptind;
}

HWTEST(InstallerShellTest, ParseParam_002, TestSize.Level1)
{
    const int savedOptind = optind;
    for (const auto& arguments : std::vector<std::vector<std::string>>{
        {"--mode", "installer", "--contents", "first", "--contents", "second"},
        {"--mode", "installer", "--compress-level", "9"},
        {"--mode", "app", "--contents", "payload"},
        {"--mode", "hap", "--manifest", "manifest.json"}}) {
        std::vector<std::string> storage = {"ohos_packing_tool", "pack"};
        storage.insert(storage.end(), arguments.begin(), arguments.end());
        std::vector<char*> pointers;
        for (auto& item : storage) {
            pointers.push_back(item.data());
        }
        ShellCommand command(pointers.size(), pointers.data(), "ohos_packing_tool");
        optind = 2;
        EXPECT_EQ(command.ParseParam(), ERR_OK);
    }
    optind = savedOptind;
}

HWTEST_F(InstallerPackagerTest, IgnoreUnrelatedProfileForDistroFilter, TestSize.Level1)
{
    Json module = Module();
    module["module"]["metadata"] = {
        {{"resource", "$profile:other"}}, {{"resource", "$profile:distribution"}}};
    Json filter = {{"distributionFilter", {{"countryCode", {{"policy", "include"}, {"value", {"CN"}}}}}}};
    WriteZip(contents_ / "app1/entry.hap", {{"module.json", module.dump()},
        {"resources/base/profile/other.json", "[]"},
        {"resources/base/profile/distribution.json", filter.dump()}});
    filter["distributionFilter"]["countryCode"]["value"] = {"US"};
    WriteZip(contents_ / "app1/variant.hap", {{"module.json", module.dump()},
        {"resources/base/profile/other.json", "[]"},
        {"resources/base/profile/distribution.json", filter.dump()}});
    EXPECT_EQ(Pack(), ERR_OK);
    fs::remove(output_);
    filter["distributionFilter"]["countryCode"]["value"] = {"CN"};
    WriteZip(contents_ / "app1/variant.hap", {{"module.json", module.dump()},
        {"resources/base/profile/other.json", "[]"},
        {"resources/base/profile/distribution.json", filter.dump()}});
    EXPECT_NE(Pack(), ERR_OK);
}

HWTEST_F(InstallerPackagerTest, ReusePtJsonManifest, TestSize.Level1)
{
    const std::string bytes = "\xef\xbb\xbf{\"unknown\":1,\"unknown\":2,\"apps\":[" + App().dump() + "]}";
    External(bytes);
    ASSERT_EQ(Pack(), ERR_OK);
    EXPECT_EQ(ReadZip(output_).at("manifest.json"), bytes);
}

HWTEST(InstallerManifestTest, ReadPtJsonNumbers, TestSize.Level1)
{
    auto parsed = PtJson::Parse(
        R"({"negative":-1,"fraction":1.5,"exponent":1e2,"large":2147483648,"text":"1.5","max":2147483647})");
    ASSERT_NE(parsed, nullptr);
    std::unique_ptr<cJSON, InstallerManifest::CJsonDeleter> owner(parsed->GetJson());
    for (const auto& field : {"negative", "fraction", "large", "text", "missing"}) {
        EXPECT_THROW(InstallerManifest::ReadNonNegativeInt32(*parsed, field), std::exception);
    }
    EXPECT_EQ(InstallerManifest::ReadNonNegativeInt32(*parsed, "exponent"), 100);
    EXPECT_EQ(InstallerManifest::ReadNonNegativeInt32(*parsed, "max"), INT32_MAX);
}

HWTEST_F(InstallerPackagerTest, RejectInvalidManifestStructure, TestSize.Level2)
{
    for (const auto& text : {"{", "{/*comment*/}", "{} trailing", "[]", "null", "{}", "{\"apps\":{}}"}) {
        External(text);
        EXPECT_NE(Pack(), ERR_OK) << text;
        EXPECT_FALSE(fs::exists(output_));
    }
}

/*
 * @tc.name: LargePackage_App_Input_Exceeds_0100
 * @tc.desc: AC-3.1: Installer uncompressed size exceeds limit, driven via MakePackage + injected size limit
 * @tc.type: FUNC
 * @tc.require: RR-30089616
 */
HWTEST_F(InstallerPackagerTest, LargePackage_App_Input_Exceeds_0100, TestSize.Level2)
{
    const int64_t testLimit = 1; // 1 byte threshold — any real product exceeds this

    Packager::setMaxAppInputSize(testLimit);

    EXPECT_NE(Pack(), ERR_OK);
    EXPECT_TRUE(result_.find("exceeds") != std::string::npos);
    EXPECT_FALSE(fs::exists(output_));

    Packager::setMaxAppInputSize(Constants::MAX_APP_INPUT_SIZE);
}
}
}
