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
#include <vector>

#include <linux/uinput.h>
#include "driver_ext_mgr_client.h"
#include "hilog_wrapper.h"


namespace OHOS {
namespace ExternalDeviceManager {
using namespace testing::ext;
class EmitEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override {};
    void TearDown() override {};

    static Hid_Device* CreateDeviceMock(int8_t isPropExMax);
    static Hid_EventProperties* CreateEventPropertiesMock(int8_t isEventTypeExMax, int8_t isKeyExMax, int8_t isAbsExMax,
        int8_t isRelExMax, int8_t isMscExMax);
    static DriverExtMgrClient &edmClient;
    static int32_t deviceId;
};

DriverExtMgrClient &EmitEventTest::edmClient = DriverExtMgrClient::GetInstance();
int32_t EmitEventTest::deviceId = -1;

void EmitEventTest::SetUpTestCase() {}

void EmitEventTest::TearDownTestCase() {}

Hid_Device* EmitEventTest::CreateDeviceMock(int8_t isPropExMax)
{
    Hid_DeviceProp hidDeviceProp[MAX_HID_DEVICE_PROP_LEN] = { HID_PROP_POINTER, HID_PROP_DIRECT };
    Hid_Device hidDevice = {
        .deviceName = "VSoC keyboard",
        .vendorId = 0x6006,
        .productId = 0x6008,
        .version = 1,
        .bustype = BUS_USB,
        .properties = hidDeviceProp,
        .propLength = isPropExMax ? (MAX_HID_DEVICE_PROP_LEN + 1) : 2
    };
    return &hidDevice;
}

Hid_EventProperties* EmitEventTest::CreateEventPropertiesMock(int8_t isEventTypeExMax, int8_t isKeyExMax,
    int8_t isAbsExMax, int8_t isRelExMax, int8_t isMscExMax)
{
    Hid_EventType hidEventType[MAX_HID_EVENT_TYPES_LEN] = { HID_EV_SYN, HID_EV_KEY };
    Hid_KeyCode hidKeyCode[MAX_HID_KEYS_LEN] = { HID_KEY_A, HID_KEY_B };
    Hid_AbsAxes hidAbsAxes[MAX_HID_ABS_LEN] = { HID_ABS_X, HID_ABS_Y };
    Hid_RelAxes hidRelAxes[MAX_HID_REL_BITS_LEN] = { HID_REL_X, HID_REL_Y };
    Hid_MscEvent hidMscEvent[MAX_HID_MISC_EVENT_LEN] = { HID_MSC_SERIAL, HID_MSC_PULSELED };

    Hid_EventProperties hidEventProperties = {
        .hidEventTypes = { hidEventType, isEventTypeExMax ? (MAX_HID_EVENT_TYPES_LEN + 1) : 2 },
        .hidKeys = { hidKeyCode, isKeyExMax ? (MAX_HID_KEYS_LEN + 1) : 2 },
        .hidAbs = { hidAbsAxes, isAbsExMax ? (MAX_HID_ABS_LEN + 1) : 2 },
        .hidRelBits = { hidRelAxes, isRelExMax ? (MAX_HID_REL_BITS_LEN + 1) : 2 },
        .hidMiscellaneous = { hidMscEvent, isMscExMax ? (MAX_HID_MISC_EVENT_LEN + 1) : 2 }
    };
    return &hidEventProperties;
}

HWTEST_F(EmitEventTest, EmitEvent001, TestSize.Level1)
{
    std::vector<Hid_EmitItem> items = {
        {1, 0x14a, 108},
        {3, 0,     50 },
        {3, 1,     50 }
    };
    auto ret = edmClient.EmitEvent(0, items);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(EmitEventTest, EmitEvent002, TestSize.Level1)
{
    const uint16_t len = 21;
    std::vector<Hid_EmitItem> items;
    Hid_EmitItem item = {1, 0x14a, 108};
    for (uint16_t i = 0; i < len; ++i) {
        items[i] = item;
    }
    auto ret = edmClient.EmitEvent(deviceId, items);
    ASSERT_NE(ret, 0);
}

HWTEST_F(EmitEventTest, EmitEvent003, TestSize.Level1)
{
    const uint16_t len = 20;
    std::vector<Hid_EmitItem> items;
    Hid_EmitItem item = {1, 0x14a, 108};
    for (uint16_t i = 0; i < len; ++i) {
        items[i] = item;
    }
    auto ret = edmClient.EmitEvent(deviceId, items);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(EmitEventTest, CreateDevice001, TestSize.Level1)
{
    auto device = CreateDeviceMock(0);
    auto eventProperties = CreateEventPropertiesMock(0, 0, 0, 0, 0);
    auto ret = EmitEventTest::edmClient.CreateDevice(device, eventProperties);
    ASSERT_GE(ret, 0);
    deviceId = ret;
}

HWTEST_F(EmitEventTest, CreateDevice002, TestSize.Level1)
{
    auto device = CreateDeviceMock(1);
    auto eventProperties = CreateEventPropertiesMock(0, 0, 0, 0, 0);
    auto ret = EmitEventTest::edmClient.CreateDevice(device, eventProperties);
    ASSERT_LT(ret, 0);
}

HWTEST_F(EmitEventTest, CreateDevice003, TestSize.Level1)
{
    auto device = CreateDeviceMock(0);
    auto eventProperties = CreateEventPropertiesMock(1, 0, 0, 0, 0);
    auto ret = EmitEventTest::edmClient.CreateDevice(device, eventProperties);
    ASSERT_LT(ret, 0);
}

HWTEST_F(EmitEventTest, CreateDevice004, TestSize.Level1)
{
    auto device = CreateDeviceMock(0);
    auto eventProperties = CreateEventPropertiesMock(0, 1, 0, 0, 0);
    auto ret = EmitEventTest::edmClient.CreateDevice(device, eventProperties);
    ASSERT_LT(ret, 0);
}

HWTEST_F(EmitEventTest, CreateDevice005, TestSize.Level1)
{
    auto device = CreateDeviceMock(0);
    auto eventProperties = CreateEventPropertiesMock(0, 0, 1, 0, 0);
    auto ret = EmitEventTest::edmClient.CreateDevice(device, eventProperties);
    ASSERT_LT(ret, 0);
}

HWTEST_F(EmitEventTest, CreateDevice006, TestSize.Level1)
{
    auto device = CreateDeviceMock(0);
    auto eventProperties = CreateEventPropertiesMock(0, 0, 0, 1, 0);
    auto ret = EmitEventTest::edmClient.CreateDevice(device, eventProperties);
    ASSERT_LT(ret, 0);
}

HWTEST_F(EmitEventTest, CreateDevice007, TestSize.Level1)
{
    auto device = CreateDeviceMock(0);
    auto eventProperties = CreateEventPropertiesMock(0, 0, 0, 0, 1);
    auto ret = EmitEventTest::edmClient.CreateDevice(device, eventProperties);
    ASSERT_LT(ret, 0);
}

HWTEST_F(EmitEventTest, CreateDevice008, TestSize.Level1)
{
    const int16_t num = 200;
    int16_t idx = deviceId < 0 ? 0 : 1;
    Hid_EventProperties *eventProperties;
    Hid_Device* device;
    int32_t ret;
    while (idx <= num) {
        device = CreateDeviceMock(0);
        eventProperties = CreateEventPropertiesMock(0, 0, 0, 0, 0);
        ret = EmitEventTest::edmClient.CreateDevice(device, eventProperties);
        if (ret >= 0) {
            idx++;
        }
    }
    device = CreateDeviceMock(0);
    eventProperties = CreateEventPropertiesMock(0, 0, 0, 0, 0);
    ret = EmitEventTest::edmClient.CreateDevice(device, eventProperties);
    ASSERT_LT(ret, 0);
}

HWTEST_F(EmitEventTest, DestroyDevice001, TestSize.Level1)
{
    int32_t ret = edmClient.DestroyDevice(deviceId);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(EmitEventTest, DestroyDevice002, TestSize.Level1)
{
    int32_t ret = edmClient.DestroyDevice(-1);
    ASSERT_NE(ret, 0);
}

HWTEST_F(EmitEventTest, DestroyDevice003, TestSize.Level1)
{
    const int16_t deviceId = 200;
    int32_t ret = edmClient.DestroyDevice(deviceId);
    ASSERT_NE(ret, 0);
}
} // namespace ExternalDeviceManager
} // namespace OHOS