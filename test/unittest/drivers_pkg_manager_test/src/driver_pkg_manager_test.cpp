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
#include <iostream>
#define private public
#include "driver_pkg_manager.h"
#include "ibus_extension.h"
#include "usb_device_info.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;
using namespace OHOS::ExternalDeviceManager;

class DriverPkgManagerTest : public testing::Test {
public:
    DriverPkgManager *drvPkgMgrInstance = nullptr;;
    void SetUp() override
    {
        drvPkgMgrInstance = new DriverPkgManager();
        cout << "DriverPkgManagerTest SetUp" << endl;
    }
    void TearDown() override
    {
        if (drvPkgMgrInstance != nullptr) {
            delete drvPkgMgrInstance;
            drvPkgMgrInstance = nullptr;
        }
        cout << "DriverPkgManagerTest TearDown" << endl;
    }
};

HWTEST_F(DriverPkgManagerTest, DrvExt_Init_Test, TestSize.Level1)
{
    int32_t ret = drvPkgMgrInstance->Init();
    EXPECT_EQ(0, ret);
    cout << "DrvExt_Init_New_BundleTest" << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_QueryMatch_Befor_Init_Test, TestSize.Level1)
{
    std::shared_ptr<DeviceInfo> devInfo = std::make_shared<DeviceInfo>(
    1);
    devInfo->devInfo_.devBusInfo.busType = BUS_TYPE_USB;
    std::shared_ptr<BundleInfoNames> bundle = drvPkgMgrInstance->QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Befor_Init_Test" << endl;
}

int32_t Fun(int a, int b, const string & c, const string & d)
{
    cout << a << endl;
    cout << b << endl;
    cout << c << endl;
    cout << d << endl;
    return EDM_OK;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_RegisterOnBundleUpdate_Init_Test, TestSize.Level1)
{
    int32_t ret = drvPkgMgrInstance->Init();
    if (ret != 0) {
        EXPECT_EQ(0, ret);
        return;
    }
    int32_t regist = drvPkgMgrInstance->RegisterOnBundleUpdate(Fun);
    EXPECT_EQ(0, regist);
    cout << "DrvExt_RegisterOnBundleUpdate_Init_Test" << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_QueryMatch_Illegal_Bus_Test, TestSize.Level1)
{
    int32_t ret = drvPkgMgrInstance->Init();
    if (ret != 0) {
        EXPECT_EQ(0, ret);
        return;
    }
    std::shared_ptr<DeviceInfo> devInfo = std::make_shared<DeviceInfo>(
    1);
    devInfo->devInfo_.devBusInfo.busType = BUS_TYPE_INVALID;
    std::shared_ptr<BundleInfoNames> bundle = drvPkgMgrInstance->QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Illegal_Bus_Test" << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_QueryMatch_ID_Test, TestSize.Level1)
{
    int32_t ret = drvPkgMgrInstance->Init();
    if (ret != 0) {
        EXPECT_EQ(0, ret);
        return;
    }
    auto deviceInfo = make_shared<UsbDeviceInfo>(0);
    deviceInfo->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_USB;
    deviceInfo->idProduct_ = 0x8835;
    deviceInfo->idVendor_ = 0x0B57;
    deviceInfo->deviceClass_ = 0;
    deviceInfo->bcdUSB_ = 0x1122;
    
    std::shared_ptr<BundleInfoNames> bundle = drvPkgMgrInstance->QueryMatchDriver(deviceInfo);
    EXPECT_NE(nullptr, bundle);
    cout << "DrvExt_QueryMatch_ID_Test" << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_QueryMatch_Null_ID_Test, TestSize.Level1)
{
    int32_t ret = drvPkgMgrInstance->Init();
    if (ret != 0) {
        EXPECT_EQ(0, ret);
        return;
    }
    std::shared_ptr<DeviceInfo> devInfo = std::make_shared<DeviceInfo>(
    0);
    devInfo->devInfo_.devBusInfo.busType = BUS_TYPE_USB;
    std::shared_ptr<BundleInfoNames> bundle = drvPkgMgrInstance->QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Illegal_ID_Test" << endl;
}

class DriverPkgManagerPtrTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(DriverPkgManagerPtrTest, DrvExt_QueryMatch_Before_Init_Test, TestSize.Level1)
{
    DriverPkgManager &drvPkgMgrInstance = DriverPkgManager::GetInstance();
    std::shared_ptr<DeviceInfo> devInfo = std::make_shared<DeviceInfo>(0);
    std::shared_ptr<BundleInfoNames> bundle = drvPkgMgrInstance.QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Before_Init_Test" << endl;
}

HWTEST_F(DriverPkgManagerPtrTest, DrvExt_New_Test, TestSize.Level1)
{
    DriverPkgManager &drvPkgMgrInstance = DriverPkgManager::GetInstance();
    bool ret = drvPkgMgrInstance.Init();
    if (ret != 0) {
        EXPECT_EQ(0, ret);
        return;
    }
    EXPECT_NE(nullptr, &drvPkgMgrInstance);
    cout << "DrvExt_New_Test" << endl;
}

HWTEST_F(DriverPkgManagerPtrTest, DrvExt_Delete_Test, TestSize.Level1)
{
    auto drvPkgMgrInstance = &(DriverPkgManager::GetInstance());
    if (drvPkgMgrInstance != nullptr) {
        delete drvPkgMgrInstance;
        drvPkgMgrInstance = nullptr;
        EXPECT_EQ(nullptr, drvPkgMgrInstance);
    }
    cout << "DrvExt_Delete_Test" << endl;
}
}
}