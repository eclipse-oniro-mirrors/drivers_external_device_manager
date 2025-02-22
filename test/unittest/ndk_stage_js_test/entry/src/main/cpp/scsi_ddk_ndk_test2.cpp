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
#include <string>

const uint32_t PARAM_0 = 0;
const uint32_t PARAM_1 = 1;
constexpr size_t DEVICE_MEM_MAP_SIZE = 10 * 1024; // 10K
constexpr size_t DEVICE_MEM_MAP_SIZE_50K = 50 * 1024; // 50K
constexpr size_t DEVICE_MEM_MAP_SIZE_128M = 128 * 1024 * 1024; // 128M
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
constexpr uint8_t TEMP_BUFFER_SIZE = 20;
constexpr uint8_t BASE_10 = 10;
constexpr uint8_t STATUS_MSG_LEN = 100;

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

static void AppendIntToString(char *buffer, int32_t ret)
{
    char temp[TEMP_BUFFER_SIZE];
    int i = 0;

    int isNegative = (ret < 0);
    if (isNegative) {
        ret = -ret;
    }

    do {
        temp[i++] = '0' + (ret % BASE_10);
        ret /= BASE_10;
    } while (ret > 0);

    if (isNegative) {
        temp[i++] = '-';
    }

    for (int start = 0, end = i - 1; start < end; start++, end--) {
        char t = temp[start];
        temp[start] = temp[end];
        temp[end] = t;
    }

    size_t bufferLen = strlen(buffer);
    for (int j = 0; j < i; j++) {
        buffer[bufferLen + j] = temp[j];
    }
    buffer[bufferLen + i] = '\0';
}

