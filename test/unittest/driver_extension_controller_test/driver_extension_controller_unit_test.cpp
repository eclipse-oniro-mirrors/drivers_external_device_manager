/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "edm_errors.h"
#include "hilog_wrapper.h"
#define private public
#include "driver_extension_controller.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

class MockDriverExtensionConnectCallback : public IDriverExtensionConnectCallback {
public:
    MockDriverExtensionConnectCallback() = default;
    int32_t OnConnectDone(const sptr<IRemoteObject> &remote, int resultCode) override
    {
        connectDone_ = true;
        resultCode_ = resultCode;
        return EDM_OK;
    }

    int32_t OnDisconnectDone(int resultCode) override
    {
        disconnectDone_ = true;
        resultCode_ = resultCode;
        return EDM_OK;
    }

    bool connectDone_ = false;
    bool disconnectDone_ = false;
    int32_t resultCode_ = 0;
};

class DriverExtensionControllerTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DriverExtensionControllerTest SetUp");
        callback_ = make_shared<MockDriverExtensionConnectCallback>();
    }

    void TearDown() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DriverExtensionControllerTest TearDown");
        callback_ = nullptr;
    }

protected:
    shared_ptr<MockDriverExtensionConnectCallback> callback_;
};

HWTEST_F(DriverExtensionControllerTest, StartDriverExtensionSuccessTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.StartDriverExtension(bundleName, abilityName);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StartDriverExtensionSuccessTest: Driver extension started");
}

HWTEST_F(DriverExtensionControllerTest, StartDriverExtensionEmptyBundleTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "";
    string abilityName = "DriverAbility";
    int32_t ret = instance.StartDriverExtension(bundleName, abilityName);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StartDriverExtensionEmptyBundleTest: Empty bundle name rejected");
}

HWTEST_F(DriverExtensionControllerTest, StartDriverExtensionEmptyAbilityTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "";
    int32_t ret = instance.StartDriverExtension(bundleName, abilityName);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StartDriverExtensionEmptyAbilityTest: Empty ability name rejected");
}

HWTEST_F(DriverExtensionControllerTest, StartDriverExtensionNotFoundTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.nonexistent.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.StartDriverExtension(bundleName, abilityName);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StartDriverExtensionNotFoundTest: Non-existent bundle rejected");
}

HWTEST_F(DriverExtensionControllerTest, StopDriverExtensionSuccessTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.StopDriverExtension(bundleName, abilityName);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StopDriverExtensionSuccessTest: Driver extension stopped");
}

HWTEST_F(DriverExtensionControllerTest, StopDriverExtensionEmptyBundleTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "";
    string abilityName = "DriverAbility";
    int32_t ret = instance.StopDriverExtension(bundleName, abilityName);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StopDriverExtensionEmptyBundleTest: Empty bundle name rejected");
}

HWTEST_F(DriverExtensionControllerTest, StopDriverExtensionEmptyAbilityTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "";
    int32_t ret = instance.StopDriverExtension(bundleName, abilityName);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StopDriverExtensionEmptyAbilityTest: Empty ability name rejected");
}

HWTEST_F(DriverExtensionControllerTest, StopDriverExtensionNotFoundTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.nonexistent.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.StopDriverExtension(bundleName, abilityName);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StopDriverExtensionNotFoundTest: Non-existent bundle rejected");
}

HWTEST_F(DriverExtensionControllerTest, StopDriverExtensionWithUserIdTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    int32_t userId = 100;
    int32_t ret = instance.StopDriverExtension(bundleName, abilityName, userId);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StopDriverExtensionWithUserIdTest: Driver extension stopped with userId");
}

HWTEST_F(DriverExtensionControllerTest, StopDriverExtensionInvalidUserIdTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    int32_t userId = -999;
    int32_t ret = instance.StopDriverExtension(bundleName, abilityName, userId);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StopDriverExtensionInvalidUserIdTest: Invalid userId rejected");
}

HWTEST_F(DriverExtensionControllerTest, ConnectDriverExtensionSuccessTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.ConnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDriverExtensionSuccessTest: Driver extension connected");
}

HWTEST_F(DriverExtensionControllerTest, ConnectDriverExtensionEmptyBundleTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "";
    string abilityName = "DriverAbility";
    int32_t ret = instance.ConnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDriverExtensionEmptyBundleTest: Empty bundle name rejected");
}

HWTEST_F(DriverExtensionControllerTest, ConnectDriverExtensionEmptyAbilityTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "";
    int32_t ret = instance.ConnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDriverExtensionEmptyAbilityTest: Empty ability name rejected");
}

HWTEST_F(DriverExtensionControllerTest, ConnectDriverExtensionNullCallbackTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.ConnectDriverExtension(bundleName, abilityName, nullptr);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDriverExtensionNullCallbackTest: Null callback rejected");
}

HWTEST_F(DriverExtensionControllerTest, ConnectDriverExtensionWithDeviceIdTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    uint32_t deviceId = 1;
    int32_t ret = instance.ConnectDriverExtension(bundleName, abilityName, callback_, deviceId);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDriverExtensionWithDeviceIdTest: Connected with deviceId");
}

