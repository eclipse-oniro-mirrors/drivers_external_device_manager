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
#include "device.h"
#include "usb_device_info.h"
#include "usb_driver_info.h"
#include "driver_info.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

class DeviceTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DeviceTest SetUp");
        deviceInfo_ = make_shared<DeviceInfo>(1, BusType::BUS_TYPE_USB, "test_device");
        device_ = make_shared<Device>(deviceInfo_);
    }

    void TearDown() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DeviceTest TearDown");
        device_ = nullptr;
        deviceInfo_ = nullptr;
    }

protected:
    shared_ptr<Device> device_;
    shared_ptr<DeviceInfo> deviceInfo_;
};

HWTEST_F(DeviceTest, DeviceConstructorTest, TestSize.Level1)
{
    ASSERT_NE(device_, nullptr);
    ASSERT_NE(device_->GetDeviceInfo(), nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "DeviceConstructorTest: Device constructed successfully");
}

HWTEST_F(DeviceTest, HasDriverTrueTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    device_->AddBundleInfo("testBundle", driverInfo);
    ASSERT_TRUE(device_->HasDriver());
    EDM_LOGI(MODULE_DEV_MGR, "HasDriverTrueTest: Device has driver");
}

HWTEST_F(DeviceTest, HasDriverFalseTest, TestSize.Level1)
{
    ASSERT_FALSE(device_->HasDriver());
    EDM_LOGI(MODULE_DEV_MGR, "HasDriverFalseTest: Device has no driver");
}

HWTEST_F(DeviceTest, GetDriverInfoTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    device_->AddBundleInfo("testBundle", driverInfo);
    ASSERT_NE(device_->GetDriverInfo(), nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "GetDriverInfoTest: Got driver info");
}

HWTEST_F(DeviceTest, GetDeviceInfoTest, TestSize.Level1)
{
    ASSERT_NE(device_->GetDeviceInfo(), nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "GetDeviceInfoTest: Got device info");
}

HWTEST_F(DeviceTest, AddBundleInfoTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    device_->AddBundleInfo("testBundle", driverInfo);
    ASSERT_EQ(device_->GetBundleInfo(), "testBundle");
    ASSERT_NE(device_->GetDriverInfo(), nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "AddBundleInfoTest: Bundle info added");
}

HWTEST_F(DeviceTest, RemoveBundleInfoTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    device_->AddBundleInfo("testBundle", driverInfo);
    device_->RemoveBundleInfo();
    ASSERT_TRUE(device_->GetBundleInfo().empty());
    EDM_LOGI(MODULE_DEV_MGR, "RemoveBundleInfoTest: Bundle info removed");
}

HWTEST_F(DeviceTest, GetDriverUidTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver", "testUid", 100);
    device_->AddBundleInfo("testBundle", driverInfo);
    ASSERT_EQ(device_->GetDriverUid(), "testUid");
    EDM_LOGI(MODULE_DEV_MGR, "GetDriverUidTest: Got driver UID");
}

HWTEST_F(DeviceTest, GetBundleInfoTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    device_->AddBundleInfo("testBundle", driverInfo);
    ASSERT_EQ(device_->GetBundleInfo(), "testBundle");
    EDM_LOGI(MODULE_DEV_MGR, "GetBundleInfoTest: Got bundle info");
}

HWTEST_F(DeviceTest, GetStichingTest, TestSize.Level1)
{
    string stiching = Device::GetStiching();
    ASSERT_FALSE(stiching.empty());
    EDM_LOGI(MODULE_DEV_MGR, "GetStichingTest: Got stitching string");
}

class TestRemoteObject : public IRemoteObject {
public:
    TestRemoteObject() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; }
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; }
};

HWTEST_F(DeviceTest, GetDrvExtRemoteTest, TestSize.Level1)
{
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device_->UpdateDrvExtRemote(remote);
    ASSERT_NE(device_->GetDrvExtRemote(), nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "GetDrvExtRemoteTest: Got driver extension remote object");
}

HWTEST_F(DeviceTest, UpdateDrvExtRemoteTest, TestSize.Level1)
{
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device_->UpdateDrvExtRemote(remote);
    ASSERT_EQ(device_->GetDrvExtRemote(), remote);
    EDM_LOGI(MODULE_DEV_MGR, "UpdateDrvExtRemoteTest: Driver extension remote updated");
}

