/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "usb_ddk_api.h"
#include "usb_ddk_types.h"
#include "hid_ddk_api.h"
#include "hid_ddk_types.h"
#include "usb_serial_api.h"
#include "ddk_api.h"
#include <cstdlib>
#include <js_native_api_types.h>
#include <vector>

constexpr size_t MAX_USB_DEVICE_NUM = 128;
constexpr uint32_t USB_SERIAL_TEST_BAUDRATE = 9600;
constexpr uint8_t USB_SERIAL_TEST_DATA_BITS = 8;
using namespace std;
const uint32_t PARAM_0 = 0;
const uint32_t PARAM_1 = 1;
const uint32_t DATA_BUFF_SIZE  = 1024;
const uint32_t READ_TIME_OUT = 10000;
const uint32_t SIXTEEN_BIT = 16;
const uint32_t THIRTYTWO_BIT = 32;
const uint32_t BUS_NUM_MASK = 0xFFFF0000;
const uint32_t DEVICE_NUM_MASK = 0x0000FFFF;

extern "C" Hid_DeviceHandle *NewHidDeviceHandle();
extern "C" void DeleteHidDeviceHandle(Hid_DeviceHandle **dev);

UsbSerial_Device *NewSerialDeviceHandle();
void DeleteUsbSerialDeviceHandle(UsbSerial_Device **dev);

