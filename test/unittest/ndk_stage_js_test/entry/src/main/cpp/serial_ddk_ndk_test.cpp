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
#include "usb_serial_api.h"
#include <cstdlib>
#include <js_native_api_types.h>
#include <unistd.h>
#include <vector>

#define PARM_0 0
#define PARM_1 1
#define USB_SERIAL_TEST_BUF_SIZE 100

UsbSerial_Device *NewSerialDeviceHandle();
void DeleteUsbSerialDeviceHandle(UsbSerial_Device **dev);

static uint64_t GetDeviceId(napi_env env, napi_callback_info info)
{
    size_t argc = PARM_1;
    napi_value args[PARM_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    int64_t tmpDeviceId;
    napi_get_value_int64(env, args[0], &tmpDeviceId);
    uint64_t deviceId = static_cast<uint64_t>(tmpDeviceId);
    return deviceId;
}

static int32_t OpenUsbSerial(uint64_t deviceId, UsbSerial_Device **deviceHandle)
{
    int32_t returnValue = OH_UsbSerial_Open(deviceId, 0, deviceHandle);
    if (returnValue != USB_SERIAL_DDK_SUCCESS) {
        OH_UsbSerial_Close(deviceHandle);
        OH_UsbSerial_Release();
    }
    return returnValue;
}

static napi_value IsUsbSerialDevice(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    UsbSerial_Device *deviceHandle = nullptr;

    returnValue = OH_UsbSerial_Open(deviceId, 0, &deviceHandle);
    bool boolRet = (returnValue == USB_SERIAL_DDK_SUCCESS ? true : false);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_get_boolean(env, boolRet, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialInit(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSerialReleaseOne(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Release();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_Release failed");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSerialReleaseTwo(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");
    returnValue = OH_UsbSerial_Release();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Release failed");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSerialOpenOne(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    UsbSerial_Device *deviceHandle = nullptr;
    int32_t returnValue = OH_UsbSerial_Open(deviceId, 0, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_Open failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialOpenTwo(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    returnValue = OH_UsbSerial_Open(deviceId, 0, nullptr);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Open failed");

    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialOpenThree(napi_env env, napi_callback_info info)
{
    napi_status status;
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    uint64_t deviceId = 10001001;
    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_DEVICE_NOT_FOUND, "OH_UsbSerial_Open failed");
    napi_value result = nullptr;
    status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialOpenFour(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OH_UsbSerial_Open(deviceId, 0, &deviceHandle);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialOpenFive(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    uint8_t interfaceIndex = 5;
    returnValue = OH_UsbSerial_Open(deviceId, interfaceIndex, &deviceHandle);
    OH_UsbSerial_Release();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_DEVICE_NOT_FOUND, "OH_UsbSerial_Open failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialWriteOne(napi_env env, napi_callback_info info)
{
    napi_status status;
    UsbSerial_Device *deviceHandle = nullptr;
    std::vector<uint8_t> buff = {1, 2, 3, 4, 5, 6};
    uint32_t bytesWritten = 0;

    int32_t returnValue = OH_UsbSerial_Write(deviceHandle, buff.data(), buff.size(), &bytesWritten);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_Write failed");

    napi_value result = nullptr;
    status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialWriteTwo(napi_env env, napi_callback_info info)
{
    napi_status status;
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    std::vector<uint8_t> buff = {1, 2, 3, 4, 5, 6};
    uint32_t bytesWritten = 0;
    returnValue = OH_UsbSerial_Write(deviceHandle, buff.data(), buff.size(), &bytesWritten);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Write failed");

    uint64_t deviceId = GetDeviceId(env, info);
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    returnValue = OH_UsbSerial_Write(deviceHandle, nullptr, buff.size(), &bytesWritten);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Write failed buffer null");

    returnValue = OH_UsbSerial_Write(deviceHandle, buff.data(), 0, &bytesWritten);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Write failed size 0");

    returnValue = OH_UsbSerial_Write(deviceHandle, buff.data(), buff.size(), nullptr);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Write failed bytesWritten null");

    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialWriteThree(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    std::vector<uint8_t> buff = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0xA};
    uint32_t bytesWritten = 0;
    returnValue = OH_UsbSerial_Write(deviceHandle, buff.data(), buff.size(), &bytesWritten);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialWriteFour(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    std::vector<uint8_t> buff = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0xA};
    uint32_t bytesWritten = 0;
    returnValue = OH_UsbSerial_Write(deviceHandle, buff.data(), buff.size(), &bytesWritten);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialCloseOne(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = nullptr;
    int32_t returnValue = OH_UsbSerial_Close(&deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_Close failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialCloseTwo(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    returnValue = OH_UsbSerial_Close(nullptr);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Close failed");

    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialCloseThree(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    returnValue = OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialCloseFour(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    returnValue = OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialReadOne(napi_env env, napi_callback_info info)
{
    napi_status status;
    UsbSerial_Device *deviceHandle = nullptr;
    std::vector<uint8_t> buff(USB_SERIAL_TEST_BUF_SIZE);
    uint32_t bytesRead = 0;

    int32_t returnValue = OH_UsbSerial_Read(deviceHandle, buff.data(), buff.size(), &bytesRead);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_Read failed");

    napi_value result = nullptr;
    status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialReadTwo(napi_env env, napi_callback_info info)
{
    napi_status status;
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    std::vector<uint8_t> buff(USB_SERIAL_TEST_BUF_SIZE);
    uint32_t bytesRead = 0;
    returnValue = OH_UsbSerial_Read(deviceHandle, buff.data(), buff.size(), &bytesRead);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Write failed");

    uint64_t deviceId = GetDeviceId(env, info);
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    returnValue = OH_UsbSerial_Read(deviceHandle, nullptr, buff.size(), &bytesRead);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Write failed");

    returnValue = OH_UsbSerial_Read(deviceHandle, buff.data(), 0, &bytesRead);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Write failed");

    returnValue = OH_UsbSerial_Read(deviceHandle, buff.data(), buff.size(), nullptr);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Write failed");

    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialReadThree(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);

    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    UsbSerial_Params params = {9600, 8, 1, 0};
    returnValue = OH_UsbSerial_SetParams(deviceHandle, &params);

    std::vector<uint8_t> writeBuff = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0xA};
    uint32_t bytesWritten = 0;
    returnValue = OH_UsbSerial_Write(deviceHandle, writeBuff.data(), writeBuff.size(), &bytesWritten);

    std::vector<uint8_t> readBuff(USB_SERIAL_TEST_BUF_SIZE);
    uint32_t bytesRead = 0;
    returnValue = OH_UsbSerial_Read(deviceHandle, readBuff.data(), readBuff.size(), &bytesRead);

    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialReadFour(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    std::vector<uint8_t> readBuff(USB_SERIAL_TEST_BUF_SIZE);
    uint32_t bytesRead = 0;
    returnValue = OH_UsbSerial_Read(deviceHandle, readBuff.data(), readBuff.size(), &bytesRead);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetBaudRateOne(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = nullptr;
    uint32_t baudRate = 9600;
    int32_t returnValue = OH_UsbSerial_SetBaudRate(deviceHandle, baudRate);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_SetBaudRate failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetBaudRateTwo(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    uint32_t baudRate = 9600;
    returnValue = OH_UsbSerial_SetBaudRate(deviceHandle, baudRate);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_SetBaudRate failed");

    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetBaudRateThree(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    uint32_t baudRate = 9600;
    returnValue = OH_UsbSerial_SetBaudRate(deviceHandle, baudRate);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetBaudRateFour(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    uint32_t baudRate = 9600;
    returnValue = OH_UsbSerial_SetBaudRate(deviceHandle, baudRate);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetParamsOne(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = nullptr;
    UsbSerial_Params params = {9600, 0, 10, 0};
    int32_t returnValue = OH_UsbSerial_SetParams(deviceHandle, &params);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_SetParams failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetParamsTwo(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    UsbSerial_Params params = {9600, 0, 10, 0};
    returnValue = OH_UsbSerial_SetParams(deviceHandle, &params);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Init failed");

    uint64_t deviceId = GetDeviceId(env, info);
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    returnValue = OH_UsbSerial_SetParams(deviceHandle, nullptr);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Init failed");
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetParamsThree(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Params params = {9600, 8, 1, 0};
    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    returnValue = OH_UsbSerial_SetParams(deviceHandle, &params);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetParamsFour(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    UsbSerial_Params params = {9600, 8, 1, 0};
    returnValue = OH_UsbSerial_SetParams(deviceHandle, &params);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetTimeoutOne(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = nullptr;
    uint32_t timeout = 1000;
    int32_t returnValue = OH_UsbSerial_SetTimeout(deviceHandle, timeout);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_SetTimeout failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetTimeoutTwo(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    uint32_t timeout = 1000;
    returnValue = OH_UsbSerial_SetTimeout(deviceHandle, timeout);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_SetTimeout failed");

    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetTimeoutThree(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    int32_t timeout = 1000;
    returnValue = OH_UsbSerial_SetTimeout(deviceHandle, timeout);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetTimeoutFour(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    uint32_t timeout = 1000;
    returnValue = OH_UsbSerial_SetTimeout(deviceHandle, timeout);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetTimeoutFive(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    uint32_t timeout = -10;
    returnValue = OH_UsbSerial_SetTimeout(deviceHandle, timeout);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_SetTimeout failed");
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetFlowControlOne(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = nullptr;
    int32_t returnValue = 0;
    returnValue = OH_UsbSerial_SetFlowControl(deviceHandle, UsbSerial_FlowControl::USB_SERIAL_SOFTWARE_FLOW_CONTROL);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_SetFlowControl failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetFlowControlTwo(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OH_UsbSerial_SetFlowControl(deviceHandle, UsbSerial_FlowControl::USB_SERIAL_SOFTWARE_FLOW_CONTROL);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_SetFlowControl failed");

    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetFlowControlThree(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    returnValue = OH_UsbSerial_SetFlowControl(deviceHandle, UsbSerial_FlowControl::USB_SERIAL_SOFTWARE_FLOW_CONTROL);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialSetFlowControlFour(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    returnValue = OH_UsbSerial_SetFlowControl(deviceHandle, UsbSerial_FlowControl::USB_SERIAL_SOFTWARE_FLOW_CONTROL);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushOne(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = nullptr;
    int32_t returnValue = OH_UsbSerial_Flush(deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_Flush failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushTwo(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OH_UsbSerial_Flush(deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_Flush failed");

    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushThree(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    returnValue = OH_UsbSerial_Flush(deviceHandle);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushFour(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    returnValue = OH_UsbSerial_Flush(deviceHandle);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushInputOne(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = nullptr;
    int32_t returnValue = OH_UsbSerial_FlushInput(deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_FlushInput failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushInputTwo(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OH_UsbSerial_FlushInput(deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_FlushInput failed");

    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushInputThree(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    returnValue = OH_UsbSerial_FlushInput(deviceHandle);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushInputFour(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    returnValue = OH_UsbSerial_FlushInput(deviceHandle);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushOutputOne(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = nullptr;
    int32_t returnValue = OH_UsbSerial_FlushOutput(deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INIT_ERROR, "OH_UsbSerial_FlushOutput failed");

    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushOutputTwo(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OH_UsbSerial_FlushOutput(deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_INVALID_PARAMETER, "OH_UsbSerial_FlushOutput failed");

    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushOutputThree(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = NewSerialDeviceHandle();
    returnValue = OH_UsbSerial_FlushOutput(deviceHandle);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

static napi_value UsbSerialFlushOutputFour(napi_env env, napi_callback_info info)
{
    uint64_t deviceId = GetDeviceId(env, info);
    int32_t returnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Init failed");

    UsbSerial_Device *deviceHandle = nullptr;
    returnValue = OpenUsbSerial(deviceId, &deviceHandle);
    NAPI_ASSERT(env, returnValue == USB_SERIAL_DDK_SUCCESS, "OH_UsbSerial_Open failed");

    returnValue = OH_UsbSerial_FlushOutput(deviceHandle);
    OH_UsbSerial_Close(&deviceHandle);
    OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, returnValue, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 failed");
    NAPI_CALL(env, status);
    return result;
}

EXTERN_C_START
static napi_value InitOne(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("isUsbSerialDevice", IsUsbSerialDevice),
        DECLARE_NAPI_FUNCTION("usbSerialInit", UsbSerialInit),
        DECLARE_NAPI_FUNCTION("usbSerialReleaseOne", UsbSerialReleaseOne),
        DECLARE_NAPI_FUNCTION("usbSerialReleaseTwo", UsbSerialReleaseTwo),
        DECLARE_NAPI_FUNCTION("usbSerialOpenOne", UsbSerialOpenOne),
        DECLARE_NAPI_FUNCTION("usbSerialOpenTwo", UsbSerialOpenTwo),
        DECLARE_NAPI_FUNCTION("usbSerialOpenThree", UsbSerialOpenThree),
        DECLARE_NAPI_FUNCTION("usbSerialOpenFour", UsbSerialOpenFour),
        DECLARE_NAPI_FUNCTION("usbSerialOpenFive", UsbSerialOpenFive),
        DECLARE_NAPI_FUNCTION("usbSerialCloseOne", UsbSerialCloseOne),
        DECLARE_NAPI_FUNCTION("usbSerialCloseTwo", UsbSerialCloseTwo),
        DECLARE_NAPI_FUNCTION("usbSerialCloseThree", UsbSerialCloseThree),
        DECLARE_NAPI_FUNCTION("usbSerialCloseFour", UsbSerialCloseFour),
        DECLARE_NAPI_FUNCTION("usbSerialWriteOne", UsbSerialWriteOne),
        DECLARE_NAPI_FUNCTION("usbSerialWriteTwo", UsbSerialWriteTwo),
        DECLARE_NAPI_FUNCTION("usbSerialWriteThree", UsbSerialWriteThree),
        DECLARE_NAPI_FUNCTION("usbSerialWriteFour", UsbSerialWriteFour),
        DECLARE_NAPI_FUNCTION("usbSerialReadOne", UsbSerialReadOne),
        DECLARE_NAPI_FUNCTION("usbSerialReadTwo", UsbSerialReadTwo),
        DECLARE_NAPI_FUNCTION("usbSerialReadThree", UsbSerialReadThree),
        DECLARE_NAPI_FUNCTION("usbSerialReadFour", UsbSerialReadFour),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

static napi_value InitTwo(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("usbSerialSetBaudRateOne", UsbSerialSetBaudRateOne),
        DECLARE_NAPI_FUNCTION("usbSerialSetBaudRateTwo", UsbSerialSetBaudRateTwo),
        DECLARE_NAPI_FUNCTION("usbSerialSetBaudRateThree", UsbSerialSetBaudRateThree),
        DECLARE_NAPI_FUNCTION("usbSerialSetBaudRateFour", UsbSerialSetBaudRateFour),
        DECLARE_NAPI_FUNCTION("usbSerialSetParamsOne", UsbSerialSetParamsOne),
        DECLARE_NAPI_FUNCTION("usbSerialSetParamsTwo", UsbSerialSetParamsTwo),
        DECLARE_NAPI_FUNCTION("usbSerialSetParamsThree", UsbSerialSetParamsThree),
        DECLARE_NAPI_FUNCTION("usbSerialSetParamsFour", UsbSerialSetParamsFour),
        DECLARE_NAPI_FUNCTION("usbSerialSetTimeoutOne", UsbSerialSetTimeoutOne),
        DECLARE_NAPI_FUNCTION("usbSerialSetTimeoutTwo", UsbSerialSetTimeoutTwo),
        DECLARE_NAPI_FUNCTION("usbSerialSetTimeoutThree", UsbSerialSetTimeoutThree),
        DECLARE_NAPI_FUNCTION("usbSerialSetTimeoutFour", UsbSerialSetTimeoutFour),
        DECLARE_NAPI_FUNCTION("usbSerialSetTimeoutFive", UsbSerialSetTimeoutFive),
        DECLARE_NAPI_FUNCTION("usbSerialSetFlowControlOne", UsbSerialSetFlowControlOne),
        DECLARE_NAPI_FUNCTION("usbSerialSetFlowControlTwo", UsbSerialSetFlowControlTwo),
        DECLARE_NAPI_FUNCTION("usbSerialSetFlowControlThree", UsbSerialSetFlowControlThree),
        DECLARE_NAPI_FUNCTION("usbSerialSetFlowControlFour", UsbSerialSetFlowControlFour),
        DECLARE_NAPI_FUNCTION("usbSerialFlushOne", UsbSerialFlushOne),
        DECLARE_NAPI_FUNCTION("usbSerialFlushTwo", UsbSerialFlushTwo),
        DECLARE_NAPI_FUNCTION("usbSerialFlushThree", UsbSerialFlushThree),
        DECLARE_NAPI_FUNCTION("usbSerialFlushFour", UsbSerialFlushFour),
        DECLARE_NAPI_FUNCTION("usbSerialFlushInputOne", UsbSerialFlushInputOne),
        DECLARE_NAPI_FUNCTION("usbSerialFlushInputTwo", UsbSerialFlushInputTwo),
        DECLARE_NAPI_FUNCTION("usbSerialFlushInputThree", UsbSerialFlushInputThree),
        DECLARE_NAPI_FUNCTION("usbSerialFlushInputFour", UsbSerialFlushInputFour),
        DECLARE_NAPI_FUNCTION("usbSerialFlushOutputOne", UsbSerialFlushOutputOne),
        DECLARE_NAPI_FUNCTION("usbSerialFlushOutputTwo", UsbSerialFlushOutputTwo),
        DECLARE_NAPI_FUNCTION("usbSerialFlushOutputThree", UsbSerialFlushOutputThree),
        DECLARE_NAPI_FUNCTION("usbSerialFlushOutputFour", UsbSerialFlushOutputFour),
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
    .nm_modname = "libserial_ddk_js_test",
    .nm_priv = ((void *)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&demoModule);
}