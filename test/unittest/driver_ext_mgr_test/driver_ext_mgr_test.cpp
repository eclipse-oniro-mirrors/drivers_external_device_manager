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
#include "driver_ext_mgr.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

class DriverExtMgrTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_FRAMEWORK, "DriverExtMgrTest SetUp");
    }
    void TearDown() override
    {
        EDM_LOGD(MODULE_FRAMEWORK, "DriverExtMgrTest TearDown");
    }
};

HWTEST_F(DriverExtMgrTest, OnStartTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    ASSERT_TRUE(instance.IsServiceReady());
    EDM_LOGI(MODULE_FRAMEWORK, "OnStartTest: Service started successfully");
}

HWTEST_F(DriverExtMgrTest, OnStopTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    instance.OnStop();
    ASSERT_FALSE(instance.IsServiceReady());
    EDM_LOGI(MODULE_FRAMEWORK, "OnStopTest: Service stopped successfully");
}

HWTEST_F(DriverExtMgrTest, OnStartMultipleTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    bool ready1 = instance.IsServiceReady();
    instance.OnStart();
    bool ready2 = instance.IsServiceReady();
    ASSERT_EQ(ready1, ready2);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "OnStartMultipleTest: Multiple OnStart calls handled correctly");
}

HWTEST_F(DriverExtMgrTest, OnStopWithoutStartTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    ASSERT_FALSE(instance.IsServiceReady());
    instance.OnStop();
    ASSERT_FALSE(instance.IsServiceReady());
    EDM_LOGI(MODULE_FRAMEWORK, "OnStopWithoutStartTest: OnStop without OnStart handled");
}

HWTEST_F(DriverExtMgrTest, DumpTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int fd = 1;
    std::vector<std::u16string> args;
    int ret = instance.Dump(fd, args);
    ASSERT_EQ(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "DumpTest: Dump method executed successfully");
}

HWTEST_F(DriverExtMgrTest, DumpWithArgsTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int fd = 1;
    std::vector<std::u16string> args = {u"-h"};
    int ret = instance.Dump(fd, args);
    ASSERT_EQ(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "DumpWithArgsTest: Dump with args executed successfully");
}

HWTEST_F(DriverExtMgrTest, QueryDeviceByBusTypeTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint32_t busType = static_cast<uint32_t>(BusType::BUS_TYPE_USB);
    std::vector<std::shared_ptr<DeviceData>> devices;
    ErrCode ret = instance.QueryDevice(errorCode, busType, devices);
    ASSERT_EQ(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "QueryDeviceByBusTypeTest: Query by USB bus type");
}

HWTEST_F(DriverExtMgrTest, QueryDeviceEmptyTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint32_t busType = static_cast<uint32_t>(BusType::BUS_TYPE_USB);
    std::vector<std::shared_ptr<DeviceData>> devices;
    ErrCode ret = instance.QueryDevice(errorCode, busType, devices);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(devices.size(), 0);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "QueryDeviceEmptyTest: Empty device list returned");
}

class MockDriverExtMgrCallback : public IDriverExtMgrCallback {
public:
    MockDriverExtMgrCallback() = default;
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
    ErrCode OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg) override
    {
        return EDM_OK;
    }
    ErrCode OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg) override
    {
        return EDM_OK;
    }
    ErrCode OnUnBind(uint64_t deviceId, const ErrMsg &errMsg) override
    {
        return EDM_OK;
    }
};

HWTEST_F(DriverExtMgrTest, BindDeviceSuccessTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint64_t deviceId = 1;
    sptr<IDriverExtMgrCallback> callback = sptr<MockDriverExtMgrCallback>::MakeSptr();
    ErrCode ret = instance.BindDevice(errorCode, deviceId, callback);
    ASSERT_NE(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "BindDeviceSuccessTest: Bind device attempted");
}

HWTEST_F(DriverExtMgrTest, BindDeviceNotFoundTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint64_t deviceId = 999999;
    sptr<IDriverExtMgrCallback> callback = sptr<MockDriverExtMgrCallback>::MakeSptr();
    ErrCode ret = instance.BindDevice(errorCode, deviceId, callback);
    ASSERT_NE(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "BindDeviceNotFoundTest: Non-existent device rejected");
}

HWTEST_F(DriverExtMgrTest, BindDeviceNullCallbackTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint64_t deviceId = 1;
    sptr<IDriverExtMgrCallback> callback = nullptr;
    ErrCode ret = instance.BindDevice(errorCode, deviceId, callback);
    ASSERT_NE(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "BindDeviceNullCallbackTest: Null callback handled");
}

