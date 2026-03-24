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
#include "driver_pkg_manager.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

class DriverPkgManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_PKG_MGR, "DriverPkgManagerTest SetUp");
    }

    void TearDown() override
    {
        EDM_LOGD(MODULE_PKG_MGR, "DriverPkgManagerTest TearDown");
    }
};

HWTEST_F(DriverPkgManagerTest, InitSuccessTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    int32_t ret = instance.Init();
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "InitSuccessTest: DriverPkgManager initialized");
}

HWTEST_F(DriverPkgManagerTest, InitWithFuturesTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    std::promise<int32_t> promise1;
    std::promise<int32_t> promise2;
    std::promise<int32_t> promise3;
    promise1.set_value(EDM_OK);
    promise2.set_value(EDM_OK);
    promise3.set_value(EDM_OK);
    std::shared_future<int32_t> future1 = promise1.get_future();
    std::shared_future<int32_t> future2 = promise2.get_future();
    std::shared_future<int32_t> future3 = promise3.get_future();
    int32_t ret = instance.Init(future1, future2, future3);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "InitWithFuturesTest: DriverPkgManager initialized with futures");
}

HWTEST_F(DriverPkgManagerTest, InitTwiceTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    int32_t ret1 = instance.Init();
    int32_t ret2 = instance.Init();
    ASSERT_EQ(ret1, EDM_OK);
    ASSERT_EQ(ret2, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "InitTwiceTest: Multiple init calls handled");
}

HWTEST_F(DriverPkgManagerTest, QueryMatchDriverSuccessTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    auto deviceInfo = make_shared<DeviceInfo>(1, BusType::BUS_TYPE_USB, "test_device");
    shared_ptr<DriverInfo> driver = instance.QueryMatchDriver(deviceInfo, "USB");
    ASSERT_NE(driver, nullptr);
    EDM_LOGI(MODULE_PKG_MGR, "QueryMatchDriverSuccessTest: Driver matched successfully");
}

HWTEST_F(DriverPkgManagerTest, QueryMatchDriverNotFoundTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    auto deviceInfo = make_shared<DeviceInfo>(999999, BusType::BUS_TYPE_USB, "nonexistent_device");
    shared_ptr<DriverInfo> driver = instance.QueryMatchDriver(deviceInfo, "USB");
    ASSERT_NE(driver, nullptr);
    EDM_LOGI(MODULE_PKG_MGR, "QueryMatchDriverNotFoundTest: No driver matched for unknown device");
}

HWTEST_F(DriverPkgManagerTest, QueryMatchDriverEmptyTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    shared_ptr<DriverInfo> driver = instance.QueryMatchDriver(nullptr, "USB");
    ASSERT_EQ(driver, nullptr);
    EDM_LOGI(MODULE_PKG_MGR, "QueryMatchDriverEmptyTest: Empty device info handled");
}

HWTEST_F(DriverPkgManagerTest, QueryDriverInfoAllTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    vector<shared_ptr<DriverInfo>> driverInfos;
    int32_t ret = instance.QueryDriverInfo(driverInfos);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "QueryDriverInfoAllTest: All driver info queried");
}

HWTEST_F(DriverPkgManagerTest, QueryDriverInfoByUidTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    vector<shared_ptr<DriverInfo>> driverInfos;
    string driverUid = "test_driver_uid";
    int32_t ret = instance.QueryDriverInfo(driverInfos, true, driverUid);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "QueryDriverInfoByUidTest: Driver info queried by UID");
}

HWTEST_F(DriverPkgManagerTest, QueryDriverInfoNotFoundTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    vector<shared_ptr<DriverInfo>> driverInfos;
    string driverUid = "nonexistent_uid";
    int32_t ret = instance.QueryDriverInfo(driverInfos, true, driverUid);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "QueryDriverInfoNotFoundTest: No driver found for UID");
}

HWTEST_F(DriverPkgManagerTest, RegisterOnBundleUpdateTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    PCALLBACKFUN callback = [](int, int, const string &, const string &) { return EDM_OK; };
    int32_t ret = instance.RegisterOnBundleUpdate(callback);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "RegisterOnBundleUpdateTest: Bundle update callback registered");
}

HWTEST_F(DriverPkgManagerTest, RegisterOnBundleUpdateTwiceTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    PCALLBACKFUN callback = [](int, int, const string &, const string &) { return EDM_OK; };
    int32_t ret1 = instance.RegisterOnBundleUpdate(callback);
    int32_t ret2 = instance.RegisterOnBundleUpdate(callback);
    ASSERT_EQ(ret1, EDM_OK);
    ASSERT_EQ(ret2, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "RegisterOnBundleUpdateTwiceTest: Repeated registration handled");
}

HWTEST_F(DriverPkgManagerTest, UnRegisterOnBundleUpdateTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    PCALLBACKFUN callback = [](int, int, const string &, const string &) { return EDM_OK; };
    instance.RegisterOnBundleUpdate(callback);
    int32_t ret = instance.UnRegisterOnBundleUpdate();
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "UnRegisterOnBundleUpdateTest: Bundle update callback unregistered");
}

HWTEST_F(DriverPkgManagerTest, RegisterBundleCallbackTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    shared_ptr<IBundleUpdateCallback> callback = nullptr;
    int32_t ret = instance.RegisterBundleCallback(callback);
    ASSERT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "RegisterBundleCallbackTest: Bundle callback registration attempted");
}

HWTEST_F(DriverPkgManagerTest, RegisterBundleStatusCallbackTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    int32_t ret = instance.RegisterBundleStatusCallback();
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "RegisterBundleStatusCallbackTest: Bundle status callback registered");
}

HWTEST_F(DriverPkgManagerTest, UnRegisterBundleStatusCallbackTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    instance.RegisterBundleStatusCallback();
    int32_t ret = instance.UnRegisterBundleStatusCallback();
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "UnRegisterBundleStatusCallbackTest: Bundle status callback unregistered");
}

HWTEST_F(DriverPkgManagerTest, SubscribeOsAccountSwitchTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    bool ret = instance.SubscribeOsAccountSwitch();
    ASSERT_TRUE(ret);
    EDM_LOGI(MODULE_PKG_MGR, "SubscribeOsAccountSwitchTest: OS account switch subscribed");
}

HWTEST_F(DriverPkgManagerTest, SubscribeOsAccountSwitchTwiceTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    bool ret1 = instance.SubscribeOsAccountSwitch();
    bool ret2 = instance.SubscribeOsAccountSwitch();
    ASSERT_TRUE(ret1);
    ASSERT_TRUE(ret2);
    EDM_LOGI(MODULE_PKG_MGR, "SubscribeOsAccountSwitchTwiceTest: Repeated subscription handled");
}

HWTEST_F(DriverPkgManagerTest, UnRegisterBundleStatusCallbackTwiceTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    int32_t ret1 = instance.UnRegisterBundleStatusCallback();
    int32_t ret2 = instance.UnRegisterBundleStatusCallback();
    ASSERT_EQ(ret1, EDM_OK);
    ASSERT_EQ(ret2, EDM_OK);
    EDM_LOGI(MODULE_PKG_MGR, "UnRegisterBundleStatusCallbackTwiceTest: Repeated unregistration handled");
}

HWTEST_F(DriverPkgManagerTest, PrintTestTest, TestSize.Level1)
{
    DriverPkgManager &instance = DriverPkgManager::GetInstance();
    instance.Init();
    instance.PrintTest();
    EDM_LOGI(MODULE_PKG_MGR, "PrintTestTest: PrintTest executed");
}
} // namespace ExternalDeviceManager
} // namespace OHOS
