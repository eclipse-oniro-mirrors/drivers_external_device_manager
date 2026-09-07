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
#include "scsi_peripheral_api.h"
#include "scsi_peripheral_types.h"
#include "v1_0/iscsi_peripheral_ddk.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::HDI::Usb::ScsiDdk::V1_0;

namespace OHOS {
namespace HDI {
namespace Usb {
namespace ScsiDdk {
namespace V1_0 {

bool operator==(const ScsiPeripheralDevice &lhs, const ScsiPeripheralDevice &rhs)
{
    if (&lhs == &rhs) {
        return true;
    }
    return std::make_tuple(lhs.devFd, lhs.memMapFd, lhs.lbLength) ==
        std::make_tuple(rhs.devFd, rhs.memMapFd, rhs.lbLength);
}
}}}}}

void SetDdk(OHOS::sptr<IScsiPeripheralDdk>&);
ScsiPeripheral_Device *NewScsiPeripheralDevice();
void DeleteScsiPeripheralDevice(ScsiPeripheral_Device **dev);

namespace {

constexpr int TEST_TIMES = 1;
constexpr uint8_t CDB_LENGTH  = 1;
constexpr size_t BUFFER_SIZE = 1024;

class MockScsiPeripheralDdk : public IScsiPeripheralDdk {
public:
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(int, Release, (), (override));
    MOCK_METHOD(int, Open, (uint64_t deviceId, uint8_t interfaceIndex, ScsiPeripheralDevice &dev, int &memMapFd),
        (override));
    MOCK_METHOD(int, Close, (const ScsiPeripheralDevice &dev), (override));
    MOCK_METHOD(int, ReadCapacity10, (const ScsiPeripheralDevice &dev, const ScsiPeripheralReadCapacityRequest &request,
        ScsiPeripheralCapacityInfo &capacityInfo, ScsiPeripheralResponse &response), (override));
    MOCK_METHOD(int, TestUnitReady, (const ScsiPeripheralDevice &dev, const ScsiPeripheralTestUnitReadyRequest &request,
        ScsiPeripheralResponse &response), (override));
    MOCK_METHOD(int, Inquiry, (const ScsiPeripheralDevice &dev, const ScsiPeripheralInquiryRequest &request,
        ScsiPeripheralInquiryInfo &inquiryInfo, ScsiPeripheralResponse &response), (override));
    MOCK_METHOD(int, RequestSense, (const ScsiPeripheralDevice &dev, const ScsiPeripheralRequestSenseRequest &request,
        ScsiPeripheralResponse &response), (override));
    MOCK_METHOD(int, Read10, (const ScsiPeripheralDevice &dev, const ScsiPeripheralIORequest &request,
        ScsiPeripheralResponse &response), (override));
    MOCK_METHOD(int, Write10, (const ScsiPeripheralDevice &dev, const ScsiPeripheralIORequest &request,
        ScsiPeripheralResponse &response), (override));
    MOCK_METHOD(int, Verify10, (const ScsiPeripheralDevice &dev, const ScsiPeripheralVerifyRequest &request,
        ScsiPeripheralResponse &response), (override));
    MOCK_METHOD(int, SendRequestByCDB, (const ScsiPeripheralDevice &dev, const ScsiPeripheralRequest &request,
        ScsiPeripheralResponse &response), (override));
};

class ScsiPeripheralTest : public testing::Test {
protected:
    void TearDown() override
    {
        OHOS::sptr<IScsiPeripheralDdk> nullDdk = nullptr;
        SetDdk(nullDdk);
    }
};

HWTEST_F(ScsiPeripheralTest, ReleaseErrorTest, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Release())
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    ASSERT_EQ(OH_ScsiPeripheral_Release(), SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, OpenErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Open(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    ScsiPeripheral_Device *dev = nullptr;
    ASSERT_EQ(OH_ScsiPeripheral_Open(0, 0, &dev), SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, OpenErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Open(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    ScsiPeripheral_Device *dev = nullptr;
    ASSERT_EQ(OH_ScsiPeripheral_Open(0, 0, &dev), SCSIPERIPHERAL_DDK_MEMORY_ERROR);
}

HWTEST_F(ScsiPeripheralTest, OpenErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Open(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    ScsiPeripheral_Device *dev = nullptr;
    ASSERT_EQ(OH_ScsiPeripheral_Open(0, 0, &dev), SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, CloseErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Close(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    int ret = OH_ScsiPeripheral_Close(&dev);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, CloseErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Close(testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    int ret = OH_ScsiPeripheral_Close(&dev);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, ReadCapacity10ErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, ReadCapacity10(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_ReadCapacityRequest request = {0};
    ScsiPeripheral_CapacityInfo capacityInfo = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_ReadCapacity10(dev, &request, &capacityInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, ReadCapacity10ErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, ReadCapacity10(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_ReadCapacityRequest request = {0};
    ScsiPeripheral_CapacityInfo capacityInfo = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_ReadCapacity10(dev, &request, &capacityInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, ReadCapacity10ErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, ReadCapacity10(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_ReadCapacityRequest request = {0};
    ScsiPeripheral_CapacityInfo capacityInfo = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_ReadCapacity10(dev, &request, &capacityInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_MEMORY_ERROR);
}

HWTEST_F(ScsiPeripheralTest, ReadCapacity10ErrorTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, ReadCapacity10(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_TIMEOUT));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_ReadCapacityRequest request = {0};
    ScsiPeripheral_CapacityInfo capacityInfo = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_ReadCapacity10(dev, &request, &capacityInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_TIMEOUT);
}

HWTEST_F(ScsiPeripheralTest, TestUnitReadyErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, TestUnitReady(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_TestUnitReadyRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_TestUnitReady(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, TestUnitReadyErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, TestUnitReady(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_TestUnitReadyRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_TestUnitReady(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, TestUnitReadyErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, TestUnitReady(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_TestUnitReadyRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_TestUnitReady(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_MEMORY_ERROR);
}

HWTEST_F(ScsiPeripheralTest, TestUnitReadyErrorTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, TestUnitReady(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_TIMEOUT));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_TestUnitReadyRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_TestUnitReady(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_TIMEOUT);
}

HWTEST_F(ScsiPeripheralTest, InquiryErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Inquiry(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    inquiryInfo.data = &memMap;
    ScsiPeripheral_Response response = {{0}};

    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, InquiryErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Inquiry(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    inquiryInfo.data = &memMap;
    ScsiPeripheral_Response response = {{0}};

    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, InquiryErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Inquiry(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    inquiryInfo.data = &memMap;
    ScsiPeripheral_Response response = {{0}};

    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_MEMORY_ERROR);
}

HWTEST_F(ScsiPeripheralTest, InquiryErrorTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Inquiry(testing::_, testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_TIMEOUT));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    inquiryInfo.data = &memMap;
    ScsiPeripheral_Response response = {{0}};

    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_TIMEOUT);
}

/**
 * @tc.name: InquiryNullTest001
 * @tc.desc: Verify OH_ScsiPeripheral_Inquiry returns INVALID_PARAMETER when dev is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, InquiryNullTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    ScsiPeripheral_InquiryRequest request = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Inquiry(nullptr, &request, &inquiryInfo, &response);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: InquiryNullTest002
 * @tc.desc: Verify OH_ScsiPeripheral_Inquiry returns INVALID_PARAMETER when request is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, InquiryNullTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Inquiry(dev, nullptr, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: InquiryNullTest003
 * @tc.desc: Verify OH_ScsiPeripheral_Inquiry returns INVALID_PARAMETER when inquiryInfo is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, InquiryNullTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, nullptr, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: InquiryNullTest004
 * @tc.desc: Verify OH_ScsiPeripheral_Inquiry returns INVALID_PARAMETER when response is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, InquiryNullTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, nullptr);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: InquiryNullTest005
 * @tc.desc: Verify OH_ScsiPeripheral_Inquiry returns INVALID_PARAMETER when inquiryInfo->data is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, InquiryNullTest005, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: InquiryNullTest006
 * @tc.desc: Verify OH_ScsiPeripheral_Inquiry returns INVALID_PARAMETER when memMap.address is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, InquiryNullTest006, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    ScsiPeripheral_DeviceMemMap memMap = {nullptr, 0, 0, 0, 0};
    inquiryInfo.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: InquiryNullTest007
 * @tc.desc: Verify OH_ScsiPeripheral_Inquiry returns INVALID_PARAMETER when memMap.bufferLength is 0.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, InquiryNullTest007, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, 0, 0};
    inquiryInfo.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: InquiryNullTest008
 * @tc.desc: Verify OH_ScsiPeripheral_Inquiry returns INVALID_PARAMETER when memMap.offset is out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, InquiryNullTest008, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, BUFFER_SIZE + 1, BUFFER_SIZE, 0};
    inquiryInfo.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: InquiryNullTest009
 * @tc.desc: Verify OH_ScsiPeripheral_Inquiry returns INVALID_PARAMETER when memMap.bufferLength is out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, InquiryNullTest009, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_InquiryRequest request = {0};
    ScsiPeripheral_InquiryInfo inquiryInfo = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE + 1, 0};
    inquiryInfo.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Inquiry(dev, &request, &inquiryInfo, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

HWTEST_F(ScsiPeripheralTest, RequestSenseErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, RequestSense(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_RequestSenseRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_RequestSense(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, RequestSenseErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, RequestSense(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_RequestSenseRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_RequestSense(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, RequestSenseErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, RequestSense(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_RequestSenseRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_RequestSense(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_MEMORY_ERROR);
}

HWTEST_F(ScsiPeripheralTest, RequestSenseErrorTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, RequestSense(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_TIMEOUT));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_RequestSenseRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_RequestSense(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_TIMEOUT);
}

HWTEST_F(ScsiPeripheralTest, Read10ErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Read10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, Read10ErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Read10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_MEMORY_ERROR);
}

HWTEST_F(ScsiPeripheralTest, Read10ErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Read10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, Read10ErrorTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Read10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_TIMEOUT));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_TIMEOUT);
}

/**
 * @tc.name: Read10NullTest001
 * @tc.desc: Verify OH_ScsiPeripheral_Read10 returns INVALID_PARAMETER when dev is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Read10NullTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    ScsiPeripheral_IORequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(nullptr, &request, &response);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Read10NullTest002
 * @tc.desc: Verify OH_ScsiPeripheral_Read10 returns INVALID_PARAMETER when request is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Read10NullTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, nullptr, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Read10NullTest003
 * @tc.desc: Verify OH_ScsiPeripheral_Read10 returns INVALID_PARAMETER when response is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Read10NullTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, nullptr);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Read10NullTest004
 * @tc.desc: Verify OH_ScsiPeripheral_Read10 returns INVALID_PARAMETER when request->data is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Read10NullTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Read10NullTest005
 * @tc.desc: Verify OH_ScsiPeripheral_Read10 returns INVALID_PARAMETER when memMap.address is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Read10NullTest005, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    ScsiPeripheral_DeviceMemMap memMap = {nullptr, 0, 0, 0, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Read10NullTest006
 * @tc.desc: Verify OH_ScsiPeripheral_Read10 returns INVALID_PARAMETER when memMap.bufferLength is 0.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Read10NullTest006, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, 0, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Read10NullTest007
 * @tc.desc: Verify OH_ScsiPeripheral_Read10 returns INVALID_PARAMETER when memMap.offset is out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Read10NullTest007, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, BUFFER_SIZE + 1, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Read10NullTest008
 * @tc.desc: Verify OH_ScsiPeripheral_Read10 returns INVALID_PARAMETER when memMap.bufferLength is out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Read10NullTest008, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE + 1, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Read10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

HWTEST_F(ScsiPeripheralTest, Write10ErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Write10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, Write10ErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Write10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_MEMORY_ERROR);
}

HWTEST_F(ScsiPeripheralTest, Write10ErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Write10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, Write10ErrorTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Write10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_TIMEOUT));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_TIMEOUT);
}

/**
 * @tc.name: Write10NullTest001
 * @tc.desc: Verify OH_ScsiPeripheral_Write10 returns INVALID_PARAMETER when dev is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Write10NullTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    ScsiPeripheral_IORequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(nullptr, &request, &response);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Write10NullTest002
 * @tc.desc: Verify OH_ScsiPeripheral_Write10 returns INVALID_PARAMETER when request is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Write10NullTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, nullptr, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Write10NullTest003
 * @tc.desc: Verify OH_ScsiPeripheral_Write10 returns INVALID_PARAMETER when response is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Write10NullTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, nullptr);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Write10NullTest004
 * @tc.desc: Verify OH_ScsiPeripheral_Write10 returns INVALID_PARAMETER when request->data is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Write10NullTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Write10NullTest005
 * @tc.desc: Verify OH_ScsiPeripheral_Write10 returns INVALID_PARAMETER when memMap.address is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Write10NullTest005, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    ScsiPeripheral_DeviceMemMap memMap = {nullptr, 0, 0, 0, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Write10NullTest006
 * @tc.desc: Verify OH_ScsiPeripheral_Write10 returns INVALID_PARAMETER when memMap.bufferLength is 0.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Write10NullTest006, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, 0, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Write10NullTest007
 * @tc.desc: Verify OH_ScsiPeripheral_Write10 returns INVALID_PARAMETER when memMap.offset is out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Write10NullTest007, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, BUFFER_SIZE + 1, BUFFER_SIZE, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: Write10NullTest008
 * @tc.desc: Verify OH_ScsiPeripheral_Write10 returns INVALID_PARAMETER when memMap.bufferLength is out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, Write10NullTest008, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);

    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_IORequest request = {0};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE + 1, 0};
    request.data = &memMap;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Write10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

HWTEST_F(ScsiPeripheralTest, Verify10ErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Verify10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_VerifyRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Verify10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, Verify10ErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Verify10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_VerifyRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Verify10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, Verify10ErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Verify10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_VerifyRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Verify10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_MEMORY_ERROR);
}

HWTEST_F(ScsiPeripheralTest, Verify10ErrorTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, Verify10(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_TIMEOUT));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_VerifyRequest request = {0};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_Verify10(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_TIMEOUT);
}

HWTEST_F(ScsiPeripheralTest, SendRequestByCDBErrorTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SendRequestByCDB(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_SERVICE_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    request.cdbLength = CDB_LENGTH;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_SERVICE_ERROR);
}

HWTEST_F(ScsiPeripheralTest, SendRequestByCDBErrorTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SendRequestByCDB(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_MEMORY_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    request.cdbLength = CDB_LENGTH;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_MEMORY_ERROR);
}

HWTEST_F(ScsiPeripheralTest, SendRequestByCDBErrorTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SendRequestByCDB(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_IO_ERROR));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    request.cdbLength = CDB_LENGTH;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_IO_ERROR);
}

HWTEST_F(ScsiPeripheralTest, SendRequestByCDBErrorTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    EXPECT_CALL(*mockDdk, SendRequestByCDB(testing::_, testing::_, testing::_))
        .Times(TEST_TIMES)
        .WillOnce(testing::Return(SCSIPERIPHERAL_DDK_TIMEOUT));
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    request.cdbLength = CDB_LENGTH;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_TIMEOUT);
}

/**
 * @tc.name: SendRequestByCdbNullTest001
 * @tc.desc: Verify OH_ScsiPeripheral_SendRequestByCdb returns INVALID_PARAMETER when dev is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, SendRequestByCdbNullTest001, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    ScsiPeripheral_Request request = {{0}};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(nullptr, &request, &response);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: SendRequestByCdbNullTest002
 * @tc.desc: Verify OH_ScsiPeripheral_SendRequestByCdb returns INVALID_PARAMETER when request is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, SendRequestByCdbNullTest002, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, nullptr, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: SendRequestByCdbNullTest003
 * @tc.desc: Verify OH_ScsiPeripheral_SendRequestByCdb returns INVALID_PARAMETER when response is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, SendRequestByCdbNullTest003, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, nullptr);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: SendRequestByCdbNullTest004
 * @tc.desc: Verify OH_ScsiPeripheral_SendRequestByCdb returns INVALID_PARAMETER when request->data is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, SendRequestByCdbNullTest004, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: SendRequestByCdbNullTest005
 * @tc.desc: Verify OH_ScsiPeripheral_SendRequestByCdb returns INVALID_PARAMETER when request->cdbLength is 0.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, SendRequestByCdbNullTest005, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE, 0};
    request.data = &memMap;
    request.cdbLength = 0;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: SendRequestByCdbNullTest006
 * @tc.desc: Verify OH_ScsiPeripheral_SendRequestByCdb returns INVALID_PARAMETER when memMap.address is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, SendRequestByCdbNullTest006, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    ScsiPeripheral_DeviceMemMap memMap = {nullptr, 0, 0, 0, 0};
    request.data = &memMap;
    request.cdbLength = CDB_LENGTH;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: SendRequestByCdbNullTest007
 * @tc.desc: Verify OH_ScsiPeripheral_SendRequestByCdb returns INVALID_PARAMETER when memMap.bufferLength is 0.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, SendRequestByCdbNullTest007, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, 0, 0};
    request.data = &memMap;
    request.cdbLength = CDB_LENGTH;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: SendRequestByCdbNullTest008
 * @tc.desc: Verify OH_ScsiPeripheral_SendRequestByCdb returns INVALID_PARAMETER when memMap.offset is out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, SendRequestByCdbNullTest008, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, BUFFER_SIZE + 1, BUFFER_SIZE, 0};
    request.data = &memMap;
    request.cdbLength = CDB_LENGTH;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}

/**
 * @tc.name: SendRequestByCdbNullTest009
 * @tc.desc: Verify OH_ScsiPeripheral_SendRequestByCdb returns INVALID_PARAMETER memMap.bufferLength is out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(ScsiPeripheralTest, SendRequestByCdbNullTest009, TestSize.Level1)
{
    auto mockDdk = OHOS::sptr<MockScsiPeripheralDdk>::MakeSptr();
    ASSERT_NE(mockDdk, nullptr);
    auto ddk = OHOS::sptr<IScsiPeripheralDdk>(mockDdk);
    SetDdk(ddk);
    auto dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    uint8_t buffer[BUFFER_SIZE] = {0};
    ScsiPeripheral_DeviceMemMap memMap = {buffer, BUFFER_SIZE, 0, BUFFER_SIZE + 1, 0};
    request.data = &memMap;
    request.cdbLength = CDB_LENGTH;
    ScsiPeripheral_Response response = {{0}};
    int ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    DeleteScsiPeripheralDevice(&dev);
    ASSERT_EQ(ret, SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
}
} // namespace
