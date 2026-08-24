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
#include <vector>
#include "usb_config_desc_parser.h"
#include "usb_ddk_types.h"

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {

class UsbConfigDescParserTest : public testing::Test {
public:
    void TearDown() override {
        if (config_ != nullptr)
        {
            FreeUsbConfigDescriptor(config_);
            config_ = nullptr;
        }
    }

protected:
    UsbDdkConfigDescriptor *config_ = nullptr;
};

static std::vector<uint8_t> BuildValidConfigDescriptor()
{
    //USB Config Descriptor (9 bytes)
    std::vector<uint8_t> buf = {
        0x09,       //bLength
        0x02,       //bDescriptorType = CONFIG
        0x12, 0x00, //wTotalLength = 18 (config 9 + interface 9)
        0x01,       //bNumInterfaces = 1
        0x00,       //bConfigurationValue
        0x80,       //bmAttributes
        0x32,       //bMaxPower
        // USB Interface Descriptor (9 bytes)
        0x09,       //bLength
        0x04,       //bDescriptorType = INTERFACE
        0x00,       //bInterfaceNumber = 0
        0x00,       //bAlternatSetting = 0
        0x00,       //bNumEndpoints = 0
        0xFF,       //bInterfaceClass = vendor
        0xFF,       //bInterfaceSubClass
        0xFF,       //bInterfaceProtocol
        0X00,       //iInterface
    };
    return buf;
}

HWTEST_F(UsbConfigDescParsetTest, ParseUsbConfigDescriptor_BlengthZero_001, TestSize.Level1)
{
    auto buf = BuildValidConfigDescriptor();
    buf[2] = 0x14;
    buf[3] = 0x00;

    buf.insert(buf.end(), {0x00, 0x21});

    int32_t ret = ParseUsbConfigDescriptor(buf, &config_);
    EXPECT_TRUE(ret != USB_DDK_SUCCESS || config_ != nullptr);
}

HWTEST_F(UsbConfigDescParsetTest, ParseUsbConfigDescriptor_BlengthOne_001, TestSize.Level1)
{
    auto buf = BuildValidConfigDescriptor();
    buf[2] = 0x14;
    buf[3] = 0x00;

    buf.insert(buf.end(), {0x01, 0x21});

    int32_t ret = ParseUsbConfigDescriptor(buf, &config_);
    EXPECT_TRUE(ret != USB_DDK_SUCCESS || config_ != nullptr);
}

HWTEST_F(UsbConfigDescParsetTest, ParseUsbConfigDescriptor_BlengthOverflow_001, TestSize.Level1)
{
    auto buf = BuildValidConfigDescriptor();
    buf[2] = 0x14;
    buf[3] = 0x00;

    buf.insert(buf.end(), {0xFF, 0x21});

    int32_t ret = ParseUsbConfigDescriptor(buf, &config_);
    EXPECT_TRUE(ret != USB_DDK_SUCCESS || config_ != nullptr);
}

HWTEST_F(UsbConfigDescParsetTest, ParseUsbConfigDescriptor_ValidDescriptor_001, TestSize.Level1)
{
    auto buf = BuildValidConfigDescriptor();
    int32_t ret = ParseUsbConfigDescriptor(buf, &config_);
    EXPECT_EQ(ret, USB_DDK_SUCCESS);
    ASSERT_NE(config_, nullptr);
    EXPECT_EQ(config_->configDescriptor.bNumInterfaces);
}

} // namespace ExternalDeviceManager
} // namespace OHOS
