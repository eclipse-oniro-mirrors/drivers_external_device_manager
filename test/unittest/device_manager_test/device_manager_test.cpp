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

#include <gtest/gtest.h>
#include "edm_errors.h"
#include "hilog_wrapper.h"
#define private public
#include "etx_device_mgr.h"
#include "ibus_extension.h"
#include "usb_bus_extension.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

class DeviceManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DeviceManagerTest SetUp");
    }
    void TearDown() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DeviceManagerTest TearDown");
    }
};

HWTEST_F(DeviceManagerTest, BusExtensionRegisterTest, TestSize.Level1)
{
    std::shared_ptr<BusExtensionCore> core = DelayedSingleton<BusExtensionCore>::GetInstance();
    int32_t ret = core->Register(BusType::BUS_TYPE_USB, std::make_shared<UsbBusExtension>());
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_NE(core->busExtensions_[BusType::BUS_TYPE_USB], nullptr);
}

HWTEST_F(DeviceManagerTest, InitTest, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<ExtDeviceManager>::GetInstance()->Init();
    ASSERT_EQ(ret, EDM_OK);
}

// test OnDeviceAdd and OnDeviceRemove
HWTEST_F(DeviceManagerTest, OnDeviceAddRemoveTest001, TestSize.Level1)
{
    std::shared_ptr<DevChangeCallback> callback =
        std::make_shared<DevChangeCallback>(BusType::BUS_TYPE_TEST, DelayedSingleton<ExtDeviceManager>::GetInstance());
    std::shared_ptr<DeviceInfo> device = std::make_shared<DeviceInfo>(0);
    device->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device->devInfo_.devBusInfo.busDeviceId = 1;
    int32_t ret = callback->OnDeviceAdd(device);
    ASSERT_EQ(ret, EDM_OK);
    std::shared_ptr<ExtDeviceManager> extMgr = DelayedSingleton<ExtDeviceManager>::GetInstance();
    ASSERT_EQ(extMgr->deviceMap_[BusType::BUS_TYPE_TEST].size(), 1);
    ret = callback->OnDeviceRemove(device);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(extMgr->deviceMap_[BusType::BUS_TYPE_TEST].size(), 0);
}

// test adding device repeatedly
HWTEST_F(DeviceManagerTest, OnDeviceAddRemoveTest002, TestSize.Level1)
{
    std::shared_ptr<DevChangeCallback> callback =
        std::make_shared<DevChangeCallback>(BusType::BUS_TYPE_TEST, DelayedSingleton<ExtDeviceManager>::GetInstance());
    std::shared_ptr<DeviceInfo> device = std::make_shared<DeviceInfo>(0);
    device->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device->devInfo_.devBusInfo.busDeviceId = 1;
    int32_t ret = callback->OnDeviceAdd(device);
    ASSERT_EQ(ret, EDM_OK);
    ret = callback->OnDeviceAdd(device);
    ASSERT_EQ(ret, EDM_OK);
    std::shared_ptr<ExtDeviceManager> extMgr = DelayedSingleton<ExtDeviceManager>::GetInstance();
    ASSERT_EQ(extMgr->deviceMap_[BusType::BUS_TYPE_TEST].size(), 1);
    ret = callback->OnDeviceRemove(device);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(extMgr->deviceMap_[BusType::BUS_TYPE_TEST].size(), 0);
    ret = callback->OnDeviceRemove(device);
    ASSERT_EQ(ret, EDM_OK);
}

HWTEST_F(DeviceManagerTest, OnDeviceAddRemoveTest003, TestSize.Level1)
{
    std::shared_ptr<DevChangeCallback> callback =
        std::make_shared<DevChangeCallback>(BusType::BUS_TYPE_TEST, DelayedSingleton<ExtDeviceManager>::GetInstance());
    std::shared_ptr<DeviceInfo> device0 = std::make_shared<DeviceInfo>(0);
    device0->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device0->devInfo_.devBusInfo.busDeviceId = 1;
    int32_t ret = callback->OnDeviceAdd(device0);
    ASSERT_EQ(ret, EDM_OK);
    std::shared_ptr<DeviceInfo> device1 = std::make_shared<DeviceInfo>(0);
    device1->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device1->devInfo_.devBusInfo.busDeviceId = 2;
    ret = callback->OnDeviceAdd(device1);
    ASSERT_EQ(ret, EDM_OK);
    std::shared_ptr<ExtDeviceManager> extMgr = DelayedSingleton<ExtDeviceManager>::GetInstance();
    ASSERT_EQ(extMgr->deviceMap_[BusType::BUS_TYPE_TEST].size(), 2);
    ret = callback->OnDeviceRemove(device1);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(extMgr->deviceMap_[BusType::BUS_TYPE_TEST].size(), 1);
    ret = callback->OnDeviceRemove(device0);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(extMgr->deviceMap_[BusType::BUS_TYPE_TEST].size(), 0);
}
} // namespace ExternalDeviceManager
} // namespace OHOS
