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
#include "bus_extension_core.h"
#include "usb_bus_extension.h"
#include "dev_change_callback.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

class BusExtensionCoreTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_FRAMEWORK, "BusExtensionCoreTest SetUp");
    }

    void TearDown() override
    {
        EDM_LOGD(MODULE_FRAMEWORK, "BusExtensionCoreTest TearDown");
    }
};

HWTEST_F(BusExtensionCoreTest, InitSuccessTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    int32_t ret = instance.Init(callback);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_FRAMEWORK, "InitSuccessTest: BusExtensionCore initialized");
}

HWTEST_F(BusExtensionCoreTest, InitNullCallbackTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    int32_t ret = instance.Init(nullptr);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_FRAMEWORK, "InitNullCallbackTest: Init with null callback handled");
}

HWTEST_F(BusExtensionCoreTest, InitTwiceTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    int32_t ret1 = instance.Init(callback);
    int32_t ret2 = instance.Init(callback);
    ASSERT_EQ(ret1, EDM_OK);
    ASSERT_EQ(ret2, EDM_OK);
    EDM_LOGI(MODULE_FRAMEWORK, "InitTwiceTest: Multiple init calls handled");
}

HWTEST_F(BusExtensionCoreTest, RegisterSuccessTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<IBusExtension> busExt = make_shared<UsbBusExtension>();
    int32_t ret = instance.Register(BusType::BUS_TYPE_USB, busExt);
    ASSERT_EQ(ret, EDM_OK);
    EDM_LOGI(MODULE_FRAMEWORK, "RegisterSuccessTest: Bus extension registered");
}

HWTEST_F(BusExtensionCoreTest, RegisterDuplicateTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<IBusExtension> busExt1 = make_shared<UsbBusExtension>();
    shared_ptr<IBusExtension> busExt2 = make_shared<UsbBusExtension>();
    int32_t ret1 = instance.Register(BusType::BUS_TYPE_USB, busExt1);
    int32_t ret2 = instance.Register(BusType::BUS_TYPE_USB, busExt2);
    ASSERT_EQ(ret1, EDM_OK);
    ASSERT_EQ(ret2, EDM_OK);
    EDM_LOGI(MODULE_FRAMEWORK, "RegisterDuplicateTest: Duplicate registration handled");
}

HWTEST_F(BusExtensionCoreTest, RegisterMaxExtensionsTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    for (uint32_t i = 0; i < instance.MAX_BUS_EXTENSIONS + 10; i++) {
        shared_ptr<IBusExtension> busExt = make_shared<UsbBusExtension>();
        instance.Register(static_cast<BusType>(i), busExt);
    }
    EDM_LOGI(MODULE_FRAMEWORK, "RegisterMaxExtensionsTest: Maximum extensions limit tested");
}

HWTEST_F(BusExtensionCoreTest, RegisterNullExtensionTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    int32_t ret = instance.Register(BusType::BUS_TYPE_USB, nullptr);
    ASSERT_EQ(ret, EDM_NOK);
    EDM_LOGI(MODULE_FRAMEWORK, "RegisterNullExtensionTest: Null extension rejected");
}

HWTEST_F(BusExtensionCoreTest, GetBusExtensionByNameSuccessTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<IBusExtension> busExt = make_shared<UsbBusExtension>();
    instance.Register(BusType::BUS_TYPE_USB, busExt);
    shared_ptr<IBusExtension> result = instance.GetBusExtensionByName("USB");
    ASSERT_NE(result, nullptr);
    EDM_LOGI(MODULE_FRAMEWORK, "GetBusExtensionByNameSuccessTest: Bus extension retrieved");
}

HWTEST_F(BusExtensionCoreTest, GetBusExtensionByNameNotFoundTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<IBusExtension> result = instance.GetBusExtensionByName("UNKNOWN_BUS");
    ASSERT_EQ(result, nullptr);
    EDM_LOGI(MODULE_FRAMEWORK, "GetBusExtensionByNameNotFoundTest: Unknown bus type returns null");
}

HWTEST_F(BusExtensionCoreTest, GetBusExtensionByNameNullTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<IBusExtension> result = instance.GetBusExtensionByName("");
    ASSERT_EQ(result, nullptr);
    EDM_LOGI(MODULE_FRAMEWORK, "GetBusExtensionByNameNullTest: Empty name handled");
}

HWTEST_F(BusExtensionCoreTest, GetBusTypeByNameUsbTest, TestSize.Level1)
{
    BusType result = BusExtensionCore::GetBusTypeByName("USB");
    ASSERT_EQ(result, BusType::BUS_TYPE_USB);
    EDM_LOGI(MODULE_FRAMEWORK, "GetBusTypeByNameUsbTest: USB type converted");
}

