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

#include "usb_ddk_api.h"
#include "usb_ddk_types.h"
#include "gtest/gtest.h"

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {

constexpr int32_t EMULATOR_NOT_SUPPORT_CODE = 804;

class DdkUsbEmulatorTest : public testing::Test {};

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_Init_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Usb_Init(), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_Release_Emulator_001, TestSize.Level1)
{
    OH_Usb_Release();
    SUCCEED();
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_ReleaseResource_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Usb_ReleaseResource(), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_GetDeviceDescriptor_Emulator_001, TestSize.Level1)
{
    struct UsbDeviceDescriptor *desc = nullptr;
    EXPECT_EQ(OH_Usb_GetDeviceDescriptor(1, desc), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_GetConfigDescriptor_Emulator_001, TestSize.Level1)
{
    struct UsbDdkConfigDescriptor *config = nullptr;
    EXPECT_EQ(OH_Usb_GetConfigDescriptor(1, 0, &config), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_FreeConfigDescriptor_Emulator_001, TestSize.Level1)
{
    struct UsbDdkConfigDescriptor *config = nullptr;
    OH_Usb_FreeConfigDescriptor(config);
    SUCCEED();
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_ClaimInterface_Emulator_001, TestSize.Level1)
{
    uint64_t interfaceHandle = 0;
    EXPECT_EQ(OH_Usb_ClaimInterface(1, 0, &interfaceHandle), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_ReleaseInterface_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Usb_ReleaseInterface(0), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_SelectInterfaceSetting_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Usb_SelectInterfaceSetting(0, 0), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_GetCurrentInterfaceSetting_Emulator_001, TestSize.Level1)
{
    uint8_t settingIndex = 0;
    EXPECT_EQ(OH_Usb_GetCurrentInterfaceSetting(0, &settingIndex), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_SendControlReadRequest_Emulator_001, TestSize.Level1)
{
    uint8_t data[16] = {};
    uint32_t dataLen = sizeof(data);
    EXPECT_EQ(OH_Usb_SendControlReadRequest(0, nullptr, 1000, data, &dataLen), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_SendControlWriteRequest_Emulator_001, TestSize.Level1)
{
    uint8_t data[16] = {};
    EXPECT_EQ(OH_Usb_SendControlWriteRequest(0, nullptr, 1000, data, sizeof(data)), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_SendPipeRequest_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Usb_SendPipeRequest(nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_SendPipeRequestWithAshmem_Emulator_001, TestSize.Level1)
{
    EXPECT_EQ(OH_Usb_SendPipeRequestWithAshmem(nullptr, nullptr), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_CreateDeviceMemMap_Emulator_001, TestSize.Level1)
{
    UsbDeviceMemMap *devMmap = nullptr;
    EXPECT_EQ(OH_Usb_CreateDeviceMemMap(1, 1024, &devMmap), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_DestroyDeviceMemMap_Emulator_001, TestSize.Level1)
{
    UsbDeviceMemMap *devMmap = nullptr;
    OH_Usb_DestroyDeviceMemMap(devMmap);
    SUCCEED();
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_GetDevices_Emulator_001, TestSize.Level1)
{
    struct Usb_DeviceArray *devices = nullptr;
    EXPECT_EQ(OH_Usb_GetDevices(devices), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_ControlTransfer_Emulator_001, TestSize.Level1)
{
    uint8_t data[16] = {};
    EXPECT_EQ(OH_Usb_ControlTransfer(1, nullptr, data, 1000), EMULATOR_NOT_SUPPORT_CODE);
}

HWTEST_F(DdkUsbEmulatorTest, OH_Usb_GetNonRootHubs_Emulator_001, TestSize.Level1)
{
    struct Usb_NonRootHubArray *nonRootHub = nullptr;
    EXPECT_EQ(OH_Usb_GetNonRootHubs(nonRootHub), EMULATOR_NOT_SUPPORT_CODE);
}
} // ExternalDeviceManager
} // OHOS