static napi_value ScsiPeripheralWriteOne(napi_env env, napi_callback_info info)
{
    ScsiPeripheral_Device *device = nullptr;
    ScsiPeripheral_IORequest req = {0};
    ScsiPeripheral_Response resp = {{0}};
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

    ScsiPeripheral_IORequest req = {0};
    ScsiPeripheral_Response resp = {{0}};
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

    DeleteScsiPeripheralDevice(&dev);
    OH_ScsiPeripheral_Release();

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
    ScsiPeripheral_IORequest req = {0};
    const uint32_t tmpTimeout = TIMEOUT2;
    req.lbAddress = 1;
    req.transferLength = 1;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Write10 failed");

    OH_ScsiPeripheral_DestroyDeviceMemMap(devMmap);
    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value WriteBytes1(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 20000;
    req.lbAddress = 0;
    req.transferLength = 1;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytes1 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value WriteBytes2(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 20000;
    req.lbAddress = 1;
    req.transferLength = 1;
    req.control = 0;
    req.byte1 = UINT8_MAX;
    req.byte6 = 0;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytes2 failed ret: ";
    AppendIntToString(statusMsg, ret);

    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value WriteBytes3(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 20000;
    req.lbAddress = 0;
    req.transferLength = 1;
    req.control = 0;
    req.byte1 = 0x13;
    req.byte6 = 0;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytes3 failed ret: ";
    AppendIntToString(statusMsg, ret);

    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value WriteBytes6Check1(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 20000;
    req.lbAddress = 0;
    req.transferLength = 1;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytes6Check1 failed ret: ";
    AppendIntToString(statusMsg, ret);

    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value WriteBytes6Check2(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 20000;
    req.lbAddress = 0;
    req.transferLength = 1;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = UINT8_MAX;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytes6Check2 failed ret: ";
    AppendIntToString(statusMsg, ret);

    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value WriteBytes6Check3(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 10000;
    req.lbAddress = 0;
    req.transferLength = 1;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0x13;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytes6Check3 failed ret: ";
    AppendIntToString(statusMsg, ret);

    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value WriteBytes6Check4(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 10000;
    req.lbAddress = UINT32_MAX;
    req.transferLength = 0x10;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0x13;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytes6Check4 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }

    if ((resp.status != SCSIPERIPHERAL_STATUS_GOOD) && (resp.status != SCSIPERIPHERAL_STATUS_CHECK_CONDITION_NEEDED)) {
        char statusMsg2[STATUS_MSG_LEN] = "WriteBytes6Check4 Status check condition needed, actual status: ";
        AppendIntToString(statusMsg2, resp.status);
        napi_throw_error(env, nullptr, statusMsg2);
        return nullptr;
    }
    return nullptr;
}

static napi_value WriteBytesTrans1(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 10000;
    req.lbAddress = 0;
    req.transferLength = 0;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0x13;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytesTrans1 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static int32_t WriteBytesTrans2(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 10000;
    req.lbAddress = 0;
    req.transferLength = UINT16_MAX;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0x13;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytesTrans2 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if ((ret != SCSIPERIPHERAL_DDK_IO_ERROR) && (ret != SCSIPERIPHERAL_DDK_SUCCESS)) {
        napi_throw_error(env, nullptr, statusMsg);
        return ret;
    }
    return ret;
}

static napi_value WriteBytesTrans3(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 10000;
    req.lbAddress = 0;
    req.transferLength = 0x10;
    req.control = 0;
    req.byte1 = 0;
    req.byte6 = 0x13;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytesTrans3 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value WriteBytesTrans4(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 10000;
    req.lbAddress = 0;
    req.transferLength = 0x10;
    req.control = UINT8_MAX;
    req.byte1 = 0;
    req.byte6 = 0x13;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytesTrans4 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value WriteBytesTrans5(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMmap)
{
    ScsiPeripheral_IORequest req;
    const uint32_t tmpTimeout = 10000;
    const uint32_t control = 100;
    req.lbAddress = 0;
    req.transferLength = 0x10;
    req.control = control;
    req.byte1 = 0;
    req.byte6 = 0x13;
    req.timeout = tmpTimeout;
    req.data = devMmap;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Write10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "WriteBytesTrans5 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value ScsiPeripheralWriteFour(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_DeviceMemMap *devMmap = nullptr;
    ret = OH_ScsiPeripheral_CreateDeviceMemMap(device, DEVICE_MEM_MAP_SIZE_50K, &devMmap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_CreateDeviceMemMap failed");
    WriteBytes1(env, device, devMmap);
    WriteBytes2(env, device, devMmap);
    WriteBytes3(env, device, devMmap);
    WriteBytes6Check1(env, device, devMmap);
    WriteBytes6Check2(env, device, devMmap);
    WriteBytes6Check3(env, device, devMmap);
    WriteBytes6Check4(env, device, devMmap);
    WriteBytesTrans1(env, device, devMmap);
    WriteBytesTrans3(env, device, devMmap);
    WriteBytesTrans4(env, device, devMmap);
    WriteBytesTrans5(env, device, devMmap);
    OH_ScsiPeripheral_DestroyDeviceMemMap(devMmap);

    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static int32_t CheckCDB1(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMemMap)
{
    ScsiPeripheral_Request request;
    request.cdbLength = 0;
    const uint8_t tmpCommand = 28;
    const int32_t tmpDataDirection = 1;
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
    ScsiPeripheral_Response response = {{0}};
    int32_t ret = OH_ScsiPeripheral_SendRequestByCdb(device, &request, &response);
    char statusMsg[STATUS_MSG_LEN] = "CheckCDB1 failed ret: ";
    AppendIntToString(statusMsg, ret);

    if (ret != SCSIPERIPHERAL_DDK_INVALID_PARAMETER) {
        napi_throw_error(env, nullptr, statusMsg);
        return ret;
    }
    return ret;
}

static napi_value ScsiPeripheralWriteFour1(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_DeviceMemMap *devMmap = nullptr;
    OH_ScsiPeripheral_CreateDeviceMemMap(device, DEVICE_MEM_MAP_SIZE, &devMmap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_CreateDeviceMemMap failed");
    ret = CheckCDB1(env, device, devMmap);

    OH_ScsiPeripheral_DestroyDeviceMemMap(devMmap);
    OH_ScsiPeripheral_Close(&device);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value ScsiPeripheralWriteFive(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ScsiPeripheral_DeviceMemMap *devMmap = nullptr;
    OH_ScsiPeripheral_CreateDeviceMemMap(device, DEVICE_MEM_MAP_SIZE_128M, &devMmap);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_CreateDeviceMemMap failed");
    ret = WriteBytesTrans2(env, device, devMmap);

    OH_ScsiPeripheral_DestroyDeviceMemMap(devMmap);
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
    ScsiPeripheral_Response resp = {{0}};
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

    ScsiPeripheral_VerifyRequest req = {0};
    ScsiPeripheral_Response resp = {{0}};
    ret = OH_ScsiPeripheral_Verify10(nullptr, &req, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Verify10 failed");

    ScsiPeripheral_Device *dev = NewScsiPeripheralDevice();
    ret = OH_ScsiPeripheral_Verify10(dev, nullptr, &resp);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Verify10 failed");

    ret = OH_ScsiPeripheral_Verify10(dev, &req, nullptr);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_Verify10 failed");

    DeleteScsiPeripheralDevice(&dev);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, ret, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value Verify0(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);

    char statusMsg[STATUS_MSG_LEN] = "Verify0 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify1(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.lbAddress = 0;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);

    char statusMsg[STATUS_MSG_LEN] = "Verify1 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify2(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.lbAddress = UINT32_MAX;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify2 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    if ((resp.status != SCSIPERIPHERAL_STATUS_GOOD) && (resp.status != SCSIPERIPHERAL_STATUS_CHECK_CONDITION_NEEDED)) {
        char statusMsg2[STATUS_MSG_LEN] = "Verify2 Status check condition needed, actual status: ";
        AppendIntToString(statusMsg2, resp.status);
        napi_throw_error(env, nullptr, statusMsg2);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify3(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.verificationLength = 0;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify3 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify4(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.verificationLength = 0;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify4 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify5(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.verificationLength = 0x16;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify5 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify6(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.control = 0;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify6 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify7(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.control = UINT8_MAX;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify7 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify8(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.control = 0x64;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify8 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify9(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.byte1 = 0;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify9 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify10(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.byte1 = UINT8_MAX;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify10 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify11(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.byte1 = 0x88;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify11 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify12(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.byte6 = 0;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify12 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify13(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.byte6 = UINT8_MAX;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify13 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify14(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.byte6 = 0x88;
    req.timeout = TIMEOUT;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify14 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify15(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.timeout = UINT32_MAX;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify15 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value Verify16(napi_env env, ScsiPeripheral_Device *device)
{
    ScsiPeripheral_VerifyRequest req;
    req.timeout = 0x10;
    ScsiPeripheral_Response resp = {{0}};
    int32_t ret = OH_ScsiPeripheral_Verify10(device, &req, &resp);
    char statusMsg[STATUS_MSG_LEN] = "Verify16 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        napi_throw_error(env, nullptr, statusMsg);
        return nullptr;
    }
    return nullptr;
}

static napi_value ScsiPeripheralVerifyThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");
    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *device = nullptr;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &device);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    Verify0(env, device);
    Verify1(env, device);
    Verify2(env, device);
    Verify3(env, device);
    Verify4(env, device);
    Verify5(env, device);
    Verify6(env, device);
    Verify7(env, device);
    Verify8(env, device);
    Verify9(env, device);
    Verify10(env, device);
    Verify11(env, device);
    Verify12(env, device);
    Verify13(env, device);
    Verify14(env, device);
    Verify15(env, device);
    Verify16(env, device);
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
    int32_t ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INIT_ERROR, "OH_ScsiPeripheral_SendRequestByCdb failed");
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
    ret = OH_ScsiPeripheral_SendRequestByCdb(nullptr, &request, &response);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER, "OH_ScsiPeripheral_SendRequestByCdb failed");

    OH_ScsiPeripheral_Release();

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

    ScsiPeripheral_Request request = {{0}};
    request.cdbLength = CDB_LENGTH_TEN;
    const uint8_t tmpCommand = 28;
    const int32_t tmpDataDirection = -3;
    constexpr uint32_t tmpTimeout = 10;
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
    request.timeout = tmpTimeout;
    request.data = devMemMap;
    ScsiPeripheral_Response response = {{0}};
    ret = OH_ScsiPeripheral_SendRequestByCdb(dev, &request, &response);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_SendRequestByCdb failed");

    OH_ScsiPeripheral_DestroyDeviceMemMap(devMemMap);
    OH_ScsiPeripheral_Close(&dev);
    OH_ScsiPeripheral_Release();

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

static int32_t CheckCDB2(napi_env env, ScsiPeripheral_Device *device, ScsiPeripheral_DeviceMemMap *devMemMap)
{
    ScsiPeripheral_Request request = {{0}};
    request.cdbLength = UINT8_MAX;
    const uint8_t tmpCommand = 28;
    const int32_t tmpDataDirection = 2;
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
    ScsiPeripheral_Response response = {{0}};
    int32_t ret = OH_ScsiPeripheral_SendRequestByCdb(device, &request, &response);
    char statusMsg[STATUS_MSG_LEN] = "CheckCDB2 failed ret: ";
    AppendIntToString(statusMsg, ret);
    if (ret != SCSIPERIPHERAL_DDK_IO_ERROR) {
        napi_throw_error(env, nullptr, statusMsg);
        return ret;
    }
    return ret;
}

static napi_value ScsiPeripheralSendRequestByCDBFour(napi_env env, napi_callback_info info)
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

    ret = CheckCDB1(env, dev, devMemMap);
    ret = CheckCDB2(env, dev, devMemMap);

    OH_ScsiPeripheral_DestroyDeviceMemMap(devMemMap);
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
    OH_ScsiPeripheral_DestroyDeviceMemMap(devMemMap);
    DeleteScsiPeripheralDevice(&dev);
    OH_ScsiPeripheral_Release();

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

    ScsiPeripheral_DeviceMemMap *devMemMap = nullptr;
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

static int32_t CheckMemory1(napi_env env, ScsiPeripheral_Device *dev)
{
    ScsiPeripheral_DeviceMemMap *devMemMap = nullptr;
    size_t memSize = 0;
    int32_t ret = OH_ScsiPeripheral_CreateDeviceMemMap(dev, memSize, &devMemMap);

    char statusMsg[STATUS_MSG_LEN] = "CheckMemory1 failed ret: ";
    AppendIntToString(statusMsg, ret);
    OH_ScsiPeripheral_DestroyDeviceMemMap(devMemMap);
    if (ret != SCSIPERIPHERAL_DDK_MEMORY_ERROR) {
        napi_throw_error(env, nullptr, statusMsg);
        return ret;
    }

    return ret;
}

static napi_value ScsiPeripheralCreateDeviceMemMapThree(napi_env env, napi_callback_info info)
{
    int32_t ret = OH_ScsiPeripheral_Init();
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Init failed");

    uint64_t deviceId = GetDeviceId(env, info);
    ScsiPeripheral_Device *dev;
    ret = OH_ScsiPeripheral_Open(deviceId, 0, &dev);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS, "OH_ScsiPeripheral_Open failed");

    ret = CheckMemory1(env, dev);

    OH_ScsiPeripheral_Close(&dev);
    OH_ScsiPeripheral_Release();

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

    uint8_t tmpLen = 0;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, tmpLen, &senseInfo);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_INVALID_PARAMETER,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    tmpLen = UINT8_MAX;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, tmpLen, &senseInfo);
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
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    senseData[0] = 0x71 | 0x80;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    senseData[0] = 0x72;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x00;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    senseData[0] = 0x73;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x00;
    senseData[NINE_BYTE] = 0x0A;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    senseData[0] = 0x73;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x01;
    senseData[NINE_BYTE] = 0x0A;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    senseData[0] = 0x73;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x02;
    senseData[NINE_BYTE] = 0x06;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    senseData[0] = 0x73;
    senseData[SEVEN_BYTE] = THIRTYTWO_BIT;
    senseData[EIGHT_BYTE] = 0x02;
    senseData[NINE_BYTE] = THIRTYTWO_BIT;
    ret = OH_ScsiPeripheral_ParseBasicSenseInfo(senseData, sizeof(senseData), &senseInfo);
    NAPI_ASSERT(env, ret == SCSIPERIPHERAL_DDK_SUCCESS,
        "OH_ScsiPeripheral_ParseBasicSenseInfo failed");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ret, &result));
    return result;
}

EXTERN_C_START
static napi_value InitOne(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("scsiPeripheralWriteOne", ScsiPeripheralWriteOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralWriteTwo", ScsiPeripheralWriteTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralWriteThree", ScsiPeripheralWriteThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralWriteFour", ScsiPeripheralWriteFour),
        DECLARE_NAPI_FUNCTION("scsiPeripheralWriteFour1", ScsiPeripheralWriteFour1),
        DECLARE_NAPI_FUNCTION("scsiPeripheralWriteFive", ScsiPeripheralWriteFive),
        DECLARE_NAPI_FUNCTION("scsiPeripheralVerifyOne", ScsiPeripheralVerifyOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralVerifyTwo", ScsiPeripheralVerifyTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralVerifyThree", ScsiPeripheralVerifyThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralSendRequestByCDBOne", ScsiPeripheralSendRequestByCDBOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralSendRequestByCDBTwo", ScsiPeripheralSendRequestByCDBTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralSendRequestByCDBThree", ScsiPeripheralSendRequestByCDBThree),
        DECLARE_NAPI_FUNCTION("scsiPeripheralSendRequestByCDBFour", ScsiPeripheralSendRequestByCDBFour),
        DECLARE_NAPI_FUNCTION("scsiPeripheralCreateDeviceMemMapOne", ScsiPeripheralCreateDeviceMemMapOne),
        DECLARE_NAPI_FUNCTION("scsiPeripheralCreateDeviceMemMapTwo", ScsiPeripheralCreateDeviceMemMapTwo),
        DECLARE_NAPI_FUNCTION("scsiPeripheralCreateDeviceMemMapThree", ScsiPeripheralCreateDeviceMemMapThree),
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
