/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "usb_ddk_api.h"
#include "usb_ddk_types.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
namespace Testing {

class UsbDdkApiTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGI(MODULE_USB_DDK, "UsbDdkApiTest SetUp");
        int32_t ret = OH_Usb_Init();
        EDM_LOGI(MODULE_USB_DDK, "OH_Usb_Init result: %{public}d", ret);
    }
    
    void TearDown() override
    {
        EDM_LOGI(MODULE_USB_DDK, "UsbDdkApiTest TearDown");
        OH_Usb_Release();
    }
};

HWTEST_F(UsbDdkApiTest, ControlTransferParameterValidationTest001, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    uint8_t data[64] = {0};
    uint32_t timeout = 1000;
    
    int32_t ret = OH_Usb_ControlTransfer(deviceId, nullptr, data, timeout);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer with null setup: %{public}d", ret);
}

HWTEST_F(UsbDdkApiTest, ControlTransferParameterValidationTest002, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x06;
    setup.wValue = 0x0100;
    setup.wIndex = 0x0000;
    setup.wLength = 0x0012;
    uint32_t timeout = 1000;
    
    int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, nullptr, timeout);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer with null data: %{public}d", ret);
}

HWTEST_F(UsbDdkApiTest, ControlTransferParameterValidationTest003, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    setup.bmRequestType = 0x00;
    setup.bRequest = 0x09;
    setup.wValue = 0x0001;
    setup.wIndex = 0x0000;
    setup.wLength = 0x0000;
    uint32_t timeout = 1000;
    
    int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, nullptr, timeout);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer with null data for write: %{public}d", ret);
}

HWTEST_F(UsbDdkApiTest, ControlTransferInvalidDeviceIdTest, testing::ext::TestSize.Level1)
{
    uint64_t invalidDeviceId = 0xFFFFFFFFFFFFFFFF;
    UsbControlRequestSetup setup = {0};
    uint8_t data[64] = {0};
    uint32_t timeout = 1000;
    
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x06;
    setup.wValue = 0x0100;
    setup.wIndex = 0x0000;
    setup.wLength = 0x0012;
    
    int32_t ret = OH_Usb_ControlTransfer(invalidDeviceId, &setup, data, timeout);
    EXPECT_LT(ret, 0);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer with invalid device ID: %{public}d", ret);
}

HWTEST_F(UsbDdkApiTest, GetNonRootHubsParameterValidationTest001, testing::ext::TestSize.Level1)
{
    int32_t ret = OH_Usb_GetNonRootHubs(nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
    EDM_LOGI(MODULE_USB_DDK, "GetNonRootHubs with null parameter: %{public}d", ret);
}

HWTEST_F(UsbDdkApiTest, GetNonRootHubsReturnValueTest001, testing::ext::TestSize.Level1)
{
    Usb_NonRootHubArray hubArray;
    hubArray.nonRootHubIds = new uint64_t[128];
    hubArray.num = 0;
    
    int32_t ret = OH_Usb_GetNonRootHubs(&hubArray);
    EXPECT_EQ(ret, USB_DDK_SUCCESS);
    EXPECT_GE(hubArray.num, 0);
    EDM_LOGI(MODULE_USB_DDK, "GetNonRootHubs success, num: %{public}u", hubArray.num);
    
    delete[] hubArray.nonRootHubIds;
}

HWTEST_F(UsbDdkApiTest, GetNonRootHubsMultipleCallsTest001, testing::ext::TestSize.Level1)
{
    Usb_NonRootHubArray hubArray1;
    hubArray1.nonRootHubIds = new uint64_t[128];
    hubArray1.num = 0;
    
    Usb_NonRootHubArray hubArray2;
    hubArray2.nonRootHubIds = new uint64_t[128];
    hubArray2.num = 0;
    
    int32_t ret1 = OH_Usb_GetNonRootHubs(&hubArray1);
    int32_t ret2 = OH_Usb_GetNonRootHubs(&hubArray2);
    
    EXPECT_EQ(ret1, USB_DDK_SUCCESS);
    EXPECT_EQ(ret2, USB_DDK_SUCCESS);
    EXPECT_EQ(hubArray1.num, hubArray2.num);
    
    EDM_LOGI(MODULE_USB_DDK, "Multiple calls - ret1: %{public}d, ret2: %{public}d, num1: %{public}u, num2: %{public}u",
             ret1, ret2, hubArray1.num, hubArray2.num);
    
    delete[] hubArray1.nonRootHubIds;
    delete[] hubArray2.nonRootHubIds;
}

HWTEST_F(UsbDdkApiTest, ControlTransferTimeoutTest001, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    uint8_t data[64] = {0};
    
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x06;
    setup.wValue = 0x0100;
    setup.wIndex = 0x0000;
    setup.wLength = 0x0012;
    
    int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, data, 0);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer with zero timeout: %{public}d", ret);
    
    ret = OH_Usb_ControlTransfer(deviceId, &setup, data, 10);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer with short timeout: %{public}d", ret);
    
    ret = OH_Usb_ControlTransfer(deviceId, &setup, data, 10000);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer with long timeout: %{public}d", ret);
}

HWTEST_F(UsbDdkApiTest, ControlTransferDifferentRequestTypesTest001, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    uint8_t data[256] = {0};
    uint32_t timeout = 1000;
    
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x08;
    setup.wValue = 0x0000;
    setup.wIndex = 0x0000;
    setup.wLength = 0x0001;
    
    int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, data, timeout);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer GET_CONFIGURATION: %{public}d", ret);
    
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x00;
    setup.wValue = 0x0000;
    setup.wIndex = 0x0000;
    setup.wLength = 0x0001;
    
    ret = OH_Usb_ControlTransfer(deviceId, &setup, data, timeout);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer GET_STATUS: %{public}d", ret);
}

HWTEST_F(UsbDdkApiTest, ControlTransferLargeBufferTest001, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    uint8_t data[512] = {0};
    uint32_t timeout = 1000;
    
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x06;
    setup.wValue = 0x0200;
    setup.wIndex = 0x0000;
    setup.wLength = 0x00FF;
    
    int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, data, timeout);
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer with large buffer: %{public}d", ret);
}

} // namespace Testing
} // namespace ExternalDeviceManager
} // namespace OHOS
