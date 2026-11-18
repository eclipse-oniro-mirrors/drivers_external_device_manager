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

#include "scsi_peripheral_api.h"
#include "scsi_peripheral_types.h"
#include "gtest/gtest.h"

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {

constexpr int32_t EMULATOR_NOT_SUPPORT_CODE = 804;

class DdkScsiEmulatorTest : public testing::Test {};

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_Init_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_Init(), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_Release_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_Release(), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_Open_Emulator_001, TestSize.Level1)
{
    ScsiPeripheral_Device *dev = nullptr;
    EXPECT_EQ(OH_ScsiPeripheral_Open(1, 0, &dev), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_Close_Emulator_001, TestSize.Level1)
{
    ScsiPeripheral_Device *dev = nullptr;
    EXPECT_EQ(OH_ScsiPeripheral_Close(&dev), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_TestUnitReady_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_TestUnitReady(nullptr, nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_Inquiry_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_Inquiry(nullptr, nullptr, nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_ReadCapacity10_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_ReadCapacity10(nullptr, nullptr, nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_RequestSense_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_RequestSense(nullptr, nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_Read10_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_Read10(nullptr, nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_Write10_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_Write10(nullptr, nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_Verify10_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_Verify10(nullptr, nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_SendRequestByCdb_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_SendRequestByCdb(nullptr, nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_CreateDeviceMemMap_Emulator_001, TestSize.Level1)
{
    ScsiPeripheral_DeviceMemMap *devMmap = nullptr;
    EXPECT_EQ(OH_ScsiPeripheral_CreateDeviceMemMap(nullptr, 1024, &devMmap), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_DestroyDeviceMemMap_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_DestroyDeviceMemMap(nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkScsiEmulatorTest, OH_ScsiPeripheral_ParseBasicSenseInfo_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_ScsiPeripheral_ParseBasicSenseInfo(nullptr, 0, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}
} // ExternalDeviceManager
} // OHOS