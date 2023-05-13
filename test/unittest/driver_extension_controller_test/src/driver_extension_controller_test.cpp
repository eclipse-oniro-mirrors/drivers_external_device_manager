/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "driver_extension_controller.h"
#include "iostream"
#include "hilog_wrapper.h"
#include "string_ex.h"
#include "iservice_registry.h"
#include "accesstoken_kit.h"
#include "token_setproc.h"
#include "nativetoken_kit.h"
#include "gtest/gtest.h"
#define USB_SYSTEM_ABILITY_ID 180
using namespace OHOS;
using namespace std;
using namespace testing::ext;
class DrvExtCtrlTest: public testing::Test {
public:
    void SetUp() override {
        cout << "DrvExtCtrlTest SetUp" << endl;
    }
    void TearDown() override {
        cout << "DrvExtCtrlTest TearDown" << endl;
    }
};


#define TEST_BUNDLE_NAME "com.huawei.ohos.screenshot"

#define TEST_ABILITY_NAME "com.huawei.ohos.screenshot.ServiceExtAbility"
#define TEST_ABILITY_NAME_ERR "com.huawei.ohos.screenshot.XXX"

using namespace OHOS::Security::AccessToken;

TokenInfoParams g_normalInfoInstance = {
    .dcapsNum = 0,
    .permsNum = 0,
    .aclsNum = 0,
    .dcaps = nullptr,
    .perms = nullptr,
    .acls = nullptr,
    .processName = "usb_manager",
    .aplStr = "normal",
};

TokenInfoParams g_sysInfoInstance = {
    .dcapsNum = 0,
    .permsNum = 0,
    .aclsNum = 0,
    .dcaps = nullptr,
    .perms = nullptr,
    .acls = nullptr,
    .processName = "usb_manager",
    .aplStr = "system_basic",
};

class AccessTokenTest
{
public:
    static void SetTestCaseNative (TokenInfoParams *infoInstance)
    {
        uint64_t tokenId = GetAccessTokenId(infoInstance);
        int ret = SetSelfTokenID(tokenId);
        if (ret == 0) {
            DEVMGR_LOGI("SetSelfTokenID success");
        } else {
            DEVMGR_LOGE("SetSelfTokenID fail");
        }
        AccessTokenKit::ReloadNativeTokenInfo();
    }
};
HWTEST_F(DrvExtCtrlTest, DrvExtCtrlWithSATest, TestSize.Level1)
{
    AccessTokenTest::SetTestCaseNative(&g_sysInfoInstance);
    int ret = 0;
    ret = DriverExtensionController::StartDriverExtension(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    ASSERT_EQ(ret, 0);
    ret = DriverExtensionController::StopDriverExtension(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(DrvExtCtrlTest, DrvExtCtrlWithoutSATest, TestSize.Level1)
{
    AccessTokenTest::SetTestCaseNative(&g_normalInfoInstance);
    int ret = 0;
    ret = DriverExtensionController::StartDriverExtension(TEST_BUNDLE_NAME, TEST_ABILITY_NAME_ERR);
    ASSERT_NE(ret, 0);
    ret = DriverExtensionController::StopDriverExtension(TEST_BUNDLE_NAME, TEST_ABILITY_NAME_ERR);
    ASSERT_NE(ret, 0);
}
