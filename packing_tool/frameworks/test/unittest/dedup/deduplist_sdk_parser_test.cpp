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

#include "dedup/deduplist_sdk_parser.h"

using testing::ext::TestSize;
using OHOS::AppPackingTool::DedupApiVersion;
using OHOS::AppPackingTool::DedupListSdkParser;
using OHOS::AppPackingTool::DedupSoMatch;

namespace {
const std::string VALID_MANIFEST = R"({
  "dedupMinApiVersion": 26,
  "deviceType": ["phone", "tablet"],
  "libraries": [
    {
      "name": "libc++_shared.so",
      "abi": "arm64-v8a",
      "25.0.0": "syssha25",
      "26.0.0": "syssha26"
    }
  ]
})";

// dedupMinApiVersion is injected as raw json so that invalid forms stay expressible
std::string BuildManifest(const std::string& minVersion, const std::string& libraryBody)
{
    return "{\"dedupMinApiVersion\":" + minVersion +
        ",\"deviceType\":[\"phone\",\"tablet\"],\"libraries\":[" + libraryBody + "]}";
}

const std::string VALID_LIBRARY = R"({"name":"a","abi":"arm64-v8a","26.0.0":"sha"})";

std::filesystem::path WriteTempFile(const std::string& name, const std::string& content)
{
    std::filesystem::path path = std::filesystem::temp_directory_path() / std::filesystem::path(name);
    std::ofstream file(path, std::ios::trunc);
    file << content;
    file.close();
    return path;
}
}  // namespace

class DeduplistSdkParserTest : public testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    DedupListSdkParser parser_;
};

void DeduplistSdkParserTest::SetUp() {}
void DeduplistSdkParserTest::TearDown() {}

/**
 * @tc.name: ParseContent_ValidAndInvalid_0100
 * @tc.desc: A well formed manifest is parsed into the expected model while empty content, non object roots,
 *           missing / empty / malformed fields and malformed library records are rejected.
 */
HWTEST_F(DeduplistSdkParserTest, ParseContent_ValidAndInvalid_0100, TestSize.Level0)
{
    ASSERT_TRUE(parser_.ParseContent(VALID_MANIFEST));
    EXPECT_TRUE(parser_.GetDedupMinApiVersion().IsValid());
    EXPECT_TRUE(DedupApiVersion::FromApiVersion(26) == parser_.GetDedupMinApiVersion());
    EXPECT_EQ(parser_.GetDeviceTypes().size(), 2U);
    EXPECT_EQ(parser_.GetDeviceTypes().count("phone"), 1U);
    EXPECT_TRUE(parser_.GetErrorMessage().empty());

    EXPECT_FALSE(parser_.ParseContent(""));
    EXPECT_FALSE(parser_.ParseContent("{oops"));
    EXPECT_FALSE(parser_.ParseContent("\"not an object\""));
    EXPECT_FALSE(parser_.ParseContent("123"));
    EXPECT_FALSE(parser_.ParseContent("[]"));
    EXPECT_FALSE(parser_.ParseContent("{\"deviceType\":[\"phone\"],\"libraries\":[]}"));
    EXPECT_FALSE(parser_.ParseContent("{\"dedupMinApiVersion\":26,\"libraries\":[]}"));
    EXPECT_FALSE(parser_.ParseContent("{\"dedupMinApiVersion\":26,\"deviceType\":[],\"libraries\":[]}"));
    EXPECT_FALSE(parser_.ParseContent("{\"dedupMinApiVersion\":26,\"deviceType\":[\"phone\"]}"));
    EXPECT_FALSE(parser_.ParseContent("{\"dedupMinApiVersion\":26,\"deviceType\":[\"phone\"],"
        "\"libraries\":[]}"));
    // The library value of an api version key must be a non-empty sha256 string
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26", R"({"abi":"arm64-v8a","26.0.0":"sha"})")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26", R"({"name":"a","26.0.0":"sha"})")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26", R"({"name":"a","abi":"arm64-v8a"})")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26",
        R"({"name":"a","abi":"arm64-v8a","bad":"sha","26.0.0":"sha"})")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26",
        R"({"name":"a","abi":"arm64-v8a","26.0.0":{}})")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26",
        R"({"name":"a","abi":"arm64-v8a","26.0.0":""})")));
    EXPECT_FALSE(parser_.GetErrorMessage().empty());
}

