/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <cstdlib>

#define private public
#define protected public
#include "hap_verify_utils.h"
#include "hap_verify_info.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {

class HapVerifyUtilsTest : public testing::Test {
public:
    HapVerifyUtilsTest() {}
    virtual ~HapVerifyUtilsTest() {}

    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void HapVerifyUtilsTest::SetUpTestCase() {}

void HapVerifyUtilsTest::TearDownTestCase() {}

void HapVerifyUtilsTest::SetUp() {}

void HapVerifyUtilsTest::TearDown() {}

namespace {
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo1;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo2;
}

void InitHapVerifyInfo1()
{
    // hapVerifyInfo1
    hapVerifyInfo1.SetBundleName("test");
    hapVerifyInfo1.SetBundleType("hap");
    hapVerifyInfo1.SetVendor("vendor");

    OHOS::AppPackingTool::Version version1;
    version1.versionCode = 1;
    version1.versionName = "1.0.0";
    version1.minCompatibleVersionCode = 1;
    hapVerifyInfo1.SetVersion(version1);

    OHOS::AppPackingTool::ModuleApiVersion moduleApiVersion1;
    moduleApiVersion1.compatibleApiVersion = 1;
    moduleApiVersion1.targetApiVersion = 1;
    moduleApiVersion1.releaseType = "release";
    hapVerifyInfo1.SetApiVersion(moduleApiVersion1);

    hapVerifyInfo1.SetTargetBundleName("test");
    hapVerifyInfo1.SetTargetPriority(1);
    hapVerifyInfo1.SetDebug(false);
    hapVerifyInfo1.SetModuleName("test");
    hapVerifyInfo1.SetModuleType("entry");

    OHOS::AppPackingTool::MultiAppMode multiAppMode1;
    multiAppMode1.multiAppModeType = "standard";
    multiAppMode1.maxCount = 1;
    hapVerifyInfo1.SetMultiAppMode(multiAppMode1);

    const std::list<std::string> deviceTypes1 = {"phone", "watch"};
    hapVerifyInfo1.SetDeviceTypes(deviceTypes1);

    OHOS::AppPackingTool::ApiVersion apiVersion1;
    apiVersion1.policy = "include";

    OHOS::AppPackingTool::ScreenShape screenShape1;
    screenShape1.policy = "include";

    OHOS::AppPackingTool::ScreenDensity screenDensity1;
    screenDensity1.policy = "include";

    OHOS::AppPackingTool::ScreenWindow screenWindow1;
    screenWindow1.policy = "include";

    OHOS::AppPackingTool::CountryCode countryCode1;
    countryCode1.policy = "include";

    OHOS::AppPackingTool::DistroFilter distroFilter1;
    distroFilter1.apiVersion = apiVersion1;
    distroFilter1.screenShape = screenShape1;
    distroFilter1.screenDensity = screenDensity1;
    distroFilter1.screenWindow = screenWindow1;
    distroFilter1.countryCode = countryCode1;

    hapVerifyInfo1.SetDistroFilter(distroFilter1);
}

void InitHapVerifyInfo2()
{
    // hapVerifyInfo2
    hapVerifyInfo2.SetBundleName("test");
    hapVerifyInfo2.SetBundleType("hap");
    hapVerifyInfo2.SetVendor("vendor");

    OHOS::AppPackingTool::Version version2;
    version2.versionCode = 1;
    version2.versionName = "1.0.0";
    version2.minCompatibleVersionCode = 1;
    hapVerifyInfo2.SetVersion(version2);

    OHOS::AppPackingTool::ModuleApiVersion moduleApiVersion2;
    moduleApiVersion2.compatibleApiVersion = 1;
    moduleApiVersion2.targetApiVersion = 1;
    moduleApiVersion2.releaseType = "release";
    hapVerifyInfo2.SetApiVersion(moduleApiVersion2);

    hapVerifyInfo2.SetTargetBundleName("test");
    hapVerifyInfo2.SetTargetPriority(1);
    hapVerifyInfo2.SetDebug(false);
    hapVerifyInfo2.SetModuleName("test2");
    hapVerifyInfo2.SetModuleType("entry");

    OHOS::AppPackingTool::MultiAppMode multiAppMode2;
    multiAppMode2.multiAppModeType = "standard";
    multiAppMode2.maxCount = 1;
    hapVerifyInfo2.SetMultiAppMode(multiAppMode2);

    const std::list<std::string> deviceTypes2 = {"computer", "ipad"};
    hapVerifyInfo2.SetDeviceTypes(deviceTypes2);

    OHOS::AppPackingTool::ApiVersion apiVersion2;
    apiVersion2.policy = "exclude";
    apiVersion2.value = {"c", "d"};

    OHOS::AppPackingTool::ScreenShape screenShape2;
    screenShape2.policy = "exclude";
    screenShape2.value = {"c", "d"};

    OHOS::AppPackingTool::ScreenDensity screenDensity2;
    screenDensity2.policy = "exclude";
    screenDensity2.value = {"c", "d"};

    OHOS::AppPackingTool::ScreenWindow screenWindow2;
    screenWindow2.policy = "exclude";
    screenWindow2.value = {"c", "d"};

    OHOS::AppPackingTool::CountryCode countryCode2;
    countryCode2.policy = "exclude";
    countryCode2.value = {"c", "d"};

    OHOS::AppPackingTool::DistroFilter distroFilter2;
    distroFilter2.apiVersion = apiVersion2;
    distroFilter2.screenShape = screenShape2;
    distroFilter2.screenDensity = screenDensity2;
    distroFilter2.screenWindow = screenWindow2;
    distroFilter2.countryCode = countryCode2;

    hapVerifyInfo2.SetDistroFilter(distroFilter2);
}

void initDistroFilter(
    OHOS::AppPackingTool::DistroFilter& distroFilter1, OHOS::AppPackingTool::DistroFilter& distroFilter2)
{
    OHOS::AppPackingTool::ApiVersion apiVersion1;
    apiVersion1.policy = "";
    OHOS::AppPackingTool::ScreenShape screenShape1;
    screenShape1.policy = "";
    OHOS::AppPackingTool::ScreenDensity screenDensity1;
    screenDensity1.policy = "";
    OHOS::AppPackingTool::ScreenWindow screenWindow1;
    screenWindow1.policy = "";
    OHOS::AppPackingTool::CountryCode countryCode1;
    countryCode1.policy = "include";
    distroFilter1.apiVersion = apiVersion1;
    distroFilter1.screenShape = screenShape1;
    distroFilter1.screenDensity = screenDensity1;
    distroFilter1.screenWindow = screenWindow1;
    distroFilter1.countryCode = countryCode1;

    OHOS::AppPackingTool::ApiVersion apiVersion2;
    apiVersion2.policy = "exclude";
    apiVersion2.value = {"c", "d"};
    OHOS::AppPackingTool::ScreenShape screenShape2;
    screenShape2.policy = "exclude";
    screenShape2.value = {"c", "d"};
    OHOS::AppPackingTool::ScreenDensity screenDensity2;
    screenDensity2.policy = "exclude";
    screenDensity2.value = {"c", "d"};
    OHOS::AppPackingTool::ScreenWindow screenWindow2;
    screenWindow2.policy = "exclude";
    screenWindow2.value = {"c", "d"};
    OHOS::AppPackingTool::CountryCode countryCode2;
    countryCode2.policy = "";
    distroFilter2.apiVersion = apiVersion2;
    distroFilter2.screenShape = screenShape2;
    distroFilter2.screenDensity = screenDensity2;
    distroFilter2.screenWindow = screenWindow2;
    distroFilter2.countryCode = countryCode2;
}

/*
 * @tc.name: CheckHapIsValid_0100
 * @tc.desc: CheckHapIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckHapIsValid_0100, Function | MediumTest | Level1)
{
    InitHapVerifyInfo1();
    InitHapVerifyInfo2();

    // hapVerifyInfos
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo2);

    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_TRUE(utils.CheckHapIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckHapIsValid_0200
 * @tc.desc: CheckHapIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckHapIsValid_0200, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckHapIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_0100
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_0100, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_0200
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_0200, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo;
    hapVerifyInfos.push_back(hapVerifyInfo);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_0300
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_0300, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleName("test3");
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_0400
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_0400, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleName("test");
    hapVerifyInfo3.SetBundleType("hap3");
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_0500
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_0500, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleName("test");
    hapVerifyInfo3.SetBundleType("hap");
    OHOS::AppPackingTool::Version version;
    version.versionCode = 3;
    version.versionName = "1.0.0";
    version.minCompatibleVersionCode = 1;
    hapVerifyInfo3.SetVersion(version);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));

    std::list<OHOS::AppPackingTool::HapVerifyInfo>::iterator it;
    it = --hapVerifyInfos.end();
    hapVerifyInfos.erase(it);
    version.versionCode = 1;
    version.versionName = "1.0.0";
    version.minCompatibleVersionCode = 2;
    hapVerifyInfo3.SetVersion(version);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_0600
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_0600, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleName("test");
    hapVerifyInfo3.SetBundleType("hap");
    OHOS::AppPackingTool::Version version;
    version.versionCode = 1;
    version.versionName = "1.0.0";
    version.minCompatibleVersionCode = 1;
    hapVerifyInfo3.SetVersion(version);
    OHOS::AppPackingTool::ModuleApiVersion moduleApiVersion;
    moduleApiVersion.compatibleApiVersion = 2;
    moduleApiVersion.targetApiVersion = 1;
    moduleApiVersion.releaseType = "release";
    hapVerifyInfo3.SetApiVersion(moduleApiVersion);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));

    std::list<OHOS::AppPackingTool::HapVerifyInfo>::iterator it;
    it = --hapVerifyInfos.end();
    hapVerifyInfos.erase(it);
    moduleApiVersion.compatibleApiVersion = 1;
    moduleApiVersion.targetApiVersion = 2;
    moduleApiVersion.releaseType = "release";
    hapVerifyInfo3.SetApiVersion(moduleApiVersion);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));

    it = --hapVerifyInfos.end();
    hapVerifyInfos.erase(it);
    moduleApiVersion.compatibleApiVersion = 1;
    moduleApiVersion.targetApiVersion = 1;
    moduleApiVersion.releaseType = "debug";
    hapVerifyInfo3.SetApiVersion(moduleApiVersion);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_0700
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_0700, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleName("test");
    hapVerifyInfo3.SetBundleType("hap");
    OHOS::AppPackingTool::Version version;
    version.versionCode = 1;
    version.versionName = "1.0.0";
    version.minCompatibleVersionCode = 1;
    hapVerifyInfo3.SetVersion(version);
    OHOS::AppPackingTool::ModuleApiVersion moduleApiVersion;
    moduleApiVersion.compatibleApiVersion = 1;
    moduleApiVersion.targetApiVersion = 1;
    moduleApiVersion.releaseType = "release";
    hapVerifyInfo3.SetApiVersion(moduleApiVersion);
    hapVerifyInfo3.SetTargetBundleName("test3");
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_0800
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_0800, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleName("test");
    hapVerifyInfo3.SetBundleType("hap");
    OHOS::AppPackingTool::Version version;
    version.versionCode = 1;
    version.versionName = "1.0.0";
    version.minCompatibleVersionCode = 1;
    hapVerifyInfo3.SetVersion(version);
    OHOS::AppPackingTool::ModuleApiVersion moduleApiVersion;
    moduleApiVersion.compatibleApiVersion = 1;
    moduleApiVersion.targetApiVersion = 1;
    moduleApiVersion.releaseType = "release";
    hapVerifyInfo3.SetApiVersion(moduleApiVersion);
    hapVerifyInfo3.SetTargetBundleName("test");
    hapVerifyInfo3.SetTargetPriority(2);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_0900
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_0900, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleName("test");
    hapVerifyInfo3.SetBundleType("hap");
    OHOS::AppPackingTool::Version version;
    version.versionCode = 1;
    version.versionName = "1.0.0";
    version.minCompatibleVersionCode = 1;
    hapVerifyInfo3.SetVersion(version);
    OHOS::AppPackingTool::ModuleApiVersion moduleApiVersion;
    moduleApiVersion.compatibleApiVersion = 1;
    moduleApiVersion.targetApiVersion = 1;
    moduleApiVersion.releaseType = "release";
    hapVerifyInfo3.SetApiVersion(moduleApiVersion);
    hapVerifyInfo3.SetTargetBundleName("test");
    hapVerifyInfo3.SetTargetPriority(1);
    hapVerifyInfo3.SetDebug(true);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckAppFieldsIsSame_1000
 * @tc.desc: CheckAppFieldsIsSame
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAppFieldsIsSame_1000, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleName("test");
    hapVerifyInfo3.SetBundleType("hap");
    OHOS::AppPackingTool::Version version;
    version.versionCode = 1;
    version.versionName = "1.0.0";
    version.minCompatibleVersionCode = 1;
    hapVerifyInfo3.SetVersion(version);
    OHOS::AppPackingTool::ModuleApiVersion moduleApiVersion;
    moduleApiVersion.compatibleApiVersion = 1;
    moduleApiVersion.targetApiVersion = 1;
    moduleApiVersion.releaseType = "release";
    hapVerifyInfo3.SetApiVersion(moduleApiVersion);
    hapVerifyInfo3.SetTargetBundleName("test");
    hapVerifyInfo3.SetTargetPriority(1);
    hapVerifyInfo3.SetDebug(false);
    hapVerifyInfo3.SetModuleType("entry");
    OHOS::AppPackingTool::MultiAppMode multiAppMode;
    multiAppMode.multiAppModeType = "nostandard";
    multiAppMode.maxCount = 2;
    hapVerifyInfo3.SetMultiAppMode(multiAppMode);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckAppFieldsIsSame(hapVerifyInfos));
}

/*
 * @tc.name: CheckModuleNameIsValid_0100
 * @tc.desc: CheckModuleNameIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckModuleNameIsValid_0100, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckModuleNameIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckModuleNameIsValid_0200
 * @tc.desc: CheckModuleNameIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckModuleNameIsValid_0200, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetModuleName("test");
    const std::list<std::string> deviceTypes = {"phone", "watch"};
    hapVerifyInfo3.SetDeviceTypes(deviceTypes);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckModuleNameIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckDistroFilterDisjoint_0100
 * @tc.desc: CheckDistroFilterDisjoint
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckDistroFilterDisjoint_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::DistroFilter distroFilter1;
    OHOS::AppPackingTool::DistroFilter distroFilter2;
    initDistroFilter(distroFilter1, distroFilter2);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckDistroFilterDisjoint(distroFilter1, distroFilter2));

    OHOS::AppPackingTool::CountryCode countryCode2;
    countryCode2.policy = "exclude";
    countryCode2.value = {"c", "d"};
    distroFilter2.countryCode = countryCode2;
    EXPECT_TRUE(utils.CheckDistroFilterDisjoint(distroFilter1, distroFilter2));

    OHOS::AppPackingTool::ScreenWindow screenWindow1;
    screenWindow1.policy = "include";
    distroFilter1.screenWindow = screenWindow1;
    EXPECT_TRUE(utils.CheckDistroFilterDisjoint(distroFilter1, distroFilter2));

    OHOS::AppPackingTool::ScreenDensity screenDensity1;
    screenDensity1.policy = "include";
    distroFilter1.screenDensity = screenDensity1;
    EXPECT_TRUE(utils.CheckDistroFilterDisjoint(distroFilter1, distroFilter2));

    OHOS::AppPackingTool::ScreenShape screenShape1;
    screenShape1.policy = "include";
    distroFilter1.screenShape = screenShape1;
    EXPECT_TRUE(utils.CheckDistroFilterDisjoint(distroFilter1, distroFilter2));

    OHOS::AppPackingTool::ApiVersion apiVersion1;
    apiVersion1.policy = "include";
    distroFilter1.apiVersion = apiVersion1;
    EXPECT_TRUE(utils.CheckDistroFilterDisjoint(distroFilter1, distroFilter2));
}

/*
 * @tc.name: CheckPolicyValueDisjoint_0100
 * @tc.desc: CheckPolicyValueDisjoint
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckPolicyValueDisjoint_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::PolicyValue policyValue1;
    OHOS::AppPackingTool::PolicyValue policyValue2;
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_FALSE(utils.CheckPolicyValueDisjoint(policyValue1, policyValue2));

    policyValue1.policy = "exclude";
    policyValue2.policy = "exclude";
    EXPECT_FALSE(utils.CheckPolicyValueDisjoint(policyValue1, policyValue2));

    policyValue1.policy = "include";
    policyValue1.value = {"c", "d"};
    policyValue2.policy = "exclude";
    EXPECT_FALSE(utils.CheckPolicyValueDisjoint(policyValue1, policyValue2));

    policyValue1.policy = "include";
    policyValue1.value = {"c", "d"};
    policyValue2.policy = "include";
    policyValue2.value = {"c", "d"};
    EXPECT_FALSE(utils.CheckPolicyValueDisjoint(policyValue1, policyValue2));

    policyValue1.value.clear();
    EXPECT_TRUE(utils.CheckPolicyValueDisjoint(policyValue1, policyValue2));

    policyValue1.policy = "exclude";
    policyValue2.policy = "include";
    EXPECT_FALSE(utils.CheckPolicyValueDisjoint(policyValue1, policyValue2));

    policyValue1.value = {"c", "d"};
    EXPECT_TRUE(utils.CheckPolicyValueDisjoint(policyValue1, policyValue2));
}

/*
 * @tc.name: CheckPackageNameIsValid_0100
 * @tc.desc: CheckPackageNameIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckPackageNameIsValid_0100, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_TRUE(utils.CheckPackageNameIsValid(hapVerifyInfos));
    hapVerifyInfos.clear();
    const std::list<std::string> deviceTypes = {"computer", "ipad"};
    hapVerifyInfo3.SetDeviceTypes(deviceTypes);
    hapVerifyInfo3.SetPackageName("test");
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    hapVerifyInfo4.SetPackageName("test");
    hapVerifyInfo4.SetDeviceTypes(deviceTypes);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckPackageNameIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckEntryIsValid_0100
 * @tc.desc: CheckEntryIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckEntryIsValid_0100, Function | MediumTest | Level1)
{
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyUtils utils;
    EXPECT_TRUE(utils.CheckEntryIsValid(hapVerifyInfos));

    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetModuleType("feature");
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    hapVerifyInfo4.SetModuleType("other");
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo4);
    EXPECT_TRUE(utils.CheckEntryIsValid(hapVerifyInfos));

    hapVerifyInfos.clear();
    hapVerifyInfo3.SetModuleType("entry");
    const std::list<std::string> deviceTypes3 = {"phone", "watch"};
    hapVerifyInfo3.SetDeviceTypes(deviceTypes3);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckEntryIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckFeature_0100
 * @tc.desc: CheckFeature
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckFeature_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    std::map<std::string, std::list<OHOS::AppPackingTool::HapVerifyInfo>> deviceHaps;
    EXPECT_TRUE(utils.CheckFeature(hapVerifyInfo3, deviceHaps));

    const std::list<std::string> deviceTypes3 = {"phone"};
    hapVerifyInfo3.SetDeviceTypes(deviceTypes3);
    EXPECT_FALSE(utils.CheckFeature(hapVerifyInfo3, deviceHaps));

    OHOS::AppPackingTool::ApiVersion apiVersion3;
    apiVersion3.policy = "other";
    OHOS::AppPackingTool::DistroFilter distroFilter3;
    distroFilter3.apiVersion = apiVersion3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    hapVerifyInfos.push_back(hapVerifyInfo3);
    std::string device = "phone";
    deviceHaps.insert(std::make_pair(device, hapVerifyInfos));
    EXPECT_FALSE(utils.CheckFeature(hapVerifyInfo3, deviceHaps));
}

/*
 * @tc.name: CheckFeatureDistroFilter_0100
 * @tc.desc: CheckFeatureDistroFilter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckFeatureDistroFilter_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> entryHaps;
    EXPECT_TRUE(utils.CheckFeatureDistroFilter(hapVerifyInfo3, entryHaps));

    OHOS::AppPackingTool::ApiVersion apiVersion3;
    apiVersion3.policy = "other";
    OHOS::AppPackingTool::DistroFilter distroFilter3;
    distroFilter3.apiVersion = apiVersion3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    entryHaps.push_back(hapVerifyInfo3);
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    EXPECT_FALSE(utils.CheckFeatureDistroFilter(hapVerifyInfo4, entryHaps));
    EXPECT_FALSE(utils.CheckFeatureDistroFilter(hapVerifyInfo3, entryHaps));

    entryHaps.clear();
    apiVersion3.policy.clear();
    distroFilter3.apiVersion = apiVersion3;
    OHOS::AppPackingTool::ScreenShape screenShape3;
    screenShape3.policy = "other";
    distroFilter3.screenShape = screenShape3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    entryHaps.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckFeatureDistroFilter(hapVerifyInfo3, entryHaps));

    entryHaps.clear();
    screenShape3.policy.clear();
    distroFilter3.screenShape = screenShape3;
    OHOS::AppPackingTool::ScreenWindow screenWindow3;
    screenWindow3.policy = "other";
    distroFilter3.screenWindow = screenWindow3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    entryHaps.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckFeatureDistroFilter(hapVerifyInfo3, entryHaps));

    entryHaps.clear();
    screenWindow3.policy.clear();
    distroFilter3.screenWindow = screenWindow3;
    OHOS::AppPackingTool::ScreenDensity screenDensity3;
    screenDensity3.policy = "other";
    distroFilter3.screenDensity = screenDensity3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    entryHaps.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckFeatureDistroFilter(hapVerifyInfo3, entryHaps));
}

/*
 * @tc.name: CheckFeatureDistroFilter_0200
 * @tc.desc: CheckFeatureDistroFilter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckFeatureDistroFilter_0200, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> entryHaps;
    OHOS::AppPackingTool::DistroFilter distroFilter3;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::CountryCode countryCode3;
    countryCode3.policy = "other";
    distroFilter3.countryCode = countryCode3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    entryHaps.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckFeatureDistroFilter(hapVerifyInfo3, entryHaps));
}

/*
 * @tc.name: CheckApiVersionCovered_0100
 * @tc.desc: CheckApiVersionCovered
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckApiVersionCovered_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    OHOS::AppPackingTool::ApiVersion apiVersion;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> entryHaps;
    apiVersion.policy = "other";
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::ApiVersion apiVersion3;
    apiVersion3.policy = "include";
    OHOS::AppPackingTool::DistroFilter distroFilter3;
    distroFilter3.apiVersion = apiVersion3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    OHOS::AppPackingTool::ApiVersion apiVersion4;
    apiVersion4.policy = "exclude";
    OHOS::AppPackingTool::DistroFilter distroFilter4;
    distroFilter4.apiVersion = apiVersion4;
    hapVerifyInfo4.SetDistroFilter(distroFilter4);
    entryHaps.push_back(hapVerifyInfo3);
    entryHaps.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckApiVersionCovered(apiVersion, entryHaps));
}

/*
 * @tc.name: CheckScreenShapeCovered_0100
 * @tc.desc: CheckScreenShapeCovered
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckScreenShapeCovered_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    OHOS::AppPackingTool::ScreenShape screenShape;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> entryHaps;
    screenShape.policy = "other";
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::ScreenShape screenShape3;
    screenShape3.policy = "include";
    OHOS::AppPackingTool::DistroFilter distroFilter3;
    distroFilter3.screenShape = screenShape3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    OHOS::AppPackingTool::ScreenShape screenShape4;
    screenShape4.policy = "exclude";
    OHOS::AppPackingTool::DistroFilter distroFilter4;
    distroFilter4.screenShape = screenShape4;
    hapVerifyInfo4.SetDistroFilter(distroFilter4);
    entryHaps.push_back(hapVerifyInfo3);
    entryHaps.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckScreenShapeCovered(screenShape, entryHaps));
}

/*
 * @tc.name: CheckScreenWindowCovered_0100
 * @tc.desc: CheckScreenWindowCovered
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckScreenWindowCovered_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    OHOS::AppPackingTool::ScreenWindow screenWindow;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> entryHaps;
    screenWindow.policy = "other";
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::ScreenWindow screenWindow3;
    screenWindow3.policy = "include";
    OHOS::AppPackingTool::DistroFilter distroFilter3;
    distroFilter3.screenWindow = screenWindow3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    OHOS::AppPackingTool::ScreenWindow screenWindow4;
    screenWindow4.policy = "exclude";
    OHOS::AppPackingTool::DistroFilter distroFilter4;
    distroFilter4.screenWindow = screenWindow4;
    hapVerifyInfo4.SetDistroFilter(distroFilter4);
    entryHaps.push_back(hapVerifyInfo3);
    entryHaps.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckScreenWindowCovered(screenWindow, entryHaps));
}

/*
 * @tc.name: CheckScreenDensityCovered_0100
 * @tc.desc: CheckScreenDensityCovered
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckScreenDensityCovered_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    OHOS::AppPackingTool::ScreenDensity screenDensity;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> entryHaps;
    screenDensity.policy = "other";
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::ScreenDensity screenDensity3;
    screenDensity3.policy = "include";
    OHOS::AppPackingTool::DistroFilter distroFilter3;
    distroFilter3.screenDensity = screenDensity3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    OHOS::AppPackingTool::ScreenDensity screenDensity4;
    screenDensity4.policy = "exclude";
    OHOS::AppPackingTool::DistroFilter distroFilter4;
    distroFilter4.screenDensity = screenDensity4;
    hapVerifyInfo4.SetDistroFilter(distroFilter4);
    entryHaps.push_back(hapVerifyInfo3);
    entryHaps.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckScreenDensityCovered(screenDensity, entryHaps));
}

/*
 * @tc.name: CheckCountryCodeCovered_0100
 * @tc.desc: CheckCountryCodeCovered
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckCountryCodeCovered_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    OHOS::AppPackingTool::CountryCode countryCode;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> entryHaps;
    countryCode.policy = "other";
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::CountryCode countryCode3;
    countryCode3.policy = "include";
    OHOS::AppPackingTool::DistroFilter distroFilter3;
    distroFilter3.countryCode = countryCode3;
    hapVerifyInfo3.SetDistroFilter(distroFilter3);
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    OHOS::AppPackingTool::CountryCode countryCode4;
    countryCode4.policy = "exclude";
    OHOS::AppPackingTool::DistroFilter distroFilter4;
    distroFilter4.countryCode = countryCode4;
    hapVerifyInfo4.SetDistroFilter(distroFilter4);
    entryHaps.push_back(hapVerifyInfo3);
    entryHaps.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckCountryCodeCovered(countryCode, entryHaps));
}

/*
 * @tc.name: CheckDependencyIsValid_0100
 * @tc.desc: CheckDependencyIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckDependencyIsValid_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    EXPECT_FALSE(utils.CheckDependencyIsValid(hapVerifyInfos));

    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    hapVerifyInfo4.SetInstallationFree(true);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckDependencyIsValid(hapVerifyInfos));

    hapVerifyInfos.clear();
    hapVerifyInfo4.SetInstallationFree(false);
    hapVerifyInfo3.SetBundleName("test");
    hapVerifyInfo3.SetModuleName("test");
    hapVerifyInfo3.SetModuleType("entry");
    std::list<std::string> deviceTypes3 = {"computer"};
    hapVerifyInfo3.SetDeviceTypes(deviceTypes3);
    hapVerifyInfo4.SetBundleName("test");
    hapVerifyInfo4.SetModuleName("test");
    hapVerifyInfo4.SetModuleType("entry");
    OHOS::AppPackingTool::DependencyItem dependencyItem;
    dependencyItem.bundleName = "test";
    dependencyItem.moduleName = "test";
    std::list<OHOS::AppPackingTool::DependencyItem> dependencyItemList;
    dependencyItemList.push_back(dependencyItem);
    hapVerifyInfo3.SetDependencyItemList(dependencyItemList);
    std::list<std::string> deviceTypes4 = {"computer"};
    hapVerifyInfo4.SetDeviceTypes(deviceTypes4);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckDependencyIsValid(hapVerifyInfos));
}

/*
 * @tc.name: DfsTraverseDependency_0100
 * @tc.desc: DfsTraverseDependency
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, DfsTraverseDependency_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> dependencyList;
    hapVerifyInfo3.SetModuleName("test");
    hapVerifyInfo4.SetModuleName("test");
    std::list<std::string> deviceTypes3 = {"computer", "ipad"};
    hapVerifyInfo3.SetDeviceTypes(deviceTypes3);
    hapVerifyInfo4.SetDeviceTypes(deviceTypes3);
    dependencyList.push_back(hapVerifyInfo3);
    dependencyList.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.DfsTraverseDependency(hapVerifyInfo3, hapVerifyInfos, dependencyList));

    dependencyList.clear();
    hapVerifyInfo3.SetBundleName("test");
    std::list<OHOS::AppPackingTool::DependencyItem> dependencyItemList;
    OHOS::AppPackingTool::DependencyItem dependencyItem1;
    OHOS::AppPackingTool::DependencyItem dependencyItem2;
    dependencyItem1.bundleName = "other";
    dependencyItem2.bundleName = "test";
    dependencyItemList.push_back(dependencyItem1);
    dependencyItemList.push_back(dependencyItem2);
    hapVerifyInfo3.SetDependencyItemList(dependencyItemList);
    EXPECT_TRUE(utils.DfsTraverseDependency(hapVerifyInfo3, hapVerifyInfos, dependencyList));
}

/*
 * @tc.name: DfsTraverseDependency_0200
 * @tc.desc: DfsTraverseDependency
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, DfsTraverseDependency_0200, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> dependencyList;
    hapVerifyInfo3.SetBundleName("test");
    hapVerifyInfo3.SetModuleName("test");
    std::list<OHOS::AppPackingTool::DependencyItem> dependencyItemList;
    OHOS::AppPackingTool::DependencyItem dependencyItem1;
    dependencyItem1.bundleName = "test";
    dependencyItem1.moduleName = "test";
    dependencyItemList.push_back(dependencyItem1);
    hapVerifyInfo3.SetDependencyItemList(dependencyItemList);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_TRUE(utils.DfsTraverseDependency(hapVerifyInfo3, hapVerifyInfos, dependencyList));
}

/*
 * @tc.name: CheckAtomicServiceIsValid_0100
 * @tc.desc: CheckAtomicServiceIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAtomicServiceIsValid_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    EXPECT_FALSE(utils.CheckAtomicServiceIsValid(hapVerifyInfos));

    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleType("hap");
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_TRUE(utils.CheckAtomicServiceIsValid(hapVerifyInfos));

    hapVerifyInfos.clear();
    hapVerifyInfo3.SetBundleType("atomicService");
    hapVerifyInfo3.SetStageModule(false);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_TRUE(utils.CheckAtomicServiceIsValid(hapVerifyInfos));

    hapVerifyInfos.clear();
    hapVerifyInfo3.SetBundleType("atomicService");
    hapVerifyInfo3.SetStageModule(true);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_TRUE(utils.CheckAtomicServiceIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckAtomicServiceIsValid_0200
 * @tc.desc: CheckAtomicServiceIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAtomicServiceIsValid_0200, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleType("atomicService");
    hapVerifyInfo3.SetModuleType("other");
    hapVerifyInfo3.SetStageModule(true);
    const std::list<std::string> deviceTypes3 = {"phone", "watch"};
    hapVerifyInfo3.SetDeviceTypes(deviceTypes3);
    std::list<OHOS::AppPackingTool::PreloadItem> preloadItems;
    OHOS::AppPackingTool::PreloadItem preloadItem;
    preloadItem.moduleName = "test";
    preloadItems.push_back(preloadItem);
    hapVerifyInfo3.SetPreloadItems(preloadItems);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckAtomicServiceIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckAtomicServiceIsValid_0300
 * @tc.desc: CheckAtomicServiceIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAtomicServiceIsValid_0300, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetBundleType("atomicService");
    hapVerifyInfo3.SetModuleType("entry");
    hapVerifyInfo3.SetStageModule(true);
    int32_t entrySizeLimit = 0;
    hapVerifyInfo3.SetEntrySizeLimit(entrySizeLimit);
    int64_t fileLength = 1024;
    hapVerifyInfo3.SetFileLength(fileLength);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckAtomicServiceIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckAbilityNameIsValid_0100
 * @tc.desc: CheckAbilityNameIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckAbilityNameIsValid_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    std::list<std::string> abilityNames = {"test", "test"};
    hapVerifyInfo3.SetAbilityNames(abilityNames);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckAbilityNameIsValid(hapVerifyInfos));

    hapVerifyInfos.clear();
    abilityNames = {"test"};
    hapVerifyInfo3.SetAbilityNames(abilityNames);
    std::list<std::string> deviceTypes = {"phone", "watch"};
    hapVerifyInfo3.SetDeviceTypes(deviceTypes);
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    hapVerifyInfo4.SetAbilityNames(abilityNames);
    hapVerifyInfo4.SetDeviceTypes(deviceTypes);
    hapVerifyInfos.push_back(hapVerifyInfo1);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckAbilityNameIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckTargetModuleNameIsExisted_0100
 * @tc.desc: CheckTargetModuleNameIsExisted
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckTargetModuleNameIsExisted_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    hapVerifyInfo3.SetTargetBundleName("test");
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_TRUE(utils.CheckTargetModuleNameIsExisted(hapVerifyInfos));

    hapVerifyInfos.clear();
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    hapVerifyInfo4.SetTargetModuleName("test");
    hapVerifyInfos.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckTargetModuleNameIsExisted(hapVerifyInfos));

    hapVerifyInfos.clear();
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo5;
    hapVerifyInfos.push_back(hapVerifyInfo5);
    EXPECT_TRUE(utils.CheckTargetModuleNameIsExisted(hapVerifyInfos));

    hapVerifyInfos.clear();
    hapVerifyInfos.push_back(hapVerifyInfo4);
    hapVerifyInfo5.SetModuleName("other");
    hapVerifyInfos.push_back(hapVerifyInfo5);
    EXPECT_FALSE(utils.CheckTargetModuleNameIsExisted(hapVerifyInfos));
}

/*
 * @tc.name: CheckCompileSdkIsValid_0100
 * @tc.desc: CheckCompileSdkIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckCompileSdkIsValid_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    EXPECT_FALSE(utils.CheckCompileSdkIsValid(hapVerifyInfos));

    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    hapVerifyInfo3.SetCompileSdkType("test");
    hapVerifyInfo4.SetCompileSdkType("other");
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckCompileSdkIsValid(hapVerifyInfos));

    hapVerifyInfos.clear();
    hapVerifyInfo3.SetCompileSdkType("test");
    hapVerifyInfo4.SetCompileSdkType("test");
    hapVerifyInfo3.SetCompileSdkVersion("test");
    hapVerifyInfo4.SetCompileSdkVersion("other");
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckCompileSdkIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckProxyDataUriIsUnique_0100
 * @tc.desc: CheckProxyDataUriIsUnique
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckProxyDataUriIsUnique_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    EXPECT_FALSE(utils.CheckProxyDataUriIsUnique(hapVerifyInfos));

    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    std::list<std::string> proxyDataUris = {"test", "test"};
    hapVerifyInfo3.SetProxyDataUris(proxyDataUris);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckProxyDataUriIsUnique(hapVerifyInfos));
}

/*
 * @tc.name: CheckContinueTypeIsValid_0100
 * @tc.desc: CheckContinueTypeIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckContinueTypeIsValid_0100, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    std::list<std::string> abilityNames = {"test1", "test2"};
    hapVerifyInfo3.SetAbilityNames(abilityNames);
    std::map<std::string, std::list<std::string>> continueTypeMap;
    std::string type1 = "test1";
    std::string type2 = "test2";
    std::list<std::string> strList = {"test"};
    continueTypeMap.insert(std::make_pair(type1, strList));
    continueTypeMap.insert(std::make_pair(type2, strList));
    hapVerifyInfo3.SetContinueTypeMap(continueTypeMap);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    EXPECT_FALSE(utils.CheckContinueTypeIsValid(hapVerifyInfos));
}

/*
 * @tc.name: CheckContinueTypeIsValid_0200
 * @tc.desc: CheckContinueTypeIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HapVerifyUtilsTest, CheckContinueTypeIsValid_0200, Function | MediumTest | Level1)
{
    OHOS::AppPackingTool::HapVerifyUtils utils;
    std::list<OHOS::AppPackingTool::HapVerifyInfo> hapVerifyInfos;
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo3;
    std::list<std::string> abilityNames = {"test1"};
    hapVerifyInfo3.SetAbilityNames(abilityNames);
    std::map<std::string, std::list<std::string>> continueTypeMap;
    std::string type1 = "test1";
    std::list<std::string> strList = {"test"};
    continueTypeMap.insert(std::make_pair(type1, strList));
    hapVerifyInfo3.SetContinueTypeMap(continueTypeMap);
    std::list<std::string> deviceTypes = {"phone"};
    hapVerifyInfo3.SetDeviceTypes(deviceTypes);
    OHOS::AppPackingTool::HapVerifyInfo hapVerifyInfo4;
    hapVerifyInfo4.SetAbilityNames(abilityNames);
    hapVerifyInfo4.SetContinueTypeMap(continueTypeMap);
    hapVerifyInfo4.SetDeviceTypes(deviceTypes);
    hapVerifyInfos.push_back(hapVerifyInfo3);
    hapVerifyInfos.push_back(hapVerifyInfo4);
    EXPECT_FALSE(utils.CheckContinueTypeIsValid(hapVerifyInfos));
}

/*
 * @tc.name: GetDistroFilter_0100
 * @tc.desc: GetDistroFilter
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetDistroFilter_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    auto ret = info.GetDistroFilter();
    EXPECT_NE(&ret, nullptr);
}

/*
 * @tc.name: IsStageModule_0100
 * @tc.desc: IsStageModule
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, IsStageModule_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    auto ret = info.IsStageModule();
    EXPECT_TRUE(ret);
}

/*
 * @tc.name: GetDependencies_0100
 * @tc.desc: GetDependencies
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetDependencies_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    std::list<std::string> dependencies;
    dependencies.push_front("test");
    info.SetDependencies(dependencies);

    std::list<std::string> ret = info.GetDependencies();
    EXPECT_EQ(ret.size(), dependencies.size());
}

/*
 * @tc.name: GetProfileStr_0100
 * @tc.desc: GetProfileStr
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetProfileStr_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    std::string ret = info.GetProfileStr();
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetAtomicServiceType_0100
 * @tc.desc: GetAtomicServiceType
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetAtomicServiceType_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    std::string ret = info.GetAtomicServiceType();
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: SetAtomicServiceType_0100
 * @tc.desc: SetAtomicServiceType
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, SetAtomicServiceType_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    std::string atomicServiceType = "test";
    info.SetAtomicServiceType(atomicServiceType);

    std::string ret = info.GetAtomicServiceType();
    EXPECT_EQ(ret, atomicServiceType);
}

/*
 * @tc.name: GetPreloadItems_0100
 * @tc.desc: GetPreloadItems
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetPreloadItems_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    std::list<AppPackingTool::PreloadItem> ret = info.GetPreloadItems();
    EXPECT_EQ(ret.size(), 0);
}

/*
 * @tc.name: GetTargetModulePriority_0100
 * @tc.desc: GetTargetModulePriority
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetTargetModulePriority_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    int32_t ret = info.GetTargetModulePriority();
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: GetFileLength_0100
 * @tc.desc: GetFileLength
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetFileLength_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    int64_t ret = info.GetFileLength();
    EXPECT_EQ(ret, 0L);
}

/*
 * @tc.name: GetEntrySizeLimit_0100
 * @tc.desc: GetEntrySizeLimit
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetEntrySizeLimit_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    int32_t limit = 100;
    info.SetEntrySizeLimit(limit);

    int32_t ret = info.GetEntrySizeLimit();
    EXPECT_EQ(ret, limit);
}

/*
 * @tc.name: GetNotEntrySizeLimit_0100
 * @tc.desc: GetNotEntrySizeLimit
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetNotEntrySizeLimit_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    int32_t notEntrySizeLimit = 100;
    info.SetNotEntrySizeLimit(notEntrySizeLimit);

    int32_t ret = info.GetNotEntrySizeLimit();
    EXPECT_EQ(ret, notEntrySizeLimit);
}

/*
 * @tc.name: GetSumSizeLimit_0100
 * @tc.desc: GetSumSizeLimit
 * @tc.type: FUNC
 */
HWTEST_F(HapVerifyUtilsTest, GetSumSizeLimit_0100, Function | MediumTest | Level1)
{
    AppPackingTool::HapVerifyInfo info;

    int32_t sumSizeLimit = 100;
    info.SetSumSizeLimit(sumSizeLimit);

    int32_t ret = info.GetSumSizeLimit();
    EXPECT_EQ(ret, sumSizeLimit);
}
}