HWTEST_F(DeviceTest, ClearDrvExtRemoteTest, TestSize.Level1)
{
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device_->UpdateDrvExtRemote(remote);
    device_->ClearDrvExtRemote();
    ASSERT_EQ(device_->GetDrvExtRemote(), nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "ClearDrvExtRemoteTest: Driver extension remote cleared");
}

HWTEST_F(DeviceTest, AddDrvExtConnNotifyTest, TestSize.Level1)
{
    device_->AddDrvExtConnNotify();
    ASSERT_NE(device_->connectNofitier_, nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "AddDrvExtConnNotifyTest: Connection notify added");
}

HWTEST_F(DeviceTest, RemoveDrvExtConnNotifyTest, TestSize.Level1)
{
    device_->AddDrvExtConnNotify();
    device_->RemoveDrvExtConnNotify();
    ASSERT_EQ(device_->connectNofitier_, nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "RemoveDrvExtConnNotifyTest: Connection notify removed");
}

HWTEST_F(DeviceTest, IsUnRegistedTrueTest, TestSize.Level1)
{
    device_->UnRegist();
    ASSERT_TRUE(device_->IsUnRegisted());
    EDM_LOGI(MODULE_DEV_MGR, "IsUnRegistedTrueTest: Device is unregistered");
}

HWTEST_F(DeviceTest, IsUnRegistedFalseTest, TestSize.Level1)
{
    ASSERT_FALSE(device_->IsUnRegisted());
    EDM_LOGI(MODULE_DEV_MGR, "IsUnRegistedFalseTest: Device is not unregistered");
}

HWTEST_F(DeviceTest, UnRegistTest, TestSize.Level1)
{
    device_->UnRegist();
    ASSERT_TRUE(device_->IsUnRegisted());
    EDM_LOGI(MODULE_DEV_MGR, "UnRegistTest: Device unregistered");
}

HWTEST_F(DeviceTest, RemoveCallerTest, TestSize.Level1)
{
    uint32_t tokenId = 100;
    CallerInfo info;
    info.isBound = true;
    device_->boundCallerInfos_[tokenId] = info;
    device_->RemoveCaller(tokenId);
    ASSERT_EQ(device_->boundCallerInfos_.find(tokenId), device_->boundCallerInfos_.end());
    EDM_LOGI(MODULE_DEV_MGR, "RemoveCallerTest: Caller removed");
}

HWTEST_F(DeviceTest, ClearBoundCallerInfosTest, TestSize.Level1)
{
    CallerInfo info;
    info.isBound = true;
    device_->boundCallerInfos_[100] = info;
    device_->boundCallerInfos_[200] = info;
    device_->ClearBoundCallerInfos();
    ASSERT_EQ(device_->boundCallerInfos_.size(), 0);
    EDM_LOGI(MODULE_DEV_MGR, "ClearBoundCallerInfosTest: All callers cleared");
}

HWTEST_F(DeviceTest, IsLastCallerTrueTest, TestSize.Level1)
{
    uint32_t tokenId = 100;
    CallerInfo info;
    info.isBound = true;
    device_->boundCallerInfos_[tokenId] = info;
    ASSERT_TRUE(device_->IsLastCaller(tokenId));
    EDM_LOGI(MODULE_DEV_MGR, "IsLastCallerTrueTest: Caller is the last one");
}

HWTEST_F(DeviceTest, IsLastCallerFalseTest, TestSize.Level1)
{
    CallerInfo info;
    info.isBound = true;
    device_->boundCallerInfos_[100] = info;
    device_->boundCallerInfos_[200] = info;
    ASSERT_FALSE(device_->IsLastCaller(100));
    EDM_LOGI(MODULE_DEV_MGR, "IsLastCallerFalseTest: Caller is not the last one");
}

HWTEST_F(DeviceTest, IsBindCallerTrueTest, TestSize.Level1)
{
    uint32_t tokenId = 100;
    CallerInfo info;
    info.isBound = true;
    device_->boundCallerInfos_[tokenId] = info;
    ASSERT_TRUE(device_->IsBindCaller(tokenId));
    EDM_LOGI(MODULE_DEV_MGR, "IsBindCallerTrueTest: Caller is bound");
}