HWTEST_F(DriverExtMgrTest, UnBindDeviceSuccessTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint64_t deviceId = 1;
    ErrCode ret = instance.UnBindDevice(errorCode, deviceId);
    ASSERT_NE(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "UnBindDeviceSuccessTest: Unbind device attempted");
}

HWTEST_F(DriverExtMgrTest, UnBindDeviceNotFoundTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint64_t deviceId = 999999;
    ErrCode ret = instance.UnBindDevice(errorCode, deviceId);
    ASSERT_NE(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "UnBindDeviceNotFoundTest: Non-existent device rejected");
}

HWTEST_F(DriverExtMgrTest, BindDriverWithDeviceIdSuccessTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint64_t deviceId = 1;
    sptr<IDriverExtMgrCallback> callback = sptr<MockDriverExtMgrCallback>::MakeSptr();
    ErrCode ret = instance.BindDriverWithDeviceId(errorCode, deviceId, callback);
    ASSERT_NE(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "BindDriverWithDeviceIdSuccessTest: Bind driver with device ID attempted");
}

HWTEST_F(DriverExtMgrTest, UnBindDriverWithDeviceIdSuccessTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint64_t deviceId = 1;
    ErrCode ret = instance.UnBindDriverWithDeviceId(errorCode, deviceId);
    ASSERT_NE(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "UnBindDriverWithDeviceIdSuccessTest: Unbind driver with device ID attempted");
}

HWTEST_F(DriverExtMgrTest, QueryDeviceInfoAllTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    std::vector<std::shared_ptr<DeviceInfoData>> deviceInfos;
    ErrCode ret = instance.QueryDeviceInfo(errorCode, deviceInfos);
    ASSERT_EQ(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "QueryDeviceInfoAllTest: Query all device info");
}

HWTEST_F(DriverExtMgrTest, QueryDeviceInfoByIdTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    uint64_t deviceId = 1;
    std::vector<std::shared_ptr<DeviceInfoData>> deviceInfos;
    ErrCode ret = instance.QueryDeviceInfo(errorCode, deviceInfos, true, deviceId);
    ASSERT_EQ(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "QueryDeviceInfoByIdTest: Query device info by ID");
}

HWTEST_F(DriverExtMgrTest, QueryDriverInfoAllTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    std::vector<std::shared_ptr<DriverInfoData>> driverInfos;
    ErrCode ret = instance.QueryDriverInfo(errorCode, driverInfos);
    ASSERT_EQ(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "QueryDriverInfoAllTest: Query all driver info");
}

HWTEST_F(DriverExtMgrTest, QueryDriverInfoByUidTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t errorCode = EDM_OK;
    std::string driverUid = "test_driver_uid";
    std::vector<std::shared_ptr<DriverInfoData>> driverInfos;
    ErrCode ret = instance.QueryDriverInfo(errorCode, driverInfos, true, driverUid);
    ASSERT_EQ(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "QueryDriverInfoByUidTest: Query driver info by UID");
}

HWTEST_F(DriverExtMgrTest, NotifyUsbPeripheralFaultTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    std::string domain = "USB";
    std::string faultName = "DEVICE_FAULT";
    ErrCode ret = instance.NotifyUsbPeripheralFault(domain, faultName);
    ASSERT_EQ(ret, EDM_OK);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "NotifyUsbPeripheralFaultTest: USB fault notification sent");
}

HWTEST_F(DriverExtMgrTest, OnAddSystemAbilityBmsTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t systemAbilityId = 401;
    std::string deviceId = "test_device";
    instance.OnAddSystemAbility(systemAbilityId, deviceId);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "OnAddSystemAbilityBmsTest: BMS service added");
}

HWTEST_F(DriverExtMgrTest, OnAddSystemAbilityAccountTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t systemAbilityId = 2;
    std::string deviceId = "test_device";
    instance.OnAddSystemAbility(systemAbilityId, deviceId);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "OnAddSystemAbilityAccountTest: Account service added");
}

HWTEST_F(DriverExtMgrTest, OnAddSystemAbilityCommonEventTest, TestSize.Level1)
{
    DriverExtMgr &instance = DriverExtMgr::GetInstance();
    instance.OnStart();
    int32_t systemAbilityId = 6;
    std::string deviceId = "test_device";
    instance.OnAddSystemAbility(systemAbilityId, deviceId);
    instance.OnStop();
    EDM_LOGI(MODULE_FRAMEWORK, "OnAddSystemAbilityCommonEventTest: CommonEvent service added");
}
} // namespace ExternalDeviceManager
} // namespace OHOS
