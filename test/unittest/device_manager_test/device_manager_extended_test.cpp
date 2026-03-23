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
#include "etx_device_mgr.h"
#include "driver_pkg_manager.h"
#include "dev_change_callback.h"
#include "usb_device_info.h"
#include "usb_driver_info.h"
#include "driver_info.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

const unordered_set<std::string> accessibleBundles = {"testBundleName1", "testBundleName2"};

class ExtDeviceManagerExtendedTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "ExtDeviceManagerExtendedTest SetUp");
        DriverPkgManager::GetInstance().Init();
        ExtDeviceManager::GetInstance().Init();
    }

    void TearDown() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "ExtDeviceManagerExtendedTest TearDown");
    }
};

class TestRemoteObject : public IRemoteObject {
public:
    TestRemoteObject() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; }
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; }
};

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

HWTEST_F(ExtDeviceManagerExtendedTest, MatchDriverInfosSuccessTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    usbDeviceInfo->idVendor_ = 0x1111;
    usbDeviceInfo->idProduct_ = 0x1234;
    int32_t ret = callback->OnDeviceAdd(usbDeviceInfo);
    ASSERT_EQ(ret, EDM_OK);

    unordered_set<uint64_t> deviceIds = {1};
    extMgr.MatchDriverInfos(deviceIds);
    EDM_LOGI(MODULE_DEV_MGR, "MatchDriverInfosSuccessTest: Driver matched successfully");
}

HWTEST_F(ExtDeviceManagerExtendedTest, MatchDriverInfosMultipleTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo1 = make_shared<UsbDeviceInfo>(1, "testInfo1");
    auto usbDeviceInfo2 = make_shared<UsbDeviceInfo>(2, "testInfo2");
    int32_t ret = callback->OnDeviceAdd(usbDeviceInfo1);
    ASSERT_EQ(ret, EDM_OK);
    ret = callback->OnDeviceAdd(usbDeviceInfo2);
    ASSERT_EQ(ret, EDM_OK);

    unordered_set<uint64_t> deviceIds = {1, 2};
    extMgr.MatchDriverInfos(deviceIds);
    EDM_LOGI(MODULE_DEV_MGR, "MatchDriverInfosMultipleTest: Multiple devices matched");
}

HWTEST_F(ExtDeviceManagerExtendedTest, MatchDriverInfosNoMatchTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    unordered_set<uint64_t> deviceIds = {999999};
    extMgr.MatchDriverInfos(deviceIds);
    EDM_LOGI(MODULE_DEV_MGR, "MatchDriverInfosNoMatchTest: No driver matched");
}

HWTEST_F(ExtDeviceManagerExtendedTest, MatchDriverInfosEmptyTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    unordered_set<uint64_t> deviceIds;
    extMgr.MatchDriverInfos(deviceIds);
    EDM_LOGI(MODULE_DEV_MGR, "MatchDriverInfosEmptyTest: Empty device list handled");
}

HWTEST_F(ExtDeviceManagerExtendedTest, ClearMatchedDriversTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    int32_t userId = 100;
    extMgr.ClearMatchedDrivers(userId);
    EDM_LOGI(MODULE_DEV_MGR, "ClearMatchedDriversTest: Matched drivers cleared");
}

HWTEST_F(ExtDeviceManagerExtendedTest, ClearMatchedDriversInvalidUserTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    int32_t userId = -1;
    extMgr.ClearMatchedDrivers(userId);
    EDM_LOGI(MODULE_DEV_MGR, "ClearMatchedDriversInvalidUserTest: Invalid userId handled");
}

HWTEST_F(ExtDeviceManagerExtendedTest, CheckAccessPermissionAllowedTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    auto driverInfo = make_shared<DriverInfo>("testBundleName1", "testDriverName1");
    int32_t ret = extMgr.CheckAccessPermission(driverInfo, accessibleBundles);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "CheckAccessPermissionAllowedTest: Access allowed");
}

HWTEST_F(ExtDeviceManagerExtendedTest, CheckAccessPermissionDeniedTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    auto driverInfo = make_shared<DriverInfo>("testBundleName3", "testDriverName3");
    int32_t ret = extMgr.CheckAccessPermission(driverInfo, accessibleBundles);
    ASSERT_EQ(ret, EDM_ERR_NO_PERM);
    EDM_LOGI(MODULE_DEV_MGR, "CheckAccessPermissionDeniedTest: Access denied");
}

HWTEST_F(ExtDeviceManagerExtendedTest, CheckAccessPermissionEmptyBundlesTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    auto driverInfo = make_shared<DriverInfo>("testBundleName1", "testDriverName1");
    unordered_set<std::string> emptyBundles;
    int32_t ret = extMgr.CheckAccessPermission(driverInfo, emptyBundles);
    ASSERT_EQ(ret, EDM_ERR_NO_PERM);
    EDM_LOGI(MODULE_DEV_MGR, "CheckAccessPermissionEmptyBundlesTest: Empty bundles handled");
}

