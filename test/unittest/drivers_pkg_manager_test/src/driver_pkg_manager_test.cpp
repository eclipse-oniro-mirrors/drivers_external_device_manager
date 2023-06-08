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
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;
using namespace OHOS::ExternalDeviceManager;

class DriverPkgManagerTest : public testing::Test {
public:
    DriverPkgManager drvPkgMgrInstance;
    void SetUp() override
    {
        cout << "DriverPkgManagerTest SetUp" << endl;
    }
    void TearDown() override
    {
        cout << "DriverPkgManagerTest TearDown" << endl;
    }
};

HWTEST_F(DriverPkgManagerTest, DrvExt_Init_Test, TestSize.Level1)
{
    bool ret = drvPkgMgrInstance.Init();
    EXPECT_EQ(true, ret);
    cout << "DrvExt_Init_New_BundleTest" << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_QueryMatch_Befor_Init_Test, TestSize.Level1)
{
    DeviceInfo devInfo = DeviceInfo(1);
    devInfo.devInfo_.devBusInfo.busType = BUS_TYPE_USB;
    BundleInfoNames* bundle = drvPkgMgrInstance.QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Befor_Init_Test" << endl;
}

void Fun(int a, string b, string c)
{
    cout << a << endl;
    cout << b << endl;
    cout << c << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_RegisterOnBundleUpdate_Befor_Init_Test, TestSize.Level1)
{
    bool ret = drvPkgMgrInstance.Init();
    if (!ret) {
        EXPECT_EQ(true, ret);
        return;
    }
    drvPkgMgrInstance.RegisterOnBundleUpdate(Fun);
    cout << "DrvExt_RegisterOnBundleUpdate_Befor_Init_Test" << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_QueryMatch_Illegal_Bus_Test, TestSize.Level1)
{
    bool ret = drvPkgMgrInstance.Init();
    if (!ret) {
        EXPECT_EQ(true, ret);
        return;
    }
    DeviceInfo devInfo = DeviceInfo(1);
    devInfo.devInfo_.devBusInfo.busType = BUS_TYPE_INVALID;
    BundleInfoNames* bundle = drvPkgMgrInstance.QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Illegal_Bus_Test" << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_QueryMatch_Illegal_ID_Test, TestSize.Level1)
{
    bool ret = drvPkgMgrInstance.Init();
    if (!ret) {
        EXPECT_EQ(true, ret);
        return;
    }
    DeviceInfo devInfo = DeviceInfo(1);
    devInfo.devInfo_.devBusInfo.busType = BUS_TYPE_USB;
    BundleInfoNames* bundle = drvPkgMgrInstance.QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Illegal_ID_Test" << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_QueryMatch_Null_ID_Test, TestSize.Level1)
{
    bool ret = drvPkgMgrInstance.Init();
    if (!ret) {
        EXPECT_EQ(true, ret);
        return;
    }
    DeviceInfo devInfo = DeviceInfo(0);
    devInfo.devInfo_.devBusInfo.busType = BUS_TYPE_USB;
    BundleInfoNames* bundle = drvPkgMgrInstance.QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Illegal_ID_Test" << endl;
}

class DriverPkgManagerPtrTest : public testing::Test {
public:
    DriverPkgManager *drvPkgMgrInstance = nullptr;
    void SetUp() override
    {
        drvPkgMgrInstance = new DriverPkgManager();
        cout << "DriverPkgManagerPtrTest SetUp" << endl;
    }
    void TearDown() override
    {
        if (drvPkgMgrInstance != nullptr) {
            delete drvPkgMgrInstance;
            drvPkgMgrInstance = nullptr;
        }
        cout << "DriverPkgManagerPtrTest TearDown" << endl;
    }
};

HWTEST_F(DriverPkgManagerPtrTest, DrvExt_New_Test, TestSize.Level1)
{
    bool ret = drvPkgMgrInstance->Init();
    if (!ret) {
        EXPECT_EQ(true, ret);
        return;
    }
    EXPECT_NE(nullptr, drvPkgMgrInstance);
    cout << "DrvExt_New_Test" << endl;
}

HWTEST_F(DriverPkgManagerPtrTest, DrvExt_QueryMatch_Before_Init_Test, TestSize.Level1)
{
    DeviceInfo devInfo = DeviceInfo(0);
    BundleInfoNames* bundle = drvPkgMgrInstance->QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Before_Init_Test" << endl;
}

HWTEST_F(DriverPkgManagerPtrTest, DrvExt_Delete_Test, TestSize.Level1)
{
    if (drvPkgMgrInstance != nullptr) {
        delete drvPkgMgrInstance;
        drvPkgMgrInstance = nullptr;
        EXPECT_EQ(nullptr, drvPkgMgrInstance);
    }
    cout << "DrvExt_Delete_Test" << endl;
}
}
}