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

#include "hid_ddk_api.h"
#include "hid_ddk_types.h"
#include "gtest/gtest.h"

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {

constexpr int32_t EMULATOR_NOT_SUPPORT_CODE = 804;

class DdkHidEmulatorTest : public testing::Test {};

HWTEST_F(DdkHidEmulatorTest, OH_Hid_CreateDevice_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Hid_CreateDevice(nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_EmitEvent_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Hid_EmitEvent(0, nullptr, 0), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_DestroyDevice_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Hid_DestroyDevice(0), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_Init_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Hid_Init(), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_Release_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Hid_Release(), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_Open_Emulator_001, TestSize.Level1)
{
    Hid_DeviceHandle *dev = nullptr;
    EXPECT_EQ(OH_Hid_Open(1, 0, &dev), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_Close_Emulator_001, TestSize.Level1)
{
    Hid_DeviceHandle *dev = nullptr;
    EXPECT_EQ(OH_Hid_Close(&dev), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_Write_Emulator_001, TestSize.Level1)
{
    uint8_t data[16] = {};
    uint32_t bytesWritten = 0;
    EXPECT_EQ(OH_Hid_Write(nullptr, data, sizeof(data), &bytesWritten), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_ReadTimeout_Emulator_001, TestSize.Level1)
{
    uint8_t data[16] = {};
    uint32_t bytesRead = 0;
    EXPECT_EQ(OH_Hid_ReadTimeout(nullptr, data, sizeof(data), 100, &bytesRead), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_Read_Emulator_001, TestSize.Level1)
{
    uint8_t data[16] = {};
    uint32_t bytesRead = 0;
    EXPECT_EQ(OH_Hid_Read(nullptr, data, sizeof(data), &bytesRead), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_SetNonBlocking_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Hid_SetNonBlocking(nullptr, 1), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_GetRawInfo_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Hid_GetRawInfo(nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_GetRawName_Emulator_001, TestSize.Level1)
{
    char data[16] = {};
    EXPECT_EQ(OH_Hid_GetRawName(nullptr, data, sizeof(data)), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_GetPhysicalAddress_Emulator_001, TestSize.Level1)
{
    char data[16] = {};
    EXPECT_EQ(OH_Hid_GetPhysicalAddress(nullptr, data, sizeof(data)), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_GetRawUniqueId_Emulator_001, TestSize.Level1)
{
    uint8_t data[16] = {};
    EXPECT_EQ(OH_Hid_GetRawUniqueId(nullptr, data, sizeof(data)), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_SendReport_Emulator_001, TestSize.Level1)
{
    uint8_t data[16] = {};
    EXPECT_EQ(OH_Hid_SendReport(nullptr, static_cast<Hid_ReportType>(0), data, sizeof(data)),
        EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_GetReport_Emulator_001, TestSize.Level1)
{
    uint8_t data[16] = {};
    EXPECT_EQ(OH_Hid_GetReport(nullptr, static_cast<Hid_ReportType>(0), data, sizeof(data)),
        EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkHidEmulatorTest, OH_Hid_GetReportDescriptor_Emulator_001, TestSize.Level1)
{
    uint8_t buf[16] = {};
    uint32_t bytesRead = 0;
    EXPECT_EQ(OH_Hid_GetReportDescriptor(nullptr, buf, sizeof(buf), &bytesRead), EMULATOR_NOT_SUPPORT_CODE);
}
} // ExternalDeviceManager
} // OHOS