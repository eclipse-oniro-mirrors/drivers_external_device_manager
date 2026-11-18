/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "usb_serial_api.h"
#include "usb_serial_types.h"
#include "gtest/gtest.h"

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {

constexpr int32_t EMULATOR_NOT_SUPPORT_CODE = 804;

class DdkUsbSerialEmulatorTest : public testing::Test {};

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_Init_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_UsbSerial_Init(), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_Release_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_UsbSerial_Release(), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_Open_Emulator_001, TestSize.Level1)
{
    UsbSerial_Device *dev = nullptr;
    EXPECT_EQ(OH_UsbSerial_Open(1, 0, &dev), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_Close_Emulator_001, TestSize.Level1)
{
    UsbSerial_Device *dev = nullptr;
    EXPECT_EQ(OH_UsbSerial_Close(&dev), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_Read_Emulator_001, TestSize.Level1)
{
    uint8_t buff[16] = {};
    uint32_t bytesRead = 0;
    EXPECT_EQ(OH_UsbSerial_Read(nullptr, buff, sizeof(buff), &bytesRead), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_Write_Emulator_001, TestSize.Level1)
{
    uint8_t buff[16] = {};
    uint32_t bytesWritten = 0;
    EXPECT_EQ(OH_UsbSerial_Write(nullptr, buff, sizeof(buff), &bytesWritten), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_SetBaudRate_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_UsbSerial_SetBaudRate(nullptr, 115200), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_SetParams_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_UsbSerial_SetParams(nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_SetTimeout_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_UsbSerial_SetTimeout(nullptr, 100), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_SetFlowControl_Emulator_001, TestSize.Level1)
{
    UsbSerial_FlowControl flowControl = {};
    EXPECT_EQ(OH_UsbSerial_SetFlowControl(nullptr, flowControl), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_Flush_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_UsbSerial_Flush(nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_FlushInput_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_UsbSerial_FlushInput(nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbSerialEmulatorTest, OH_UsbSerial_FlushOutput_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_UsbSerial_FlushOutput(nullptr), EMULATOR_NOT_SUPPORT_CODE);
}
} // ExternalDeviceManager
} // OHOS