HWTEST_F(ExtDeviceManagerExtendedTest, CheckAccessPermissionNullDriverTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    int32_t ret = extMgr.CheckAccessPermission(nullptr, accessibleBundles);
    ASSERT_EQ(ret, EDM_ERR_INVALID_PARAM);
    EDM_LOGI(MODULE_DEV_MGR, "CheckAccessPermissionNullDriverTest: Null driver info handled");
}

HWTEST_F(ExtDeviceManagerExtendedTest, ConnectDriverWithDeviceIdSuccessTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    usbDeviceInfo->idVendor_ = 0x1111;
    usbDeviceInfo->idProduct_ = 0x1234;
    callback->OnDeviceAdd(usbDeviceInfo);

    auto device = extMgr.QueryDeviceByDeviceID(1);
    device->driverInfo_ = make_shared<DriverInfo>("testBundleName1", "testDriverName1");
    device->driverInfo_->accessAllowed_ = true;
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device->OnConnect(remote, 0);

    sptr<IDriverExtMgrCallback> connectCallback = sptr<TestDriverExtMgrCallback>::MakeSptr();
    uint32_t tokenId = 100;
    int32_t ret = extMgr.ConnectDriverWithDeviceId(1, tokenId, accessibleBundles, connectCallback);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDriverWithDeviceIdSuccessTest: Connected successfully");
}

HWTEST_F(ExtDeviceManagerExtendedTest, ConnectDriverWithDeviceIdPermissionTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    callback->OnDeviceAdd(usbDeviceInfo);

    auto device = extMgr.QueryDeviceByDeviceID(1);
    device->driverInfo_ = make_shared<DriverInfo>("testBundleName3", "testDriverName3");
    device->driverInfo_->accessAllowed_ = false;

    sptr<IDriverExtMgrCallback> connectCallback = sptr<TestDriverExtMgrCallback>::MakeSptr();
    uint32_t tokenId = 100;
    int32_t ret = extMgr.ConnectDriverWithDeviceId(1, tokenId, accessibleBundles, connectCallback);
    ASSERT_EQ(ret, EDM_ERR_SERVICE_NOT_ALLOW_ACCESS);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDriverWithDeviceIdPermissionTest: Permission checked");
}

HWTEST_F(ExtDeviceManagerExtendedTest, ConnectDriverWithDeviceIdNotFoundTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    sptr<IDriverExtMgrCallback> connectCallback = sptr<TestDriverExtMgrCallback>::MakeSptr();
    uint32_t tokenId = 100;
    int32_t ret = extMgr.ConnectDriverWithDeviceId(999999, tokenId, accessibleBundles, connectCallback);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "ConnectDriverWithDeviceIdNotFoundTest: Device not found");
}

HWTEST_F(ExtDeviceManagerExtendedTest, DisConnectDriverWithDeviceIdSuccessTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    callback->OnDeviceAdd(usbDeviceInfo);

    auto device = extMgr.QueryDeviceByDeviceID(1);
    device->driverInfo_ = make_shared<DriverInfo>("testBundleName1", "testDriverName1");
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    device->OnConnect(remote, 0);

    sptr<IDriverExtMgrCallback> connectCallback = sptr<TestDriverExtMgrCallback>::MakeSptr();
    uint32_t tokenId = 100;
    extMgr.ConnectDriverWithDeviceId(1, tokenId, accessibleBundles, connectCallback);

    int32_t ret = extMgr.DisConnectDriverWithDeviceId(1, tokenId);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisConnectDriverWithDeviceIdSuccessTest: Disconnected successfully");
}

HWTEST_F(ExtDeviceManagerExtendedTest, DisConnectDriverWithDeviceIdNotFoundTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    uint32_t tokenId = 100;
    int32_t ret = extMgr.DisConnectDriverWithDeviceId(999999, tokenId);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "DisConnectDriverWithDeviceIdNotFoundTest: Device not found");
}

HWTEST_F(ExtDeviceManagerExtendedTest, DisConnectDriverWithDeviceIdNotBoundTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    callback->OnDeviceAdd(usbDeviceInfo);

    auto device = extMgr.QueryDeviceByDeviceID(1);
    device->driverInfo_ = make_shared<DriverInfo>("testBundleName1", "testDriverName1");

    uint32_t tokenId = 100;
    int32_t ret = extMgr.DisConnectDriverWithDeviceId(1, tokenId);
    ASSERT_EQ(ret, EDM_ERR_SERVICE_NOT_BOUND);
    EDM_LOGI(MODULE_DEV_MGR, "DisConnectDriverWithDeviceIdNotBoundTest: Device not bound");
}