static napi_value UsbInit(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_Usb_Init();
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbReleaseResource(napi_env env, napi_callback_info info)
{
    int32_t usbInitReturnValue = OH_Usb_Init();
    NAPI_ASSERT(env, usbInitReturnValue == USB_DDK_NO_PERM, "OH_Usb_Init failed, no permission");
    int32_t returnValue = OH_Usb_ReleaseResource();
    napi_value result = nullptr;
    napi_create_int32(env, returnValue, &result);
    return result;
}

static napi_value UsbGetDeviceDescriptor(napi_env env, napi_callback_info info)
{
    struct UsbDeviceDescriptor devDesc;
    int32_t returnValue = OH_Usb_GetDeviceDescriptor(0, &devDesc);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbGetConfigDescriptor(napi_env env, napi_callback_info info)
{
    struct UsbDdkConfigDescriptor *config = nullptr;
    int32_t returnValue = OH_Usb_GetConfigDescriptor(0, 1, &config);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbClaimInterface(napi_env env, napi_callback_info info)
{
    uint64_t g_interfaceHandle = 0;
    int32_t returnValue = OH_Usb_ClaimInterface(0, 0, &g_interfaceHandle);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbReleaseInterface(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_Usb_ReleaseInterface(0);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSelectInterfaceSetting(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_Usb_SelectInterfaceSetting(0, 0);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbGetCurrentInterfaceSetting(napi_env env, napi_callback_info info)
{
    uint8_t settingIndex = 0;
    int32_t returnValue = OH_Usb_GetCurrentInterfaceSetting(0, &settingIndex);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSendControlReadRequest(napi_env env, napi_callback_info info)
{
    struct UsbControlRequestSetup setup;
    uint8_t strDesc[2] = {0};
    uint32_t len = 100;
    int32_t returnValue = OH_Usb_SendControlReadRequest(0, &setup, 0, strDesc, &len);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSendControlWriteRequest(napi_env env, napi_callback_info info)
{
    struct UsbControlRequestSetup strDescSetup;
    uint8_t data[2] = {0x02, 0x02};
    int32_t returnValue = OH_Usb_SendControlWriteRequest(0, &strDescSetup, 0, data, 2);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSendPipeRequest(napi_env env, napi_callback_info info)
{
    struct UsbRequestPipe pipe;
    uint8_t address = 0;
    struct UsbDeviceMemMap devMmap = {.address = &address};
    int32_t returnValue = OH_Usb_SendPipeRequest(&pipe, &devMmap);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbCreateDeviceMemMap(napi_env env, napi_callback_info info)
{
    UsbDeviceMemMap *devMmap = nullptr;
    int32_t returnValue = OH_Usb_CreateDeviceMemMap(0, 100, &devMmap);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSendPipeRequestWithAshmem(napi_env env, napi_callback_info info)
{
    DDK_Ashmem *ashmem = nullptr;
    struct UsbRequestPipe pipe;
    int32_t returnValue = OH_Usb_SendPipeRequestWithAshmem(&pipe, ashmem);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbGetDevices(napi_env env, napi_callback_info info)
{
    int32_t usbInitReturnValue = OH_Usb_Init();
    NAPI_ASSERT(env, usbInitReturnValue == USB_DDK_NO_PERM, "OH_Usb_Init failed, no permission");
    struct Usb_DeviceArray deviceArray;
    deviceArray.deviceIds = new uint64_t[MAX_USB_DEVICE_NUM];
    int32_t returnValue = OH_Usb_GetDevices(&deviceArray);
    OH_Usb_Release();
    delete[] deviceArray.deviceIds;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static int32_t CreateTestDevice()
{
    std::vector<Hid_DeviceProp> deviceProp = { HID_PROP_DIRECT };
    Hid_Device hidDevice = {
        .deviceName = "VSoC keyboard",
        .vendorId = 0x6006,
        .productId = 0x6006,
        .version = 1,
        .bustype = 3,
        .properties = deviceProp.data(),
        .propLength = (uint16_t)deviceProp.size()
    };

    std::vector<Hid_EventType> eventType = { HID_EV_ABS, HID_EV_KEY, HID_EV_SYN, HID_EV_MSC };
    Hid_EventTypeArray eventTypeArray = {
        .hidEventType = eventType.data(),
        .length = (uint16_t)eventType.size()
    };
    std::vector<Hid_KeyCode> keyCode = { HID_BTN_TOOL_PEN, HID_BTN_TOOL_RUBBER, HID_BTN_TOUCH, HID_BTN_STYLUS,
        HID_BTN_RIGHT };
    Hid_KeyCodeArray keyCodeArray = {
        .hidKeyCode = keyCode.data(),
        .length = (uint16_t)keyCode.size()
    };
    std::vector<Hid_MscEvent> mscEvent = { HID_MSC_SCAN };
    Hid_MscEventArray mscEventArray = {
        .hidMscEvent = mscEvent.data(),
        .length = (uint16_t)mscEvent.size()
    };
    std::vector<Hid_AbsAxes> absAxes = { HID_ABS_X, HID_ABS_Y, HID_ABS_PRESSURE };
    Hid_AbsAxesArray absAxesArray = {
        .hidAbsAxes = absAxes.data(),
        .length = (uint16_t)absAxes.size()
    };
    Hid_EventProperties hidEventProp = {
        .hidEventTypes = eventTypeArray,
        .hidKeys = keyCodeArray,
        .hidAbs = absAxesArray,
        .hidMiscellaneous = mscEventArray
    };
    return OH_Hid_CreateDevice(&hidDevice, &hidEventProp);
}

static napi_value HidCreateDevice(napi_env env, napi_callback_info info)
{
    int32_t deviceId = CreateTestDevice();
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, deviceId, &result));
    return result;
}

static napi_value HidEmitEvent(napi_env env, napi_callback_info info)
{
    Hid_EmitItem event = {
        .type = HID_EV_MSC,
        .code = HID_MSC_SCAN,
        .value = 0x000d0042
    };
    std::vector<Hid_EmitItem> items;
    items.push_back(event);
    int32_t returnValue = OH_Hid_EmitEvent(0, items.data(), (uint16_t)items.size());
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value HidDestroyDevice(napi_env env, napi_callback_info info)
{
    int32_t deviceId = 0;
    int32_t returnValue = OH_Hid_DestroyDevice(deviceId);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value HidInit(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_Hid_Init();
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value HidRelease(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");
    int32_t returnValue = OH_Hid_Release();
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value HidOpen(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_1;
    napi_value args[PARAM_1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    int64_t deviceId64;
    NAPI_CALL(env, napi_get_value_int64(env, args[PARAM_0], &deviceId64));
    int32_t deviceId32 = (int32_t)(deviceId64 >> THIRTYTWO_BIT);
    uint32_t busNum = ((deviceId32 & BUS_NUM_MASK) >> SIXTEEN_BIT);
    uint32_t deviceNum = deviceId32 & DEVICE_NUM_MASK;
    uint64_t deviceId = ((uint64_t)busNum << THIRTYTWO_BIT) | deviceNum;

    struct Hid_DeviceHandle *dev = nullptr;
    int32_t returnValue = OH_Hid_Open(deviceId, 0, &dev);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    OH_Hid_Close(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidClose(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    int32_t returnValue = OH_Hid_Close(&dev);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidWrite(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    uint8_t data[] = {0x02, 0x02};
    uint32_t bytesWritten = 0;
    int32_t returnValue = OH_Hid_Write(dev, data, sizeof(data), &bytesWritten);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidReadTimeout(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    uint8_t data[DATA_BUFF_SIZE] = {0};
    uint32_t bytesRead = 0;
    int32_t returnValue = OH_Hid_ReadTimeout(dev, data, DATA_BUFF_SIZE, READ_TIME_OUT, &bytesRead);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidRead(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    uint8_t data[DATA_BUFF_SIZE] = {0};
    uint32_t bytesRead = 0;
    int32_t returnValue = OH_Hid_Read(dev, data, DATA_BUFF_SIZE, &bytesRead);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidSetNonBlocking(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    int32_t returnValue = OH_Hid_SetNonBlocking(dev, 0);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidGetRawInfo(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    Hid_RawDevInfo rawDevInfo = {0};
    int32_t returnValue = OH_Hid_GetRawInfo(dev, &rawDevInfo);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidGetRawName(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    char data[DATA_BUFF_SIZE] = {0};
    int32_t returnValue = OH_Hid_GetRawName(dev, data, DATA_BUFF_SIZE);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidGetPhysicalAddress(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    char data[DATA_BUFF_SIZE] = {0};
    int32_t returnValue = OH_Hid_GetPhysicalAddress(dev, data, DATA_BUFF_SIZE);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidGetRawUniqueId(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    uint8_t data[DATA_BUFF_SIZE] = {0};
    int32_t returnValue = OH_Hid_GetRawUniqueId(dev, data, DATA_BUFF_SIZE);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidSendReport(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    uint8_t data[DATA_BUFF_SIZE] = {0};
    int32_t returnValue = OH_Hid_SendReport(dev, HID_FEATURE_REPORT, data, DATA_BUFF_SIZE);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidGetReport(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    uint8_t data[DATA_BUFF_SIZE] = {0};
    int32_t returnValue = OH_Hid_GetReport(dev, HID_FEATURE_REPORT, data, DATA_BUFF_SIZE);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

static napi_value HidGetReportDescriptor(napi_env env, napi_callback_info info)
{
    int32_t initRetVal = OH_Hid_Init();
    NAPI_ASSERT(env, initRetVal == USB_DDK_NO_PERM, "OH_Hid_Init failed, no permission");

    Hid_DeviceHandle *dev = NewHidDeviceHandle();
    uint8_t data[DATA_BUFF_SIZE] = {0};
    uint32_t bytesRead = 0;
    int32_t returnValue = OH_Hid_GetReportDescriptor(dev, data, DATA_BUFF_SIZE, &bytesRead);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    DeleteHidDeviceHandle(&dev);
    int32_t releaseRetVal = OH_Hid_Release();
    NAPI_ASSERT(env, releaseRetVal == USB_DDK_NO_PERM, "OH_Hid_Release failed, no permission");
    return result;
}

UsbSerial_Device *InitializeAndCreateDeviceHandle(napi_env env)
{
    int32_t usbSerialInitReturnValue = OH_UsbSerial_Init();
    if (usbSerialInitReturnValue != USB_SERIAL_DDK_NO_PERM) {
        OH_UsbSerial_Release();
    }
    NAPI_ASSERT(env, usbSerialInitReturnValue == USB_SERIAL_DDK_NO_PERM, "OH_UsbSerial_Init, no permission");
    return NewSerialDeviceHandle();
}

static napi_value UsbSerialInit(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_UsbSerial_Init();
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSerialRelease(napi_env env, napi_callback_info info)
{
    int32_t usbSerialInitReturnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, usbSerialInitReturnValue == USB_SERIAL_DDK_NO_PERM, "OH_UsbSerial_Init, no permission");
    int32_t returnValue = OH_UsbSerial_Release();
    napi_value result = nullptr;
    napi_create_int32(env, returnValue, &result);
    return result;
}

static napi_value UsbSerialOpen(napi_env env, napi_callback_info info)
{
    int32_t usbSerialInitReturnValue = OH_UsbSerial_Init();
    NAPI_ASSERT(env, usbSerialInitReturnValue == USB_SERIAL_DDK_NO_PERM, "OH_UsbSerial_Init, no permission");
    UsbSerial_Device *deviceHandle = nullptr;
    int32_t returnValue = OH_UsbSerial_Open(0, 0, &deviceHandle);
    napi_value result = nullptr;
    napi_create_int32(env, returnValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

static napi_value UsbSerialClose(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    int32_t returnValue = OH_UsbSerial_Close(&deviceHandle);
    napi_value result = nullptr;
    napi_create_int32(env, returnValue, &result);
    return result;
}

static napi_value UsbSerialRead(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    uint8_t dataBuff[8];
    uint32_t bytesRead;
    int32_t retrunValue = OH_UsbSerial_Read(deviceHandle, dataBuff, sizeof(dataBuff), &bytesRead);
    napi_value result = nullptr;
    napi_create_int32(env, retrunValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

static napi_value UsbSerialWrite(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    uint8_t buff[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
    uint32_t bytesWritten;
    int32_t retrunValue = OH_UsbSerial_Write(deviceHandle, buff, sizeof(buff), &bytesWritten);
    napi_value result = nullptr;
    napi_create_int32(env, retrunValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

static napi_value UsbSerialSetBaudRate(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    uint32_t baudRate = 9600;
    int32_t retrunValue = OH_UsbSerial_SetBaudRate(deviceHandle, baudRate);
    napi_value result = nullptr;
    napi_create_int32(env, retrunValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

static napi_value UsbSerialSetParams(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    UsbSerial_Parity parity = USB_SERIAL_PARITY_ODD;
    UsbSerial_Params serialParams;
    serialParams.baudRate = USB_SERIAL_TEST_BAUDRATE;
    serialParams.nDataBits = USB_SERIAL_TEST_DATA_BITS;
    serialParams.nStopBits = 1;
    serialParams.parity = parity;
    int32_t retrunValue = OH_UsbSerial_SetParams(deviceHandle, &serialParams);
    napi_value result = nullptr;
    napi_create_int32(env, retrunValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

static napi_value UsbSerialSetTimeout(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    int timeout = 100;
    int32_t retrunValue = OH_UsbSerial_SetTimeout(deviceHandle, timeout);
    napi_value result = nullptr;
    napi_create_int32(env, retrunValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

static napi_value UsbSerialSetFlowControl(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    UsbSerial_FlowControl flowControl = USB_SERIAL_SOFTWARE_FLOW_CONTROL;
    int32_t retrunValue = OH_UsbSerial_SetFlowControl(deviceHandle, flowControl);
    napi_value result;
    napi_create_int32(env, retrunValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

static napi_value UsbSerialFlush(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    int32_t retrunValue = OH_UsbSerial_Flush(deviceHandle);
    napi_value result;
    napi_create_int32(env, retrunValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

static napi_value UsbSerialFlushInput(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    int32_t retrunValue = OH_UsbSerial_FlushInput(deviceHandle);
    napi_value result;
    napi_create_int32(env, retrunValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

static napi_value UsbSerialFlushOutput(napi_env env, napi_callback_info info)
{
    UsbSerial_Device *deviceHandle = InitializeAndCreateDeviceHandle(env);
    int32_t retrunValue = OH_UsbSerial_FlushOutput(deviceHandle);
    napi_value result;
    napi_create_int32(env, retrunValue, &result);
    DeleteUsbSerialDeviceHandle(&deviceHandle);
    return result;
}

EXTERN_C_START
static napi_value InitUsb(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("usbInit", UsbInit),
        DECLARE_NAPI_FUNCTION("usbReleaseResource", UsbReleaseResource),
        DECLARE_NAPI_FUNCTION("usbGetDeviceDescriptor", UsbGetDeviceDescriptor),
        DECLARE_NAPI_FUNCTION("usbGetConfigDescriptor", UsbGetConfigDescriptor),
        DECLARE_NAPI_FUNCTION("usbClaimInterface", UsbClaimInterface),
        DECLARE_NAPI_FUNCTION("usbReleaseInterface", UsbReleaseInterface),
        DECLARE_NAPI_FUNCTION("usbSelectInterfaceSetting", UsbSelectInterfaceSetting),
        DECLARE_NAPI_FUNCTION("usbGetCurrentInterfaceSetting", UsbGetCurrentInterfaceSetting),
        DECLARE_NAPI_FUNCTION("usbSendControlReadRequest", UsbSendControlReadRequest),
        DECLARE_NAPI_FUNCTION("usbSendControlWriteRequest", UsbSendControlWriteRequest),
        DECLARE_NAPI_FUNCTION("usbSendPipeRequest", UsbSendPipeRequest),
        DECLARE_NAPI_FUNCTION("usbCreateDeviceMemMap", UsbCreateDeviceMemMap),
        DECLARE_NAPI_FUNCTION("usbSendPipeRequestWithAshmem", UsbSendPipeRequestWithAshmem),
        DECLARE_NAPI_FUNCTION("usbGetDevices", UsbGetDevices),
        DECLARE_NAPI_FUNCTION("hidCreateDevice", HidCreateDevice),
        DECLARE_NAPI_FUNCTION("hidEmitEvent", HidEmitEvent),
        DECLARE_NAPI_FUNCTION("hidDestroyDevice", HidDestroyDevice),
        DECLARE_NAPI_FUNCTION("hidInit", HidInit),
        DECLARE_NAPI_FUNCTION("hidRelease", HidRelease),
        DECLARE_NAPI_FUNCTION("hidOpen", HidOpen),
        DECLARE_NAPI_FUNCTION("hidClose", HidClose),
        DECLARE_NAPI_FUNCTION("hidWrite", HidWrite),
        DECLARE_NAPI_FUNCTION("hidReadTimeout", HidReadTimeout),
        DECLARE_NAPI_FUNCTION("hidRead", HidRead),
        DECLARE_NAPI_FUNCTION("hidSetNonBlocking", HidSetNonBlocking),
        DECLARE_NAPI_FUNCTION("hidGetRawInfo", HidGetRawInfo),
        DECLARE_NAPI_FUNCTION("hidGetRawName", HidGetRawName),
        DECLARE_NAPI_FUNCTION("hidGetPhysicalAddress", HidGetPhysicalAddress),
        DECLARE_NAPI_FUNCTION("hidGetRawUniqueId", HidGetRawUniqueId),
        DECLARE_NAPI_FUNCTION("hidSendReport", HidSendReport),
        DECLARE_NAPI_FUNCTION("hidGetReport", HidGetReport),
        DECLARE_NAPI_FUNCTION("hidGetReportDescriptor", HidGetReportDescriptor),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

static napi_value InitUsbSerial(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("usbSerialInit", UsbSerialInit),
        DECLARE_NAPI_FUNCTION("usbSerialRelease", UsbSerialRelease),
        DECLARE_NAPI_FUNCTION("usbSerialOpen", UsbSerialOpen),
        DECLARE_NAPI_FUNCTION("usbSerialClose", UsbSerialClose),
        DECLARE_NAPI_FUNCTION("usbSerialRead", UsbSerialRead),
        DECLARE_NAPI_FUNCTION("usbSerialWrite", UsbSerialWrite),
        DECLARE_NAPI_FUNCTION("usbSerialSetBaudRate", UsbSerialSetBaudRate),
        DECLARE_NAPI_FUNCTION("usbSerialSetParams", UsbSerialSetParams),
        DECLARE_NAPI_FUNCTION("usbSerialSetTimeout", UsbSerialSetTimeout),
        DECLARE_NAPI_FUNCTION("usbSerialSetFlowControl", UsbSerialSetFlowControl),
        DECLARE_NAPI_FUNCTION("usbSerialFlush", UsbSerialFlush),
        DECLARE_NAPI_FUNCTION("usbSerialFlushInput", UsbSerialFlushInput),
        DECLARE_NAPI_FUNCTION("usbSerialFlushOutput", UsbSerialFlushOutput),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

static napi_value Init(napi_env env, napi_value exports)
{
    InitUsb(env, exports);
    InitUsbSerial(env, exports);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "libddk_permission_test",
    .nm_priv = ((void *)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&demoModule);
}
