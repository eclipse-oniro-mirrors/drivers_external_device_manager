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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include "usb_serial_api.h"
#include "v1_0/iusb_serial_ddk.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::HDI::Usb::UsbSerialDdk::V1_0;

namespace OHOS {
namespace HDI {
namespace Usb {
namespace UsbSerialDdk {
namespace V1_0 {

bool operator==(const UsbSerialDeviceHandle &lhs, const UsbSerialDeviceHandle &rhs)
{
    if (&lhs == &rhs) {
        return true;
    }
    return std::make_tuple(lhs.fd) == std::make_tuple(rhs.fd);
}
}}}}}

void SetDdk(OHOS::sptr<IUsbSerialDdk>&);
UsbSerial_Device *NewSerialDeviceHandle();

namespace {

constexpr int TEST_TIMES = 1;
constexpr uint32_t USB_SERIAL_TEST_BAUDRATE = 9600;
constexpr uint8_t USB_SERIAL_TEST_DATA_BITS = 8;

class MockUsbSerialDdk : public IUsbSerialDdk {
public:
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(int, Release, (), (override));
    MOCK_METHOD(int, Open, (uint64_t deviceId, uint64_t interfaceIndex, UsbSerialDeviceHandle& dev), (override));
    MOCK_METHOD(int, Close, (const UsbSerialDeviceHandle& dev), (override));
    MOCK_METHOD(int, Read, (const UsbSerialDeviceHandle& dev, uint32_t bufferSize, vector<uint8_t>& buff), (override));
    MOCK_METHOD(int, Write, (const UsbSerialDeviceHandle& dev, const vector<uint8_t>& buff,
        uint32_t& bytesWritten), (override));
    MOCK_METHOD(int, SetBaudRate, (const UsbSerialDeviceHandle& dev, uint32_t baudRate), (override));
    MOCK_METHOD(int, SetParams, (const UsbSerialDeviceHandle& dev, const UsbSerialParams& params), (override));
    MOCK_METHOD(int, SetTimeout, (const UsbSerialDeviceHandle& dev, int32_t timeout), (override));
    MOCK_METHOD(int, SetFlowControl, (const UsbSerialDeviceHandle& dev, UsbSerialFlowControl flowControl), (override));
    MOCK_METHOD(int, Flush, (const UsbSerialDeviceHandle& dev), (override));
    MOCK_METHOD(int, FlushInput, (const UsbSerialDeviceHandle& dev), (override));
    MOCK_METHOD(int, FlushOutput, (const UsbSerialDeviceHandle& dev), (override));
};

class UsbSerialTest : public testing::Test {
};

HWTEST_F(UsbSerialTest, ReleaseErrorTest, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Release())
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    ASSERT_EQ(OH_UsbSerial_Release(), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, OpenErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Open(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    UsbSerial_Device *dev = nullptr;;
    ASSERT_EQ(OH_UsbSerial_Open(0, 0, &dev), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, OpenErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Open(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    UsbSerial_Device *dev = nullptr;;
    ASSERT_EQ(OH_UsbSerial_Open(0, 0, &dev), USB_SERIAL_DDK_MEMORY_ERROR);
}

HWTEST_F(UsbSerialTest, OpenErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Open(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    UsbSerial_Device *dev = nullptr;;
    ASSERT_EQ(OH_UsbSerial_Open(0, 0, &dev), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, CloseErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Close(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    ASSERT_EQ(OH_UsbSerial_Close(&dev), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, CloseErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Close(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    ASSERT_EQ(OH_UsbSerial_Close(&dev), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, ReadErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Read(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint8_t dataBuff[8];
    uint32_t bytesRead = 0;
    ASSERT_EQ(OH_UsbSerial_Read(dev, dataBuff, sizeof(dataBuff), &bytesRead), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, ReadErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Read(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint8_t dataBuff[8];
    uint32_t bytesRead = 0;
    ASSERT_EQ(OH_UsbSerial_Read(dev, dataBuff, sizeof(dataBuff), &bytesRead), USB_SERIAL_DDK_MEMORY_ERROR);
}

HWTEST_F(UsbSerialTest, ReadErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Read(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint8_t dataBuff[8];
    uint32_t bytesRead = 0;
    ASSERT_EQ(OH_UsbSerial_Read(dev, dataBuff, sizeof(dataBuff), &bytesRead), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, WriteErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Write(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint8_t writeBuff[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
    uint32_t bytesWritten = 0;
    ASSERT_EQ(OH_UsbSerial_Write(dev, writeBuff, sizeof(writeBuff), &bytesWritten), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, WriteErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Write(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_MEMORY_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint8_t writeBuff[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
    uint32_t bytesWritten = 0;
    ASSERT_EQ(OH_UsbSerial_Write(dev, writeBuff, sizeof(writeBuff), &bytesWritten), USB_SERIAL_DDK_MEMORY_ERROR);
}

HWTEST_F(UsbSerialTest, WriteErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Write(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint8_t writeBuff[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
    uint32_t bytesWritten = 0;
    ASSERT_EQ(OH_UsbSerial_Write(dev, writeBuff, sizeof(writeBuff), &bytesWritten), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, SetBaudRateErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SetBaudRate(testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint32_t baudRate = 9600;
    ASSERT_EQ(OH_UsbSerial_SetBaudRate(dev, baudRate), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, SetBaudRateErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SetBaudRate(testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint32_t baudRate = 9600;
    ASSERT_EQ(OH_UsbSerial_SetBaudRate(dev, baudRate), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, SetParamsRateErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SetParams(testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    UsbSerial_Params serialParams;
    serialParams.baudRate = USB_SERIAL_TEST_BAUDRATE;
    serialParams.nDataBits = USB_SERIAL_TEST_DATA_BITS;
    serialParams.nStopBits = 1;
    serialParams.parity = 1;
    ASSERT_EQ(OH_UsbSerial_SetParams(dev, &serialParams), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, SetParamsRateErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SetParams(testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    UsbSerial_Params serialParams;
    serialParams.baudRate = USB_SERIAL_TEST_BAUDRATE;
    serialParams.nDataBits = USB_SERIAL_TEST_DATA_BITS;
    serialParams.nStopBits = 1;
    serialParams.parity = 1;
    ASSERT_EQ(OH_UsbSerial_SetParams(dev, &serialParams), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, SetTimeoutErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SetTimeout(testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint32_t readTimeout = 1; // 0 means no timeout
    ASSERT_EQ(OH_UsbSerial_SetTimeout(dev, readTimeout), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, SetTimeoutErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SetTimeout(testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    uint32_t readTimeout = 1; // 0 means no timeout
    ASSERT_EQ(OH_UsbSerial_SetTimeout(dev, readTimeout), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, SetFlowControlErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SetFlowControl(testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    UsbSerial_FlowControl flowControl = ::USB_SERIAL_NO_FLOW_CONTROL; // 0 means no flow control
    ASSERT_EQ(OH_UsbSerial_SetFlowControl(dev, flowControl), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, SetFlowControlErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SetFlowControl(testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    UsbSerial_FlowControl flowControl = ::USB_SERIAL_NO_FLOW_CONTROL; // 0 means no flow control
    ASSERT_EQ(OH_UsbSerial_SetFlowControl(dev, flowControl), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, FlushErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Flush(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    ASSERT_EQ(OH_UsbSerial_Flush(dev), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, FlushErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Flush(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    ASSERT_EQ(OH_UsbSerial_Flush(dev), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, FlushInputErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, FlushInput(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    ASSERT_EQ(OH_UsbSerial_FlushInput(dev), USB_SERIAL_DDK_IO_ERROR);
}

HWTEST_F(UsbSerialTest, FlushInputErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, FlushInput(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    ASSERT_EQ(OH_UsbSerial_FlushInput(dev), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, FlushOutputErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, FlushOutput(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_SERVICE_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    ASSERT_EQ(OH_UsbSerial_FlushOutput(dev), USB_SERIAL_DDK_SERVICE_ERROR);
}

HWTEST_F(UsbSerialTest, FlushOutputErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockUsbSerialDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, FlushOutput(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(USB_SERIAL_DDK_IO_ERROR));
        auto ddk = OHOS::sptr<IUsbSerialDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewSerialDeviceHandle();
    ASSERT_EQ(OH_UsbSerial_FlushOutput(dev), USB_SERIAL_DDK_IO_ERROR);
}
} // namespace