HWTEST_F(ExtDeviceManagerExtendedTest, DeleteBundlesOfBundleInfoMapTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    callback->OnDeviceAdd(usbDeviceInfo);

    auto device = extMgr.QueryDeviceByDeviceID(1);
    device->AddBundleInfo("testBundleName1", make_shared<DriverInfo>("testBundleName1", "testDriverName1"));
    extMgr.AddDevIdOfBundleInfoMap(device, string("testBundleName1"));

    unordered_set<uint64_t> result = extMgr.DeleteBundlesOfBundleInfoMap("testBundleName1");
    ASSERT_EQ(result.size(), 1);
    EDM_LOGI(MODULE_DEV_MGR, "DeleteBundlesOfBundleInfoMapTest: Bundle deleted");
}

HWTEST_F(ExtDeviceManagerExtendedTest, DeleteBundlesOfBundleInfoMapAllTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    unordered_set<uint64_t> result = extMgr.DeleteBundlesOfBundleInfoMap("");
    EDM_LOGI(MODULE_DEV_MGR, "DeleteBundlesOfBundleInfoMapAllTest: All bundles deleted");
}

HWTEST_F(ExtDeviceManagerExtendedTest, DeleteBundlesOfBundleInfoMapEmptyTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    unordered_set<uint64_t> result = extMgr.DeleteBundlesOfBundleInfoMap("nonexistentBundle");
    ASSERT_EQ(result.size(), 0);
    EDM_LOGI(MODULE_DEV_MGR, "DeleteBundlesOfBundleInfoMapEmptyTest: Empty bundle handled");
}

HWTEST_F(ExtDeviceManagerExtendedTest, SetDriverChangeCallbackTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<IDriverChangeCallback> callback = nullptr;
    extMgr.SetDriverChangeCallback(callback);
    EDM_LOGI(MODULE_DEV_MGR, "SetDriverChangeCallbackTest: Callback set");
}

HWTEST_F(ExtDeviceManagerExtendedTest, GetTotalDeviceNumTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    size_t num = extMgr.GetTotalDeviceNum();
    EDM_LOGI(MODULE_DEV_MGR, "GetTotalDeviceNumTest: Total device count = %zu", num);
}

HWTEST_F(ExtDeviceManagerExtendedTest, PrintMatchDriverMapTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    extMgr.PrintMatchDriverMap();
    EDM_LOGI(MODULE_DEV_MGR, "PrintMatchDriverMapTest: Match driver map printed");
}

HWTEST_F(ExtDeviceManagerExtendedTest, AddDevIdOfBundleInfoMapTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    callback->OnDeviceAdd(usbDeviceInfo);

    auto device = extMgr.QueryDeviceByDeviceID(1);
    string bundleInfo = "testBundleName1/testAbility";
    int32_t ret = extMgr.AddDevIdOfBundleInfoMap(device, bundleInfo);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "AddDevIdOfBundleInfoMapTest: Device added to bundle map");
}

HWTEST_F(ExtDeviceManagerExtendedTest, RemoveDevIdOfBundleInfoMapTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    callback->OnDeviceAdd(usbDeviceInfo);

    auto device = extMgr.QueryDeviceByDeviceID(1);
    string bundleInfo = "testBundleName1/testAbility";
    extMgr.AddDevIdOfBundleInfoMap(device, bundleInfo);
    int32_t ret = extMgr.RemoveDevIdOfBundleInfoMap(device, bundleInfo);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_DEV_MGR, "RemoveDevIdOfBundleInfoMapTest: Device removed from bundle map");
}

HWTEST_F(ExtDeviceManagerExtendedTest, UpdateDriverInfoTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    callback->OnDeviceAdd(usbDeviceInfo);

    auto device = extMgr.QueryDeviceByDeviceID(1);
    extMgr.UpdateDriverInfo(device);
    EDM_LOGI(MODULE_DEV_MGR, "UpdateDriverInfoTest: Driver info updated");
}

HWTEST_F(ExtDeviceManagerExtendedTest, RemoveDriverInfoTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    auto usbDeviceInfo = make_shared<UsbDeviceInfo>(1, "testInfo");
    callback->OnDeviceAdd(usbDeviceInfo);

    auto device = extMgr.QueryDeviceByDeviceID(1);
    extMgr.RemoveDriverInfo(device);
    EDM_LOGI(MODULE_DEV_MGR, "RemoveDriverInfoTest: Driver info removed");
}
} // namespace ExternalDeviceManager
} // namespace OHOS