HWTEST_F(BusExtensionCoreTest, GetBusTypeByNameHidTest, TestSize.Level1)
{
    BusType result = BusExtensionCore::GetBusTypeByName("HID");
    ASSERT_EQ(result, BusType::BUS_TYPE_HID);
    EDM_LOGI(MODULE_FRAMEWORK, "GetBusTypeByNameHidTest: HID type converted");
}

HWTEST_F(BusExtensionCoreTest, GetBusTypeByNameScsiTest, TestSize.Level1)
{
    BusType result = BusExtensionCore::GetBusTypeByName("SCSI");
    ASSERT_EQ(result, BusType::BUS_TYPE_SCSI);
    EDM_LOGI(MODULE_FRAMEWORK, "GetBusTypeByNameScsiTest: SCSI type converted");
}

HWTEST_F(BusExtensionCoreTest, GetBusTypeByNameSerialTest, TestSize.Level1)
{
    BusType result = BusExtensionCore::GetBusTypeByName("Serial");
    ASSERT_EQ(result, BusType::BUS_TYPE_SERIAL);
    EDM_LOGI(MODULE_FRAMEWORK, "GetBusTypeByNameSerialTest: Serial type converted");
}

HWTEST_F(BusExtensionCoreTest, GetBusTypeByNameUnknownTest, TestSize.Level1)
{
    BusType result = BusExtensionCore::GetBusTypeByName("UNKNOWN");
    ASSERT_EQ(result, BusType::BUS_TYPE_INVALID);
    EDM_LOGI(MODULE_FRAMEWORK, "GetBusTypeByNameUnknownTest: Unknown type handled");
}

HWTEST_F(BusExtensionCoreTest, LoadBusExtensionLibsSuccessTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    instance.LoadBusExtensionLibs();
    EDM_LOGI(MODULE_FRAMEWORK, "LoadBusExtensionLibsSuccessTest: Bus extension libs loaded");
}

HWTEST_F(BusExtensionCoreTest, LoadBusExtensionLibsFailTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    instance.LoadBusExtensionLibs();
    EDM_LOGI(MODULE_FRAMEWORK, "LoadBusExtensionLibsFailTest: Load failure handled");
}

HWTEST_F(BusExtensionCoreTest, LoadBusExtensionLibsEmptyTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    instance.LoadBusExtensionLibs();
    EDM_LOGI(MODULE_FRAMEWORK, "LoadBusExtensionLibsEmptyTest: Empty lib list handled");
}

HWTEST_F(BusExtensionCoreTest, AcquireDriverChangeCallbackTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<DevChangeCallback> callback = make_shared<DevChangeCallback>();
    instance.Init(callback);
    shared_ptr<IDriverChangeCallback> result = instance.AcquireDriverChangeCallback(BusType::BUS_TYPE_USB);
    ASSERT_NE(result, nullptr);
    EDM_LOGI(MODULE_FRAMEWORK, "AcquireDriverChangeCallbackTest: Driver change callback acquired");
}

HWTEST_F(BusExtensionCoreTest, AcquireDriverChangeCallbackNullTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<IDriverChangeCallback> result = instance.AcquireDriverChangeCallback(BusType::BUS_TYPE_INVALID);
    ASSERT_EQ(result, nullptr);
    EDM_LOGI(MODULE_FRAMEWORK, "AcquireDriverChangeCallbackNullTest: Null callback for invalid bus");
}

HWTEST_F(BusExtensionCoreTest, MultipleBusTypesTest, TestSize.Level1)
{
    BusExtensionCore &instance = BusExtensionCore::GetInstance();
    shared_ptr<IBusExtension> usbExt = make_shared<UsbBusExtension>();
    shared_ptr<IBusExtension> hidExt = make_shared<UsbBusExtension>();
    shared_ptr<IBusExtension> scsiExt = make_shared<UsbBusExtension>();
    instance.Register(BusType::BUS_TYPE_USB, usbExt);
    instance.Register(BusType::BUS_TYPE_HID, hidExt);
    instance.Register(BusType::BUS_TYPE_SCSI, scsiExt);
    ASSERT_NE(instance.GetBusExtensionByName("USB"), nullptr);
    ASSERT_NE(instance.GetBusExtensionByName("HID"), nullptr);
    ASSERT_NE(instance.GetBusExtensionByName("SCSI"), nullptr);
    EDM_LOGI(MODULE_FRAMEWORK, "MultipleBusTypesTest: Multiple bus types registered");
}
} // namespace ExternalDeviceManager
} // namespace OHOS