HWTEST_F(DeviceTest, IsBindCallerFalseTest, TestSize.Level1)
{
    uint32_t tokenId = 100;
    CallerInfo info;
    info.isBound = false;
    device_->boundCallerInfos_[tokenId] = info;
    ASSERT_FALSE(device_->IsBindCaller(tokenId));
    EDM_LOGI(MODULE_DEV_MGR, "IsBindCallerFalseTest: Caller is not bound");
}

HWTEST_F(DeviceTest, GetBundleNameTest, TestSize.Level1)
{
    string bundleInfo = "com.example.driver/DriverAbility";
    string bundleName = Device::GetBundleName(bundleInfo);
    ASSERT_EQ(bundleName, "com.example.driver");
    EDM_LOGI(MODULE_DEV_MGR, "GetBundleNameTest: Bundle name parsed");
}

HWTEST_F(DeviceTest, GetAbilityNameTest, TestSize.Level1)
{
    string bundleInfo = "com.example.driver/DriverAbility";
    string abilityName = Device::GetAbilityName(bundleInfo);
    ASSERT_EQ(abilityName, "DriverAbility");
    EDM_LOGI(MODULE_DEV_MGR, "GetAbilityNameTest: Ability name parsed");
}

class TestDriverExtMgrCallback : public IDriverExtMgrCallback {
public:
    TestDriverExtMgrCallback() = default;
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

HWTEST_F(DeviceTest, ConnectSuccessTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    driverInfo->accessAllowed_ = true;
    device_->AddBundleInfo("testBundle", driverInfo);
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device_->OnConnect(remote, 0);
    int32_t ret = device_->Connect();
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectSuccessTest: Device connected");
}

HWTEST_F(DeviceTest, ConnectWithCallbackTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    driverInfo->accessAllowed_ = true;
    device_->AddBundleInfo("testBundle", driverInfo);
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device_->OnConnect(remote, 0);
    sptr<IDriverExtMgrCallback> callback = sptr<TestDriverExtMgrCallback>::MakeSptr();
    uint32_t tokenId = 100;
    int32_t ret = device_->Connect(callback, tokenId);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectWithCallbackTest: Device connected with callback");
}

HWTEST_F(DeviceTest, ConnectNoDriverTest, TestSize.Level1)
{
    ASSERT_FALSE(device_->HasDriver());
    int32_t ret = device_->Connect();
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectNoDriverTest: Connect failed without driver");
}

HWTEST_F(DeviceTest, ConnectAccessDeniedTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    driverInfo->accessAllowed_ = false;
    device_->AddBundleInfo("testBundle", driverInfo);
    int32_t ret = device_->Connect();
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectAccessDeniedTest: Connect failed due to access denied");
}

HWTEST_F(DeviceTest, DisconnectSuccessTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    driverInfo->accessAllowed_ = true;
    device_->AddBundleInfo("testBundle", driverInfo);
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device_->OnConnect(remote, 0);
    uint32_t tokenId = 100;
    CallerInfo info;
    info.isBound = true;
    device_->boundCallerInfos_[tokenId] = info;
    int32_t ret = device_->Disconnect(false);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisconnectSuccessTest: Device disconnected");
}

HWTEST_F(DeviceTest, DisconnectFromBindTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    device_->AddBundleInfo("testBundle", driverInfo);
    int32_t ret = device_->Disconnect(true);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisconnectFromBindTest: Device disconnected from bind");
}

HWTEST_F(DeviceTest, OnConnectSuccessTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    device_->AddBundleInfo("testBundle", driverInfo);
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device_->OnConnect(remote, 0);
    ASSERT_NE(device_->GetDrvExtRemote(), nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "OnConnectSuccessTest: OnConnect callback succeeded");
}

HWTEST_F(DeviceTest, OnConnectFailedTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    device_->AddBundleInfo("testBundle", driverInfo);
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device_->OnConnect(remote, -1);
    ASSERT_EQ(device_->GetDrvExtRemote(), nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "OnConnectFailedTest: OnConnect callback failed");
}

HWTEST_F(DeviceTest, OnDisconnectTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    device_->AddBundleInfo("testBundle", driverInfo);
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device_->OnConnect(remote, 0);
    device_->OnDisconnect(0);
    EDM_LOGI(MODULE_DEV_MGR, "OnDisconnectTest: OnDisconnect callback executed");
}
} // namespace ExternalDeviceManager
} // namespace OHOS
