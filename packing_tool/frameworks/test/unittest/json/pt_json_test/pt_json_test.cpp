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
#include "pt_json.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {

std::string content = "{"
   "\"name\": \"Json.CN\","
   "\"app\": \"apptest\","
   "\"module\": {"
        "\"name\": \"nametest\","
        "\"type\": \"typetest\","
        "\"deviceTypes\": ["
            "\"aaaaaaaaaaa\","
            "\"bbbbbbbbbbb\","
            "\"ccccccccccc\""
       "]"
   "},"
   "\"bundleName\": \"bundleNametest\","
   "\"versionCode\": 100,"
   "\"versionName\": \"versionNametest\","
   "\"patchVersionCode\": 200,"
   "\"patchVersionName\": \"patchVersionNametest\","
   "\"originalModuleHash\": \"originalModuleHashtest\""
"}";

class PtJsonTest : public testing::Test {
public:
    PtJsonTest() {}
    virtual ~PtJsonTest() {}

    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void PtJsonTest::SetUpTestCase() {}

void PtJsonTest::TearDownTestCase() {}

void PtJsonTest::SetUp() {}

void PtJsonTest::TearDown() {}

/*
 * @tc.name: CreateObject_0100
 * @tc.desc: CreateObject.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, CreateObject_0100, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjsonObject = ptJson.CreateObject();
    EXPECT_TRUE(ptjsonObject != NULL);
}

/*
 * @tc.name: CreateArray_0200
 * @tc.desc: CreateArray.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, CreateArray_0200, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjsonArray = ptJson.CreateArray();
    EXPECT_TRUE(ptjsonArray != NULL);
}

/*
 * @tc.name: ReleaseRoot_0300
 * @tc.desc: ReleaseRoot.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, ReleaseRoot_0300, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    ptJson.ReleaseRoot();
    EXPECT_TRUE(ptJson.object_ == NULL);
}

/*
 * @tc.name: Parse_0400
 * @tc.desc: Parse.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, Parse_0400, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjson = ptJson.Parse(content);
    EXPECT_TRUE(ptjson != NULL);
}

/*
 * @tc.name: Stringify_0500
 * @tc.desc: Stringify.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, Stringify_0500, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjson = ptJson.Parse(content);
    EXPECT_TRUE(ptjson != NULL);
    EXPECT_TRUE(!ptjson->Stringify().empty());
}

/*
 * @tc.name: Add_0600
 * @tc.desc: Add.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, Add_0600, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjsonObject = ptJson.CreateObject();
    EXPECT_TRUE(ptjsonObject != NULL);

    EXPECT_TRUE(ptJson.Add("AAA", true));
    EXPECT_TRUE(ptJson.Add("BBB", 123));
    EXPECT_TRUE(ptJson.Add("CCC", 123.5));
    EXPECT_TRUE(ptJson.Add("DDD", "ABC"));
    EXPECT_TRUE(ptJson.Add("EEE", ptjsonObject));
}

/*
 * @tc.name: Push_0700
 * @tc.desc: Push.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, Push_0700, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjsonObject = ptJson.CreateObject();
    EXPECT_TRUE(ptjsonObject != NULL);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjsonArray = ptJson.CreateArray();
    EXPECT_TRUE(ptjsonArray != NULL);

    EXPECT_TRUE(ptJson.Add("AAA", true));
    EXPECT_TRUE(ptJson.Add("BBB", 123));
    EXPECT_TRUE(ptJson.Add("CCC", 123.5));
    EXPECT_TRUE(ptJson.Add("DDD", "ABC"));
    EXPECT_TRUE(ptJson.Add("EEE", ptjsonObject));

    EXPECT_TRUE(ptJson.Push(true));
    EXPECT_TRUE(ptJson.Push(123));
    EXPECT_TRUE(ptJson.Push(123.5));
    EXPECT_TRUE(ptJson.Push("ABC"));
    EXPECT_TRUE(ptJson.Push(ptjsonArray));
}

/*
 * @tc.name: Contains_0800
 * @tc.desc: Contains.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, Contains_0800, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);
    EXPECT_TRUE(ptJson.Add("AAA", true));
    EXPECT_TRUE(ptJson.Remove("AAA"));
}

/*
 * @tc.name: Contains_0900
 * @tc.desc: Contains.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, Contains_0900, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);
    EXPECT_TRUE(ptJson.Add("AAA", true));
    EXPECT_TRUE(ptJson.Contains("AAA"));
}

/*
 * @tc.name: GetKey_1000
 * @tc.desc: GetKey.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, GetKey_1000, Function | MediumTest | Level1)
{
    char a[] = "{\"firstName\":\"Brett\"}";
    cJSON*root = cJSON_Parse(a);
    cJSON*item = cJSON_GetObjectItem(root, "firstName");
    OHOS::AppPackingTool::PtJson ptJson(item);
    EXPECT_TRUE(!ptJson.GetKey().empty());
    EXPECT_TRUE(!ptJson.Stringify().empty());
}

/*
 * @tc.name: GetJson_1100
 * @tc.desc: GetJson.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, GetJson_1100, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    EXPECT_TRUE(ptJson.GetJson() != nullptr);
}

/*
 * @tc.name: IsBool_1200
 * @tc.desc: IsBool.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, IsBool_1200, Function | MediumTest | Level1)
{
    cJSON *node = cJSON_CreateBool(true);
    OHOS::AppPackingTool::PtJson ptJsonBool(node);

    EXPECT_TRUE(ptJsonBool.IsBool());
    EXPECT_TRUE(ptJsonBool.GetBool(true));
}

/*
 * @tc.name: IsNumber_1300
 * @tc.desc: IsNumber.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, IsNumber_1300, Function | MediumTest | Level1)
{
    cJSON *node = cJSON_CreateNumber(12345);
    OHOS::AppPackingTool::PtJson ptJsonNumber(node);

    EXPECT_TRUE(ptJsonNumber.IsNumber());
    EXPECT_EQ(ptJsonNumber.GetInt(12345), 12345);
    EXPECT_EQ(ptJsonNumber.GetInt64(12345), 12345);
    EXPECT_EQ(ptJsonNumber.GetUInt(12345), 12345);
    EXPECT_EQ(ptJsonNumber.GetUInt64(12345), 12345);
    EXPECT_EQ(ptJsonNumber.GetDouble(12345.5), 12345);
}

/*
 * @tc.name: IsString_1400
 * @tc.desc: IsString.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, IsString_1400, Function | MediumTest | Level1)
{
    cJSON *node = cJSON_CreateString("abcd");
    OHOS::AppPackingTool::PtJson ptJsonString(node);

    EXPECT_TRUE(ptJsonString.IsString());
    EXPECT_STREQ(ptJsonString.GetString().c_str(), "abcd");
}

/*
 * @tc.name: IsObject_1500
 * @tc.desc: IsObject.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, IsObject_1500, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjsonObject = ptJson.CreateObject();
    EXPECT_TRUE(ptjsonObject != NULL);
    EXPECT_TRUE(ptjsonObject->IsObject());
}

/*
 * @tc.name: IsArray_1600
 * @tc.desc: IsArray.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, IsArray_1600, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjsonArray = ptJson.CreateArray();
    EXPECT_TRUE(ptjsonArray != NULL);
    EXPECT_TRUE(ptjsonArray->IsArray());
    ptjsonArray->Push("11111");
    ptjsonArray->Push("22222");
    EXPECT_STREQ(ptjsonArray->Get(1)->GetString().c_str(), "22222");
    EXPECT_TRUE(ptjsonArray->GetSize() > 0);
}

/*
 * @tc.name: IsNull_1700
 * @tc.desc: IsNull.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, IsNull_1700, Function | MediumTest | Level1)
{
    cJSON *cjson = cJSON_CreateNull();
    OHOS::AppPackingTool::PtJson ptJson(cjson);
    EXPECT_TRUE(ptJson.IsNull());
}

/*
 * @tc.name: SetBool_1800
 * @tc.desc: SetBool.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, SetBool_1800, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    bool flag = false;
    ptJson.Add("AAA", true);
    EXPECT_EQ(ptJson.SetBool("AAA", flag), OHOS::AppPackingTool::Result::SUCCESS);
    ptJson.GetBool("AAA", &flag);
    EXPECT_FALSE(flag);
}

/*
 * @tc.name: SetInt_1900
 * @tc.desc: SetInt.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, SetInt_1900, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    EXPECT_TRUE(ptJson.Add("BBB", 123));
    int number = 0;
    EXPECT_EQ(ptJson.SetInt("BBB", 321), OHOS::AppPackingTool::Result::SUCCESS);
    ptJson.GetInt("BBB", &number);
    EXPECT_EQ(number, 321);
}

/*
 * @tc.name: SetInt64_2000
 * @tc.desc: SetInt64.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, SetInt64_2000, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    EXPECT_TRUE(ptJson.Add("BBB", 123));
    EXPECT_TRUE(ptJson.Add("CCC", 123.5));

    int64_t value64 = 11111;
    EXPECT_EQ(ptJson.SetInt64("BBB", value64), OHOS::AppPackingTool::Result::SUCCESS);
    int64_t value642;
    EXPECT_EQ(ptJson.GetInt64("BBB", &value642), OHOS::AppPackingTool::Result::SUCCESS);
    EXPECT_EQ(value642, 11111);

    uint32_t value32 = 2222;
    EXPECT_EQ(ptJson.SetUInt("BBB", value32), OHOS::AppPackingTool::Result::SUCCESS);
    uint32_t value322;
    EXPECT_EQ(ptJson.GetUInt("BBB", &value322), OHOS::AppPackingTool::Result::SUCCESS);
    EXPECT_EQ(value322, 2222);

    uint64_t valueInt64 = 3333;
    EXPECT_EQ(ptJson.SetUInt64("BBB", valueInt64), OHOS::AppPackingTool::Result::SUCCESS);
    uint64_t valueInt642;
    EXPECT_EQ(ptJson.GetUInt64("BBB", &valueInt642), OHOS::AppPackingTool::Result::SUCCESS);
    EXPECT_EQ(valueInt642, 3333);

    double valueDouble = 4444;
    EXPECT_EQ(ptJson.SetDouble("CCC", valueDouble), OHOS::AppPackingTool::Result::SUCCESS);
    double valueDouble2;
    EXPECT_EQ(ptJson.GetDouble("CCC", &valueDouble2), OHOS::AppPackingTool::Result::SUCCESS);
    EXPECT_EQ(valueDouble2, 4444);
}

/*
 * @tc.name: SetString_2100
 * @tc.desc: SetString.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, SetString_2100, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    EXPECT_TRUE(ptJson.Add("DDD", "ABC"));
    std::string str("1234567890");
    EXPECT_EQ(ptJson.SetString("DDD", str), OHOS::AppPackingTool::Result::SUCCESS);
    std::string str2;
    EXPECT_EQ(ptJson.GetString("DDD", &str2), OHOS::AppPackingTool::Result::SUCCESS);
    EXPECT_STREQ(str2.c_str(), str.c_str());
}

/*
 * @tc.name: GetObject_2200
 * @tc.desc: GetObject.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, GetObject_2200, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    std::unique_ptr<OHOS::AppPackingTool::PtJson> ptjsonObject = ptJson.CreateObject();
    EXPECT_TRUE(ptjsonObject != NULL);
    EXPECT_TRUE(ptJson.Add("EEE", ptjsonObject));
    std::unique_ptr<OHOS::AppPackingTool::PtJson> Object;
    EXPECT_EQ(ptJson.GetObject("EEE", &Object), OHOS::AppPackingTool::Result::SUCCESS);
}

/*
 * @tc.name: GetArray_2300
 * @tc.desc: GetArray.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, GetArray_2300, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    ptJson.Add("FFF", ptJson.CreateArray());
    std::unique_ptr<OHOS::AppPackingTool::PtJson> Array;
    EXPECT_EQ(ptJson.GetArray("FFF", &Array), OHOS::AppPackingTool::Result::SUCCESS);
}

/*
 * @tc.name: GetAny_2400
 * @tc.desc: GetAny.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PtJsonTest, GetAny_2400, Function | MediumTest | Level1)
{
    cJSON *cjson = new cJSON();
    OHOS::AppPackingTool::PtJson ptJson(cjson);

    ptJson.Add("FFF", ptJson.CreateArray());
    std::unique_ptr<OHOS::AppPackingTool::PtJson> Object;
    EXPECT_EQ(ptJson.GetAny("FFF", &Object), OHOS::AppPackingTool::Result::SUCCESS);
}
} // namespace OHOS