/**
 * @tc.name: ParseContent_MinApiVersionBranches_0200
 * @tc.desc: dedupMinApiVersion must be a positive integer: the dotted string, a numeric string, a boolean, an
 *           array, zero, a negative value and an out of range value are rejected, while a plain and a
 *           composite integer api version are accepted.
 */
HWTEST_F(DeduplistSdkParserTest, ParseContent_MinApiVersionBranches_0200, TestSize.Level0)
{
    ASSERT_TRUE(parser_.ParseContent(BuildManifest("26", VALID_LIBRARY)));
    EXPECT_TRUE(parser_.GetDedupMinApiVersion() == DedupApiVersion::FromApiVersion(26));

    // A composite integer api version keeps the three segment form
    ASSERT_TRUE(parser_.ParseContent(BuildManifest("26000000", VALID_LIBRARY)));
    DedupApiVersion composite;
    ASSERT_TRUE(DedupApiVersion::Parse("2.60.0", composite));
    EXPECT_TRUE(parser_.GetDedupMinApiVersion() == composite);

    EXPECT_FALSE(parser_.ParseContent(BuildManifest("\"26.0.0\"", VALID_LIBRARY)));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("\"26\"", VALID_LIBRARY)));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("true", VALID_LIBRARY)));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("[]", VALID_LIBRARY)));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("{}", VALID_LIBRARY)));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("0", VALID_LIBRARY)));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("-1", VALID_LIBRARY)));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("2147483648", VALID_LIBRARY)));
    EXPECT_FALSE(parser_.GetErrorMessage().empty());
}

/**
 * @tc.name: ParseContent_DeviceTypeAndLibraryBranches_0300
 * @tc.desc: A deviceType element that is not a non-empty string and a libraries element that is not an object
 *           are rejected, as are library records with an empty / non string name or abi.
 */
HWTEST_F(DeduplistSdkParserTest, ParseContent_DeviceTypeAndLibraryBranches_0300, TestSize.Level0)
{
    EXPECT_FALSE(parser_.ParseContent("{\"dedupMinApiVersion\":26,\"deviceType\":[\"phone\",1],"
        "\"libraries\":[" + VALID_LIBRARY + "]}"));
    EXPECT_FALSE(parser_.ParseContent("{\"dedupMinApiVersion\":26,\"deviceType\":[\"\"],"
        "\"libraries\":[" + VALID_LIBRARY + "]}"));

    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26", "1")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26", "[]")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26", R"({"name":"","abi":"arm64-v8a","26.0.0":"sha"})")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26", R"({"name":"a","abi":"","26.0.0":"sha"})")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26", R"({"name":1,"abi":"arm64-v8a","26.0.0":"sha"})")));
    EXPECT_FALSE(parser_.ParseContent(BuildManifest("26", R"({"name":"a","abi":1,"26.0.0":"sha"})")));
    EXPECT_FALSE(parser_.GetErrorMessage().empty());
}

/**
 * @tc.name: ParseFile_FileAndMissing_0400
 * @tc.desc: A real manifest file is parsed while missing, empty, unreadable and directory paths fail.
 */
HWTEST_F(DeduplistSdkParserTest, ParseFile_FileAndMissing_0400, TestSize.Level0)
{
    std::filesystem::path validPath = WriteTempFile("deduplist_valid_0400.json", VALID_MANIFEST);
    ASSERT_TRUE(parser_.ParseFile(validPath.string()));
    EXPECT_TRUE(parser_.GetDedupMinApiVersion().IsValid());
    std::filesystem::remove(validPath);

    EXPECT_FALSE(parser_.ParseFile(validPath.string()));
    EXPECT_FALSE(parser_.ParseFile(""));
    EXPECT_FALSE(parser_.ParseFile(std::filesystem::temp_directory_path().string()));
    std::filesystem::path emptyPath = WriteTempFile("deduplist_empty_0400.json", "");
    EXPECT_FALSE(parser_.ParseFile(emptyPath.string()));
    std::filesystem::remove(emptyPath);
    EXPECT_FALSE(parser_.GetErrorMessage().empty());
}

/**
 * @tc.name: FindMatch_ImplicitInheritance_0500
 * @tc.desc: A group declared for a lower API version stays visible to equal and higher versions, while an
 *           API version above compatibleSdkVersion, an unknown so / abi / sha256 and an invalid version
 *           never match.
 */