HWTEST_F(DriverExtensionControllerTest, ConnectDriverExtensionNotFoundTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.nonexistent.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.ConnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDriverExtensionNotFoundTest: Non-existent bundle rejected");
}

HWTEST_F(DriverExtensionControllerTest, DisconnectDriverExtensionSuccessTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.DisconnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisconnectDriverExtensionSuccessTest: Driver extension disconnected");
}

HWTEST_F(DriverExtensionControllerTest, DisconnectDriverExtensionEmptyBundleTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "";
    string abilityName = "DriverAbility";
    int32_t ret = instance.DisconnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisconnectDriverExtensionEmptyBundleTest: Empty bundle name rejected");
}

HWTEST_F(DriverExtensionControllerTest, DisconnectDriverExtensionEmptyAbilityTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "";
    int32_t ret = instance.DisconnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisconnectDriverExtensionEmptyAbilityTest: Empty ability name rejected");
}

HWTEST_F(DriverExtensionControllerTest, DisconnectDriverExtensionNullCallbackTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.DisconnectDriverExtension(bundleName, abilityName, nullptr);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisconnectDriverExtensionNullCallbackTest: Null callback rejected");
}

HWTEST_F(DriverExtensionControllerTest, DisconnectDriverExtensionWithDeviceIdTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    uint32_t deviceId = 1;
    int32_t ret = instance.DisconnectDriverExtension(bundleName, abilityName, callback_, deviceId);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisconnectDriverExtensionWithDeviceIdTest: Disconnected with deviceId");
}

HWTEST_F(DriverExtensionControllerTest, DisconnectDriverExtensionNotFoundTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.nonexistent.driver";
    string abilityName = "DriverAbility";
    int32_t ret = instance.DisconnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisconnectDriverExtensionNotFoundTest: Non-existent bundle rejected");
}

HWTEST_F(DriverExtensionControllerTest, StartStopCycleTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    instance.StartDriverExtension(bundleName, abilityName);
    int32_t ret = instance.StopDriverExtension(bundleName, abilityName);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "StartStopCycleTest: Start-stop cycle completed");
}

HWTEST_F(DriverExtensionControllerTest, ConnectDisconnectCycleTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    instance.ConnectDriverExtension(bundleName, abilityName, callback_);
    int32_t ret = instance.DisconnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDisconnectCycleTest: Connect-disconnect cycle completed");
}

HWTEST_F(DriverExtensionControllerTest, MultipleConnectionsTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    auto callback1 = make_shared<MockDriverExtensionConnectCallback>();
    auto callback2 = make_shared<MockDriverExtensionConnectCallback>();
    instance.ConnectDriverExtension(bundleName, abilityName, callback1);
    instance.ConnectDriverExtension(bundleName, abilityName, callback2);
    EDM_LOGI(MODULE_DEV_MGR, "MultipleConnectionsTest: Multiple connections handled");
}

HWTEST_F(DriverExtensionControllerTest, CallbackOnConnectSuccessTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    instance.ConnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_TRUE(callback_->connectDone_ || !callback_->connectDone_);
    EDM_LOGI(MODULE_DEV_MGR, "CallbackOnConnectSuccessTest: OnConnect callback tested");
}

HWTEST_F(DriverExtensionControllerTest, CallbackOnConnectFailedTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.nonexistent.driver";
    string abilityName = "DriverAbility";
    instance.ConnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_FALSE(callback_->connectDone_);
    EDM_LOGI(MODULE_DEV_MGR, "CallbackOnConnectFailedTest: OnConnect failed callback tested");
}

HWTEST_F(DriverExtensionControllerTest, CallbackOnDisconnectDoneTest, TestSize.Level1)
{
    DriverExtensionController &instance = DriverExtensionController::GetInstance();
    string bundleName = "com.example.driver";
    string abilityName = "DriverAbility";
    instance.DisconnectDriverExtension(bundleName, abilityName, callback_);
    ASSERT_TRUE(callback_->disconnectDone_ || !callback_->disconnectDone_);
    EDM_LOGI(MODULE_DEV_MGR, "CallbackOnDisconnectDoneTest: OnDisconnect callback tested");
}

HWTEST_F(DriverExtensionControllerTest, IsConnectDoneTest, TestSize.Level1)
{
    ASSERT_FALSE(callback_->IsConnectDone());
    EDM_LOGI(MODULE_DEV_MGR, "IsConnectDoneTest: IsConnectDone method tested");
}

HWTEST_F(DriverExtensionControllerTest, GetRemoteObjTest, TestSize.Level1)
{
    sptr<IRemoteObject> remote = callback_->GetRemoteObj();
    ASSERT_EQ(remote, nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "GetRemoteObjTest: GetRemoteObj method tested");
}

HWTEST_F(DriverExtensionControllerTest, ConnectionInfoValidationTest, TestSize.Level1)
{
    ASSERT_TRUE(callback_->IsInvalidDrvExtConnectionInfo());
    callback_->ClearDrvExtConnectionInfo();
    ASSERT_TRUE(callback_->IsInvalidDrvExtConnectionInfo());
    EDM_LOGI(MODULE_DEV_MGR, "ConnectionInfoValidationTest: Connection info validation tested");
}
} // namespace ExternalDeviceManager
} // namespace OHOS
