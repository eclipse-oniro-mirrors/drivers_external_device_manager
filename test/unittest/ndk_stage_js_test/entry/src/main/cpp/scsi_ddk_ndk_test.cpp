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

#include "hilog/log.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "scsi_peripheral_api.h"
#include "scsi_peripheral_types.h"
#include <cstdlib>
#include <js_native_api_types.h>
#include <unistd.h>
#include <vector>

const uint32_t PARAM_0 = 0;
const uint32_t PARAM_1 = 1;
constexpr size_t DEVICE_MEM_MAP_SIZE = 10 * 1024; // 10K

const uint64_t SCSI_DDK_INVALID_DEVICE_ID = 0xFFFFFFFFFFFFFFFF;
const uint32_t SIXTEEN_BIT = 16;
const uint32_t THIRTYTWO_BIT = 32;
const uint32_t BUS_NUM_MASK = 0xFFFF0000;
const uint32_t DEVICE_NUM_MASK = 0x0000FFFF;
constexpr uint8_t ONE_BYTE = 1;
constexpr uint8_t TWO_BYTE = 2;
constexpr uint8_t THREE_BYTE = 3;
constexpr uint8_t FOUR_BYTE = 4;
constexpr uint8_t FIVE_BYTE = 5;
constexpr uint8_t SIX_BYTE = 6;
constexpr uint8_t SEVEN_BYTE = 7;
constexpr uint8_t EIGHT_BYTE = 8;
constexpr uint8_t NINE_BYTE = 9;
constexpr uint16_t MAX_MEM_LEN = 256;
constexpr uint32_t TIMEOUT = 5000;
constexpr uint32_t TIMEOUT2 = 20000;
constexpr uint8_t CDB_LENGTH_TEN = 10;

ScsiPeripheral_Device *NewScsiPeripheralDevice();
void DeleteScsiPeripheralDevice(ScsiPeripheral_Device **dev);

static uint64_t ConvertDeviceId(uint64_t deviceId64)
{
    int32_t deviceId32 = static_cast<uint32_t>(deviceId64 >> THIRTYTWO_BIT);
    uint32_t busNum = (deviceId32 & BUS_NUM_MASK) >> SIXTEEN_BIT;
    uint32_t deviceNum = deviceId32 & DEVICE_NUM_MASK;
    uint64_t deviceId = ((static_cast<uint64_t>(busNum) << THIRTYTWO_BIT) | deviceNum);

    return deviceId;
}

