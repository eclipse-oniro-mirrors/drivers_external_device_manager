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
#include "json.h"
#include "ibus_extension.h"
#include "hilog_wrapper.h"
#include "usb_driver_info.h"
#include "usb_device_info.h"
#include "usb_bus_extension.h"

namespace OHOS {
namespace ExtDevMgr {
using namespace std;
using namespace testing::ext;

class UsbBusExtensionTest : public testing::Test {
public:
    void SetUp() override
    {
        DEVMGR_LOGD("UsbBusExtensionTest SetUp");
    }
    void TearDown() override
    {
        DEVMGR_LOGD("UsbBusExtensionTest TearDown");
    }
};

static const vector<Metadata> g_testMetaDatas = {
    Metadata("bus", "usb", ""),
    Metadata("desc", "test usb driver extension", ""),
    Metadata("vendor", "testVendor", ""),
    Metadata("pid", "1234,5678", ""),
    Metadata("vid", "1111,2222", "")
};
HWTEST_F(UsbBusExtensionTest, GetExtensionInstanceTest, TestSize.Level1)
{
    auto usbBus = IBusExtension::GetInstance("usb");
    ASSERT_NE(usbBus, nullptr);
    auto bus2 = IBusExtension::GetInstance("unknow");
    ASSERT_EQ(bus2, nullptr);
}

HWTEST_F(UsbBusExtensionTest, SetDevChangeCallbackTest, TestSize.Level1)
{
    auto usbBus = IBusExtension::GetInstance("usb");
    ASSERT_NE(usbBus, nullptr);
    auto ret = usbBus->SetDevChangeCallback(nullptr);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(UsbBusExtensionTest, ParseDriverInfoTest, TestSize.Level1)
{
    DEVMGR_LOGI("PraseDriverInfoTest Start");
    auto usbBus = IBusExtension::GetInstance("usb");
    ASSERT_NE(usbBus, nullptr);
    auto driverInfoExt = usbBus->ParseDriverInfo(g_testMetaDatas);
    DEVMGR_LOGD("parse driver info done");
    ASSERT_NE(driverInfoExt, nullptr);
    UsbDriverInfo *usbDriverinfo = static_cast<UsbDriverInfo*>(driverInfoExt.get());
    ASSERT_NE(usbDriverinfo, nullptr);
    ASSERT_EQ(usbDriverinfo->pids.size(), (size_t)2);
    ASSERT_EQ(usbDriverinfo->vids.size(), (size_t)2);
    ASSERT_EQ(usbDriverinfo->pids[0], 1234);
    ASSERT_EQ(usbDriverinfo->pids[1], 5678);
    ASSERT_EQ(usbDriverinfo->vids[0], 1111);
    ASSERT_EQ(usbDriverinfo->vids[1], 2222);
}

HWTEST_F(UsbBusExtensionTest, MatchDriverTest, TestSize.Level1)
{
    auto usbDrvInfo = make_shared<UsbDriverInfo>();
    usbDrvInfo->pids.push_back(1234);
    usbDrvInfo->pids.push_back(5678);
    usbDrvInfo->vids.push_back(1111);
    usbDrvInfo->vids.push_back(2222);
    auto drvInfo = make_shared<DriverInfo>();
    drvInfo->bus = "USB";
    drvInfo->vendor = "TestVendor";
    drvInfo->version = "0.1.1";
    drvInfo->driverInfoExt = usbDrvInfo;
    string drvInfoStr;
    DEVMGR_LOGD("build driverInfo Done");

    auto deviceInfo = make_shared<UsbDeviceInfo>();
    deviceInfo->busType = "usb";
    deviceInfo->idProduct = 1234;
    deviceInfo->idVendor = 1111;
    deviceInfo->deviceClass = 0;
    deviceInfo->deviceId = "usb_XXX";
    deviceInfo->bcdUSB = 0x1122;

    auto usbBus = IBusExtension::GetInstance("usb");
    bool isMatched = usbBus->MatchDriver(*drvInfo, *deviceInfo);
    ASSERT_EQ(isMatched, true);

    UsbDeviceInfo deviceInfo2 = *deviceInfo;
    deviceInfo2.idProduct = 9999;
    isMatched = usbBus->MatchDriver(*drvInfo, deviceInfo2);
    ASSERT_EQ(isMatched, false);

    UsbDeviceInfo deviceInfo3 = *deviceInfo;
    deviceInfo3.idVendor = 9999;
    isMatched = usbBus->MatchDriver(*drvInfo, deviceInfo3);
    ASSERT_EQ(isMatched, false);

    UsbDeviceInfo deviceInfo4 = *deviceInfo;
    deviceInfo4.busType = "pcie";
    isMatched = usbBus->MatchDriver(*drvInfo, deviceInfo4);
    ASSERT_EQ(isMatched, false);

    drvInfo->bus = "peci";
    isMatched = usbBus->MatchDriver(*drvInfo, *deviceInfo);
    ASSERT_EQ(isMatched, false);
}
}
}