HWTEST_F(DeduplistSdkParserTest, FindMatch_ImplicitInheritance_0500, TestSize.Level0)
{
    ASSERT_TRUE(parser_.ParseContent(VALID_MANIFEST));
    DedupSoMatch match;
    ASSERT_TRUE(parser_.FindMatch("libc++_shared.so", "arm64-v8a", "syssha25",
        DedupApiVersion::FromApiVersion(25), match));
    EXPECT_EQ(match.sha256, "syssha25");
    EXPECT_EQ(match.abi, "arm64-v8a");

    // A lower group is inherited by equal and higher versions
    ASSERT_TRUE(parser_.FindMatch("libc++_shared.so", "arm64-v8a", "syssha25",
        DedupApiVersion::FromApiVersion(26), match));
    EXPECT_EQ(match.sha256, "syssha25");
    EXPECT_FALSE(parser_.FindMatch("libc++_shared.so", "arm64-v8a", "syssha25",
        DedupApiVersion::FromApiVersion(24), match));

    ASSERT_TRUE(parser_.FindMatch("libc++_shared.so", "arm64-v8a", "syssha26",
        DedupApiVersion::FromApiVersion(26), match));
    EXPECT_EQ(match.sha256, "syssha26");
    EXPECT_FALSE(parser_.FindMatch("libc++_shared.so", "arm64-v8a", "syssha26",
        DedupApiVersion::FromApiVersion(25), match));

    EXPECT_FALSE(parser_.FindMatch("libother.so", "arm64-v8a", "syssha26",
        DedupApiVersion::FromApiVersion(26), match));
    EXPECT_FALSE(parser_.FindMatch("libc++_shared.so", "armeabi-v7a", "syssha26",
        DedupApiVersion::FromApiVersion(26), match));
    EXPECT_FALSE(parser_.FindMatch("libc++_shared.so", "arm64-v8a", "unknownsha",
        DedupApiVersion::FromApiVersion(26), match));
    EXPECT_FALSE(parser_.FindMatch("libc++_shared.so", "arm64-v8a", "syssha26", DedupApiVersion(), match));
}

/**
 * @tc.name: FindMatch_SameNameMultipleAbis_0600
 * @tc.desc: The same so name declared for two abis is kept as two library entries and each abi only matches
 *           its own compatibility groups.
 */
HWTEST_F(DeduplistSdkParserTest, FindMatch_SameNameMultipleAbis_0600, TestSize.Level0)
{
    const std::string twoAbis = R"({"name":"libc++_shared.so","abi":"arm64-v8a","26.0.0":"sha64"},)"
        R"({"name":"libc++_shared.so","abi":"armeabi-v7a","25.0.0":"sha32"})";
    ASSERT_TRUE(parser_.ParseContent(BuildManifest("26", twoAbis)));

    DedupSoMatch match;
    ASSERT_TRUE(parser_.FindMatch("libc++_shared.so", "arm64-v8a", "sha64",
        DedupApiVersion::FromApiVersion(26), match));
    EXPECT_EQ(match.abi, "arm64-v8a");
    EXPECT_EQ(match.sha256, "sha64");

    // The arm64 sha256 only belongs to the arm64 entry even when the abi is looked up explicitly
    ASSERT_TRUE(parser_.FindMatch("libc++_shared.so", "armeabi-v7a", "sha32",
        DedupApiVersion::FromApiVersion(26), match));
    EXPECT_EQ(match.abi, "armeabi-v7a");
    EXPECT_EQ(match.sha256, "sha32");
    EXPECT_FALSE(parser_.FindMatch("libc++_shared.so", "armeabi-v7a", "sha64",
        DedupApiVersion::FromApiVersion(26), match));
}

/**
 * @tc.name: DedupApiVersion_ParseAndCompare_0700
 * @tc.desc: Only the dotted three-component form is accepted and comparison zero-pads missing parts.
 */