static uint64_t GetDeviceId(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_1;
    napi_value args[PARAM_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    int64_t tmpDeviceId;
    napi_get_value_int64(env, args[PARAM_0], &tmpDeviceId);
    uint64_t deviceId = ConvertDeviceId(tmpDeviceId);
    return deviceId;
}

static napi_value IsScsiDevice(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t ret = OH_ScsiPeripheral_Init();
    ScsiPeripheral_Device *dev = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &dev);
    bool boolRet = ret == SCSIPERIPHERAL_DDK_SUCCESS ? true : false;

    OH_ScsiPeripheral_Close(&dev);
    OH_ScsiPeripheral_Release();
    napi_value result = nullptr;
    napi_status status = napi_get_boolean(env, boolRet, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralInitOne(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralReleaseOne(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Release();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_Release failed");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralReleaseTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    ret = OH_ScsiPeripheral_Release();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Release failed");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralOpenOne(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    int32_t ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_Open failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralOpenTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    uint64_t deviceId = GetDeviceId(env, info);
    ret = OH_ScsiPeripheral_Open(deviceId, 0, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Open failed");

    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralOpenThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(SCSI_DDK_INVALID_DEVICE_ID, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_DEVICE_NOT_FOUND, "OH_ScsiPeripheral_Open failed");

    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralOpenFour(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralCloseOne(napi_env env, napi_callback_info info)
{
    ScsiPeripheral_Device *device = nullptr;
    int32_t ret = OH_ScsiPeripheral_Close(&device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_Close failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralCloseTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_Device **dev = nullptr;
    ret = OH_ScsiPeripheral_Close(dev);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Close failed");

    ScsiPeripheral_Device *dev2 = nullptr;
    ret = OH_ScsiPeripheral_Close(&dev2);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Close failed");

    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralCloseThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ret = OH_ScsiPeripheral_Close(&device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Close failed");

    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralReadCapacityOne(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_ReadCapacity10(nullptr, nullptr, nullptr, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_ReadCapacity10 failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralReadCapacityTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_Device *dev = nullptr;
    ScsiPeripheral_ReadCapacityRequest req;
    ScsiPeripheral_CapacityInfo capacityInfo;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_ReadCapacity10(dev, &req, &capacityInfo, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_ReadCapacity10 failed");

    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *dev2 = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &dev2);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_ReadCapacityRequest *req2 = nullptr;
    ScsiPeripheral_CapacityInfo capacityInfo2;
    ScsiPeripheral_Response resp2;
    ret = OH_ScsiPeripheral_ReadCapacity10(dev2, req2, &capacityInfo2, &resp2);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_ReadCapacity10 failed");

    ScsiPeripheral_Device *dev3 = dev2;
    ScsiPeripheral_ReadCapacityRequest req3;
    ScsiPeripheral_CapacityInfo *capacityInfo3 = nullptr;
    ScsiPeripheral_Response resp3;
    ret = OH_ScsiPeripheral_ReadCapacity10(dev3, &req3, capacityInfo3, &resp3);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_ReadCapacity10 failed");

    ScsiPeripheral_Device *dev4 = dev2;
    ScsiPeripheral_ReadCapacityRequest req4;
    ScsiPeripheral_CapacityInfo capacityInfo4;
    ScsiPeripheral_Response *resp4 = nullptr;
    ret = OH_ScsiPeripheral_ReadCapacity10(dev4, &req4, &capacityInfo4, resp4);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_ReadCapacity10 failed");

    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralReadCapacityThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    const uint32_t timeOut = 2000;
    ScsiPeripheral_ReadCapacityRequest req;
    req.lbAddress = 0;
    req.control = 0;
    req.byte8 = 0;
    req.timeout = timeOut;
    ScsiPeripheral_CapacityInfo capacityInfo;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_ReadCapacity10(device, &req, &capacityInfo, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_ReadCapacity10 failed");

    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralTestUnitReadyOne(napi_env env, napi_callback_info info)
{
    ScsiPeripheral_Device *device = nullptr;
    ScsiPeripheral_TestUnitReadyRequest req;
    ScsiPeripheral_Response resp;
    int32_t ret = OH_ScsiPeripheral_TestUnitReady(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_TestUnitReady failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralTestUnitReadyTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_Device *device = nullptr;
    ScsiPeripheral_TestUnitReadyRequest req;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_TestUnitReady(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_TestUnitReady failed");

    ScsiPeripheral_Device *dev = NewScsiPeripheralDevice();
    ret = OH_ScsiPeripheral_TestUnitReady(dev, nullptr, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_TestUnitReady failed");

    ret = OH_ScsiPeripheral_TestUnitReady(dev, &req, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_TestUnitReady failed");

    OH_ScsiPeripheral_Release();
    DeleteScsiPeripheralDevice(&dev);

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralTestUnitReadyThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_TestUnitReadyRequest req;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_TestUnitReady(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_TestUnitReady failed");

    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralInquiryOne(napi_env env, napi_callback_info info)
{
    ScsiPeripheral_InquiryRequest req;
    ScsiPeripheral_InquiryInfo inquiryInfo;
    ScsiPeripheral_Response resp;
    int32_t ret = OH_ScsiPeripheral_Inquiry(nullptr, &req, &inquiryInfo, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_Inquiry failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralInquiryTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_Device *dev = nullptr;
    ScsiPeripheral_InquiryRequest req;
    ScsiPeripheral_InquiryInfo inquiryInfo;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_Inquiry(dev, &req, &inquiryInfo, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Inquiry failed");

    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *dev2 = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &dev2);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_InquiryRequest *req2 = nullptr;
    ScsiPeripheral_InquiryInfo inquiryInfo2;
    ScsiPeripheral_Response resp2;
    ret = OH_ScsiPeripheral_Inquiry(dev2, req2, &inquiryInfo2, &resp2);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Inquiry failed");

    ScsiPeripheral_Device *dev3 = dev2;
    ScsiPeripheral_InquiryRequest req3;
    ScsiPeripheral_InquiryInfo *inquiryInfo3 = nullptr;
    ScsiPeripheral_Response resp3;
    ret = OH_ScsiPeripheral_Inquiry(dev3, &req3, inquiryInfo3, &resp3);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Inquiry failed");

    ScsiPeripheral_Device *dev4 = dev2;
    ScsiPeripheral_InquiryRequest req4;
    ScsiPeripheral_InquiryInfo inquiryInfo4;
    ScsiPeripheral_Response *resp4 = nullptr;
    ret = OH_ScsiPeripheral_Inquiry(dev4, &req4, &inquiryInfo4, resp4);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Inquiry failed");

    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralInquiryThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_DeviceMemMap *devMmap = nullptr;
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(device, DEVICE_MEM_MAP_SIZE, &devMmap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_CreateDeviceMemMap failed");
    ScsiPeripheral_InquiryInfo inquiryInfo;
    ScsiPeripheral_InquiryRequest req;
    req.timeout = TIMEOUT;
    inquiryInfo.data = devMmap;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_Inquiry(device, &req, &inquiryInfo, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Inquiry failed");

    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralRequestSenseOne(napi_env env, napi_callback_info info)
{
    ScsiPeripheral_Device *device = nullptr;
    ScsiPeripheral_RequestSenseRequest req;
    ScsiPeripheral_Response resp;
    int32_t ret = OH_ScsiPeripheral_RequestSense(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_RequestSense failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralRequestSenseTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_RequestSenseRequest req;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_RequestSense(nullptr, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_RequestSense failed");

    ScsiPeripheral_Device *dev = NewScsiPeripheralDevice();
    ret = OH_ScsiPeripheral_RequestSense(dev, nullptr, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_RequestSense failed");

    ret = OH_ScsiPeripheral_RequestSense(dev, &req, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_RequestSense failed");

    OH_ScsiPeripheral_Release();
    DeleteScsiPeripheralDevice(&dev);

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralRequestSenseThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_DeviceMemMap *devMmap = nullptr;
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(device, DEVICE_MEM_MAP_SIZE, &devMmap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_CreateDeviceMemMap failed");
    ScsiPeripheral_RequestSenseRequest req;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_RequestSense(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_RequestSense failed");

    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralReadOne(napi_env env, napi_callback_info info)
{
    ScsiPeripheral_Device *device = nullptr;
    ScsiPeripheral_IORequest req;
    ScsiPeripheral_Response resp;
    int32_t ret = OH_ScsiPeripheral_Read10(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_Read10 failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralReadTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_IORequest req;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_Read10(nullptr, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Read10 failed");

    ScsiPeripheral_Device *dev = NewScsiPeripheralDevice();
    ret = OH_ScsiPeripheral_Read10(dev, nullptr, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Read10 failed");

    ret = OH_ScsiPeripheral_Read10(dev, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Read10 failed");

    uint8_t buff;
    ScsiPeripheral_DeviceMemMap devMmap({&buff, sizeof(buff), 0, sizeof(buff), 0});
    req.data = &devMmap;
    ret = OH_ScsiPeripheral_Read10(dev, &req, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Read10 failed");

    OH_ScsiPeripheral_Release();
    DeleteScsiPeripheralDevice(&dev);

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralReadThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_DeviceMemMap *devMmap = nullptr;
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(device, DEVICE_MEM_MAP_SIZE, &devMmap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_CreateDeviceMemMap failed");
    NAPI_ASSERT(env, devMmap != nullptr, "devMmap is nullptr");
    ScsiPeripheral_IORequest req;

    const uint32_t tmpTimeout = TIMEOUT2;
    req.lbAddress = 1;
    req.transferLength = 1;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_Read10(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Read10 failed");

    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralWriteOne(napi_env env, napi_callback_info info)
{
    ScsiPeripheral_Device *device = nullptr;
    ScsiPeripheral_IORequest req;
    ScsiPeripheral_Response resp;
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_Write10 failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralWriteTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_IORequest req;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_Write10(nullptr, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Write10 failed");

    ScsiPeripheral_Device *dev = NewScsiPeripheralDevice();
    ret = OH_ScsiPeripheral_Write10(dev, nullptr, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Write10 failed");

    ret = OH_ScsiPeripheral_Write10(dev, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Write10 failed");

    uint8_t buff;
    ScsiPeripheral_DeviceMemMap devMmap({&buff, sizeof(buff), 0, sizeof(buff), 0});
    req.data = &devMmap;
    ret = OH_ScsiPeripheral_Write10(dev, &req, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Write10 failed");

    OH_ScsiPeripheral_Release();
    DeleteScsiPeripheralDevice(&dev);

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralWriteThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_DeviceMemMap *devMmap = nullptr;
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(device, DEVICE_MEM_MAP_SIZE, &devMmap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_CreateDeviceMemMap failed");
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = TIMEOUT2;
    req.lbAddress = 1;
    req.transferLength = 1;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Write10 failed");

    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralVerifyOne(napi_env env, napi_callback_info info)
{
    ScsiPeripheral_Device *device = nullptr;
    ScsiPeripheral_VerifyRequest req;
    ScsiPeripheral_Response resp;
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_Verify10 failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralVerifyTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_VerifyRequest req;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_Verify10(nullptr, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Verify10 failed");

    ScsiPeripheral_Device *dev = NewScsiPeripheralDevice();
    ret = OH_ScsiPeripheral_Verify10(dev, nullptr, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Verify10 failed");

    ret = OH_ScsiPeripheral_Verify10(dev, &req, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Verify10 failed");

    OH_ScsiPeripheral_Release();
    DeleteScsiPeripheralDevice(&dev);

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralVerifyThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_DeviceMemMap *devMmap = nullptr;
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(device, DEVICE_MEM_MAP_SIZE, &devMmap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_CreateDeviceMemMap failed");
    ScsiPeripheral_VerifyRequest req;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp;
    ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Verify10 failed");

    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralSendRequestByCDBOne(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_1;
    napi_value args[PARAM_1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    ScsiPeripheral_Device *dev = NewScsiPeripheralDevice();
    ScsiPeripheral_Request request = {{0}};
    ScsiPeripheral_Response response = {{0}};
    int32_t ret = OH_ScsiPeripheral_SendRequestByCDB(dev, &request, &response);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_SendRequestByCDB failed");
    DeleteScsiPeripheralDevice(&dev);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralSendRequestByCDBTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_Request request = {{0}};
    ScsiPeripheral_Response response = {{0}};
    ret = OH_ScsiPeripheral_SendRequestByCDB(nullptr, &request, &response);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_SendRequestByCDB failed");

    ScsiPeripheral_Device *dev = NewScsiPeripheralDevice();
    ret = OH_ScsiPeripheral_SendRequestByCDB(dev, nullptr, &response);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_SendRequestByCDB failed");

    ret = OH_ScsiPeripheral_SendRequestByCDB(dev, &request, &response);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_SendRequestByCDB failed");

    uint8_t buff;
    ScsiPeripheral_DeviceMemMap devMmap({&buff, sizeof(buff), 0, sizeof(buff), 0});
    request.data = &devMmap;
    ret = OH_ScsiPeripheral_SendRequestByCDB(dev, &request, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_SendRequestByCDB failed");

    OH_ScsiPeripheral_Release();
    DeleteScsiPeripheralDevice(&dev);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralSendRequestByCDBThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *dev = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &dev);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_DeviceMemMap *devMemMap = nullptr;
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(dev, DEVICE_MEM_MAP_SIZE, &devMemMap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_CreateDeviceMemMap failed");

    ScsiPeripheral_Request request;
    request.cdbLength = CDB_LENGTH_TEN;
    const uint8_t tmpCommand = 28;
    const int32_t tmpDataDirection = -3;
    request.commandDescriptorBlock[0] = tmpCommand;
    request.commandDescriptorBlock[ONE_BYTE] = 0;
    request.commandDescriptorBlock[TWO_BYTE] = 0;
    request.commandDescriptorBlock[THREE_BYTE] = 0;
    request.commandDescriptorBlock[FOUR_BYTE] = 0;
    request.commandDescriptorBlock[FIVE_BYTE] = 0;
    request.commandDescriptorBlock[SIX_BYTE] = 0;
    request.commandDescriptorBlock[SEVEN_BYTE] = 0;
    request.commandDescriptorBlock[EIGHT_BYTE] = 1;
    request.commandDescriptorBlock[NINE_BYTE] = 0;
    request.dataTransferDirection = tmpDataDirection;
    request.timeout = TIMEOUT;
    request.data = devMemMap;
    ScsiPeripheral_Response response;
    ret = OH_ScsiPeripheral_SendRequestByCDB(dev, &request, &response);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_SendRequestByCDB failed");

    OH_ScsiPeripheral_Close(&dev);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralCreateDeviceMemMapOne(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    ScsiPeripheral_DeviceMemMap *devMemMap = nullptr;
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(nullptr, DEVICE_MEM_MAP_SIZE, &devMemMap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER,
        "OH_ScsiPeripheral_CreateDeviceMemMap failed");

    ScsiPeripheral_Device *dev = NewScsiPeripheralDevice();
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(dev, DEVICE_MEM_MAP_SIZE, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER,
        "OH_ScsiPeripheral_CreateDeviceMemMap failed");

    ret = OH_ScsiPeripheral_CreateDeviceMemMap(dev, DEVICE_MEM_MAP_SIZE, &devMemMap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER,
        "OH_ScsiPeripheral_CreateDeviceMemMap failed");

    OH_ScsiPeripheral_Release();
    DeleteScsiPeripheralDevice(&dev);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralCreateDeviceMemMapTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *dev;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &dev);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    struct ScsiPeripheral_DeviceMemMap *devMemMap = nullptr;
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(dev, MAX_MEM_LEN, &devMemMap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_CreateDeviceMemMap failed");

    ret = OH_ScsiPeripheral_DestroyDeviceMemMap(devMemMap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_DestroyDeviceMemMap failed");
    OH_ScsiPeripheral_Close(&dev);
    ret = OH_ScsiPeripheral_Release();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Release failed");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralDestroyDeviceMemMapOne(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_DestroyDeviceMemMap(nullptr);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralDestroyDeviceMemMapTwo(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *dev = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &dev);

    ScsiPeripheral_DeviceMemMap *devMemMap = nullptr;
    OH_ScsiPeripheral_CreateDeviceMemMap(dev, DEVICE_MEM_MAP_SIZE, &devMemMap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_CreateDeviceMemMap failed");

    ret = OH_ScsiPeripheral_DestroyDeviceMemMap(devMemMap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_DestroyDeviceMemMap failed");

    OH_ScsiPeripheral_Close(&dev);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralParseBasicSenseInfoOne(napi_env env, napi_callback_info info)
{
    uint8_t senseData[SCSIPERIPHERAL_MAX_SENSE_DATA_LEN] = {0x00};
    ScsiPeripheral_BasicSenseInfo senseInfo = {0};
    int32_t ret = OH_ScsiPeripheral_ParseBasicSenseInfo(nullptr, sizeof(senseData), &senseInfo);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, 0, &senseInfo);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static napi_value ScsiPeripheralParseBasicSenseInfoTwo(napi_env env, napi_callback_info info)
{
    uint8_t senseData[SCSIPERIPHERAL_MAX_SENSE_DATA_LEN] = {0x00};
    senseData[0] = 0x70 | 0x80;
    ScsiPeripheral_BasicSenseInfo senseInfo = {0};
    int32_t ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);

    senseData[0] = 0x71 | 0x80;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);

    senseData[0] = 0x72;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x00;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);

    senseData[0] = 0x73;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x00;
    senseData[NINE_BYTE] = 0x0A;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);

    senseData[0] = 0x73;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x01;
    senseData[NINE_BYTE] = 0x0A;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);

    senseData[0] = 0x73;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x02;
    senseData[NINE_BYTE] = 0x06;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);

    senseData[0] = 0x73;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x02;
    senseData[NINE_BYTE] = THIRTYTWO_BIT;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

EXTERN_C_START
static napi_value InitOne(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("isScsiDevice", IsScsiDevice),
        DECLARE_NAPI_FUNCTION("scsiPeripheralInitOne", ScsiPeripheralInitOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralReleaseOne", ScsiPeripheralReleaseOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralReleaseTwo", ScsiPeripheralReleaseTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralOpenOne", ScsiPeripheralOpenOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralOpenTwo", ScsiPeripheralOpenTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralOpenThree", ScsiPeripheralOpenThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralOpenFour", ScsiPeripheralOpenFour),
        DECLARE_NAPI_FUNCTION("scsiPeripheralCloseOne", ScsiPeripheralCloseOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralCloseTwo", ScsiPeripheralCloseTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralCloseThree", ScsiPeripheralCloseThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralReadCapacityOne", ScsiPeripheralReadCapacityOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralReadCapacityTwo", ScsiPeripheralReadCapacityTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralReadCapacityThree", ScsiPeripheralReadCapacityThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralTestUnitReadyOne", ScsiPeripheralTestUnitReadyOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralTestUnitReadyTwo", ScsiPeripheralTestUnitReadyTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralTestUnitReadyThree", ScsiPeripheralTestUnitReadyThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralInquiryOne", ScsiPeripheralInquiryOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralInquiryTwo", ScsiPeripheralInquiryTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralInquiryThree", ScsiPeripheralInquiryThree),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

static napi_value InitTwo(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("scsiPeripheralRequestSenseOne", ScsiPeripheralRequestSenseOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralRequestSenseTwo", ScsiPeripheralRequestSenseTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralRequestSenseThree", ScsiPeripheralRequestSenseThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralReadOne", ScsiPeripheralReadOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralReadTwo", ScsiPeripheralReadTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralReadThree", ScsiPeripheralReadThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralWriteOne", ScsiPeripheralWriteOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralWriteTwo", ScsiPeripheralWriteTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralWriteThree", ScsiPeripheralWriteThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralVerifyOne", ScsiPeripheralVerifyOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralVerifyTwo", ScsiPeripheralVerifyTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralVerifyThree", ScsiPeripheralVerifyThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralSendRequestByCDBOne", ScsiPeripheralSendRequestByCDBOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralSendRequestByCDBTwo", ScsiPeripheralSendRequestByCDBTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralSendRequestByCDBThree", ScsiPeripheralSendRequestByCDBThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralCreateDeviceMemMapOne", ScsiPeripheralCreateDeviceMemMapOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralCreateDeviceMemMapTwo", ScsiPeripheralCreateDeviceMemMapTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralDestroyDeviceMemMapOne", ScsiPeripheralDestroyDeviceMemMapOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralDestroyDeviceMemMapTwo", ScsiPeripheralDestroyDeviceMemMapTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralParseBasicSenseInfoOne", ScsiPeripheralParseBasicSenseInfoOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralParseBasicSenseInfoTwo", ScsiPeripheralParseBasicSenseInfoTwo),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

static napi_value Init(napi_env env, napi_value exports)
{
    InitOne(env, exports);
    InitTwo(env, exports);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "libscsi_ddk_js_test",
    .nm_priv = ((void *)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&demoModule);
}