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
#include <string>

#include "constants.h"
#define private public
#define protected public
#include "packager.h"
#include "multiapp_packager.h"
#include "zip_wrapper.h"
#include "log.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace {
const std::string OUT_PATH = "/data/test_1.app";
const std::string OUT_PATH_WITHOUT_SUFFIX = "/data/test_1";
const std::string OUT_PATH_WITHOUT_APP_SUFFIX = "/data/test_1.hap";
const std::string HAP_LIST = "/data/test/resource/packingtool/test_file/multiApp/hap/multiappPackagerHapTest.hap";
const std::string HSP_LIST = "/data/test/resource/packingtool/test_file/multiApp/hsp/multiappPackagerHspTest.hsp";
}

class MultiAppPackagerTest : public testing::Test {
public:
    MultiAppPackagerTest() {}
    virtual ~MultiAppPackagerTest() {}

    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void MultiAppPackagerTest::SetUpTestCase()
{}

void MultiAppPackagerTest::TearDownTestCase()
{}

void MultiAppPackagerTest::SetUp()
{}

void MultiAppPackagerTest::TearDown()
{}

/*
 * @tc.name: MultiAppPackager_0100
 * @tc.desc: InitAllowedParam.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, MultiAppPackager_0100, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_OUT_PATH, OUT_PATH},
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
        {OHOS::AppPackingTool::Constants::PARAM_HSP_LIST, HSP_LIST},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.InitAllowedParam(), 0);
    EXPECT_EQ(multiAppPackager.PreProcess(), 0);
    EXPECT_EQ(multiAppPackager.Process(), 0);
    EXPECT_EQ(multiAppPackager.PostProcess(), 0);
}

/*
 * @tc.name: PreProcess_0100
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_0100, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, ""},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 1);
}

/*
 * @tc.name: PreProcess_0200
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_0200, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 1);
}

/*
 * @tc.name: PreProcess_0300
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_0300, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 0);
}

/*
 * @tc.name: PreProcess_0400
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_0400, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_APP_LIST, ""},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 1);
}

/*
 * @tc.name: PreProcess_0500
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_0500, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 0);
}

/*
 * @tc.name: PreProcess_0600
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_0600, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, ""},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 1);
}

/*
 * @tc.name: PreProcess_0700
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_0700, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
        {OHOS::AppPackingTool::Constants::PARAM_HSP_LIST, ""},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 0);
}

/*
 * @tc.name: PreProcess_0800
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_0800, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
        {OHOS::AppPackingTool::Constants::PARAM_HSP_LIST, HSP_LIST},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 0);
}

/*
 * @tc.name: PreProcess_0900
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_0900, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_OUT_PATH, ""},
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
        {OHOS::AppPackingTool::Constants::PARAM_HSP_LIST, HSP_LIST},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 0);
}

/*
 * @tc.name: PreProcess_1000
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_1000, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_OUT_PATH, OUT_PATH},
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "false"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
        {OHOS::AppPackingTool::Constants::PARAM_HSP_LIST, HSP_LIST},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 1);
}

/*
 * @tc.name: PreProcess_1100
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_1100, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_OUT_PATH, ""},
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "false"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
        {OHOS::AppPackingTool::Constants::PARAM_HSP_LIST, HSP_LIST},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 0);
}

/*
 * @tc.name: PreProcess_1200
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_1200, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_OUT_PATH, OUT_PATH_WITHOUT_SUFFIX},
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
        {OHOS::AppPackingTool::Constants::PARAM_HSP_LIST, HSP_LIST},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 1);
}

/*
 * @tc.name: PreProcess_1300
 * @tc.desc: test pre process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, PreProcess_1300, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_OUT_PATH, OUT_PATH_WITHOUT_APP_SUFFIX},
        {OHOS::AppPackingTool::Constants::PARAM_FORCE, "true"},
        {OHOS::AppPackingTool::Constants::PARAM_HAP_LIST, HAP_LIST},
        {OHOS::AppPackingTool::Constants::PARAM_HSP_LIST, HSP_LIST},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.PreProcess(), 1);
}

/*
 * @tc.name: Process_0100
 * @tc.desc: test process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, Process_0100, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_OUT_PATH, ""},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.Process(), 1);
}

/*
 * @tc.name: Process_0200
 * @tc.desc: test process.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MultiAppPackagerTest, Process_0200, Function | MediumTest | Level1)
{
    std::string resultReceiver;
    std::map<std::string, std::string> parameterMap = {
        {OHOS::AppPackingTool::Constants::PARAM_OUT_PATH, OUT_PATH},
    };

    OHOS::AppPackingTool::MultiAppPackager multiAppPackager(parameterMap, resultReceiver);
    EXPECT_EQ(multiAppPackager.Process(), 1);
}
} // namespace OHOS