HWTEST_F(DeduplistSdkParserTest, DedupApiVersion_ParseAndCompare_0700, TestSize.Level0)
{
    DedupApiVersion version;
    EXPECT_TRUE(DedupApiVersion::Parse("26.0.0", version));
    EXPECT_TRUE(version.IsValid());
    EXPECT_FALSE(DedupApiVersion::Parse("26", version));
    EXPECT_FALSE(DedupApiVersion::Parse("26.0", version));
    EXPECT_FALSE(DedupApiVersion::Parse("26.0.0.0", version));
    EXPECT_FALSE(DedupApiVersion::Parse("v10", version));
    EXPECT_FALSE(DedupApiVersion::Parse("10.a.0", version));
    EXPECT_FALSE(DedupApiVersion::Parse("26.0.", version));
    EXPECT_FALSE(DedupApiVersion::Parse("26..0", version));
    EXPECT_FALSE(DedupApiVersion::Parse("", version));

    // An app side integer API version equals its zero padded form
    ASSERT_TRUE(DedupApiVersion::Parse("26.0.0", version));
    EXPECT_TRUE(DedupApiVersion::FromApiVersion(26) == version);
    EXPECT_TRUE(DedupApiVersion::FromApiVersion(25) < version);
    ASSERT_TRUE(DedupApiVersion::Parse("25.9.9", version));
    EXPECT_TRUE(version < DedupApiVersion::FromApiVersion(26));
    EXPECT_TRUE(DedupApiVersion::FromApiVersion(26).IsValid());
    EXPECT_FALSE(DedupApiVersion::FromApiVersion(0).IsValid());
    EXPECT_FALSE(DedupApiVersion().IsValid());

    // A composite integer api version is split into its three segments
    DedupApiVersion composite;
    ASSERT_TRUE(DedupApiVersion::Parse("2.60.0", composite));
    EXPECT_TRUE(DedupApiVersion::FromApiVersion(26000000) == composite);
    EXPECT_TRUE(composite <= DedupApiVersion::FromApiVersion(26000000));
}

/**
 * @tc.name: DedupApiVersion_FromApiVersionBoundaries_0800
 * @tc.desc: A non positive api version has no valid form, the composite threshold maps onto its three
 *           segments exactly and a value below it is used as the major segment of three segments.
 */
HWTEST_F(DeduplistSdkParserTest, DedupApiVersion_FromApiVersionBoundaries_0800, TestSize.Level0)
{
    EXPECT_FALSE(DedupApiVersion::FromApiVersion(-1).IsValid());

    DedupApiVersion threshold;
    ASSERT_TRUE(DedupApiVersion::Parse("1.0.0", threshold));
    EXPECT_TRUE(DedupApiVersion::FromApiVersion(10000000) == threshold);
    EXPECT_EQ(DedupApiVersion::FromApiVersion(10000000).parts.size(), 3U);

    DedupApiVersion belowThreshold;
    ASSERT_TRUE(DedupApiVersion::Parse("9999999.0.0", belowThreshold));
    EXPECT_TRUE(DedupApiVersion::FromApiVersion(9999999) == belowThreshold);
    EXPECT_EQ(DedupApiVersion::FromApiVersion(9999999).parts.size(), 3U);
}

/**
 * @tc.name: DedupApiVersion_ThreeSegmentNormalisation_0900
 * @tc.desc: The high part of a composite app minAPIVersion and a plain dedupMinApiVersion normalise to
 *           the same three segments, so the version gate, dedupMinApiVersion and the manifest keys
 *           compare the same value.
 */
HWTEST_F(DeduplistSdkParserTest, DedupApiVersion_ThreeSegmentNormalisation_0900, TestSize.Level0)
{
    DedupApiVersion expected;
    ASSERT_TRUE(DedupApiVersion::Parse("26.0.0", expected));
    EXPECT_FALSE(DedupApiVersion::FromApiVersion(0).IsValid());

    // The composite app minAPIVersion keeps the SDK version from its high part
    DedupApiVersion fromApp = DedupApiVersion::FromApiVersion(260000026);
    EXPECT_EQ(fromApp.parts.size(), 3U);
    EXPECT_TRUE(fromApp == expected);

    // A plain dedupMinApiVersion becomes the major segment
    DedupApiVersion fromDedupMinApiVersion = DedupApiVersion::FromApiVersion(26);
    EXPECT_EQ(fromDedupMinApiVersion.parts.size(), 3U);
    EXPECT_TRUE(fromDedupMinApiVersion == expected);
    EXPECT_TRUE(fromDedupMinApiVersion <= expected);

    // A higher composite version stays comparable component by component
    DedupApiVersion higher;
    ASSERT_TRUE(DedupApiVersion::Parse("26.1.0", higher));
    EXPECT_TRUE(higher == DedupApiVersion::FromApiVersion(260100026));
    EXPECT_TRUE(expected < DedupApiVersion::FromApiVersion(260100026));
}
