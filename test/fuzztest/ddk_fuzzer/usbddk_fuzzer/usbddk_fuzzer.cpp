/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "usbddk_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "securec.h"
#include "usb_ddk_api.h"
#include "usb_config_desc_parser.h"

namespace OHOS {
namespace ExternalDeviceManager {
constexpr size_t OPERATOR_INDEX_SIZE = 1;
constexpr size_t DEVICE_ID_SIZE = sizeof(uint64_t);
constexpr size_t INTERFACE_INDEX_SIZE = sizeof(uint8_t);
constexpr size_t INTERFACE_HANDLE_SIZE = sizeof(uint64_t);
constexpr size_t TIMEOUT_SIZE = sizeof(uint32_t);
constexpr size_t ENDPOINT_SIZE = sizeof(uint8_t);
constexpr size_t MEM_OFFSET_SIZE = sizeof(uint32_t);
constexpr size_t BUFFER_LENGTH_SIZE = sizeof(uint32_t);
constexpr size_t MMAP_SIZE_FIELD_SIZE = sizeof(uint64_t);
constexpr size_t PIPE_FUZZ_BUFFER_SIZE = 1024;
constexpr uint64_t MAX_MEM_MAP_SIZE = 64 * 1024;
constexpr uint32_t MAX_PIPE_TIMEOUT_MS = 1000;

static bool UsbDdkInitFuzz(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    (void)OH_Usb_Init();
    return true;
}

static bool UsbDdkReleaseFuzz(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    (void)OH_Usb_Release();
    return true;
}

static bool UsbDdkClaimInterfaceFuzz(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < DEVICE_ID_SIZE + INTERFACE_INDEX_SIZE) {
        return false;
    }
    uint64_t deviceId = 0;
    if (memcpy_s(&deviceId, sizeof(deviceId), data, DEVICE_ID_SIZE) != EOK) {
        return false;
    }
    uint8_t interfaceIndex = data[DEVICE_ID_SIZE];
    uint64_t interfaceHandle = 0;
    (void)OH_Usb_ClaimInterface(deviceId, interfaceIndex, &interfaceHandle);
    (void)OH_Usb_ClaimInterface(deviceId, interfaceIndex, nullptr);
    return true;
}

static bool UsbDdkReleaseInterfaceFuzz(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < INTERFACE_HANDLE_SIZE) {
        return false;
    }
    uint64_t interfaceHandle = 0;
    if (memcpy_s(&interfaceHandle, sizeof(interfaceHandle), data, INTERFACE_HANDLE_SIZE) != EOK) {
        return false;
    }
    (void)OH_Usb_ReleaseInterface(interfaceHandle);
    return true;
}

static bool UsbDdkInterfaceLifecycleFuzz(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < DEVICE_ID_SIZE + INTERFACE_INDEX_SIZE) {
        return false;
    }
    (void)OH_Usb_Init();
    uint64_t deviceId = 0;
    if (memcpy_s(&deviceId, sizeof(deviceId), data, DEVICE_ID_SIZE) != EOK) {
        return false;
    }
    uint8_t interfaceIndex = data[DEVICE_ID_SIZE];
    uint64_t interfaceHandle = 0;
    if (OH_Usb_ClaimInterface(deviceId, interfaceIndex, &interfaceHandle) == USB_DDK_SUCCESS) {
        (void)OH_Usb_ReleaseInterface(interfaceHandle);
    }
    size_t offset = DEVICE_ID_SIZE + INTERFACE_INDEX_SIZE;
    if (size - offset >= INTERFACE_HANDLE_SIZE) {
        uint64_t fuzzedHandle = 0;
        if (memcpy_s(&fuzzedHandle, sizeof(fuzzedHandle), data + offset, INTERFACE_HANDLE_SIZE) == EOK) {
            (void)OH_Usb_ReleaseInterface(fuzzedHandle);
        }
    }
    return true;
}

static bool UsbDdkSendPipeRequestFuzz(const uint8_t *data, size_t size)
{
    constexpr size_t minSize =
        INTERFACE_HANDLE_SIZE + TIMEOUT_SIZE + ENDPOINT_SIZE + MEM_OFFSET_SIZE + BUFFER_LENGTH_SIZE;
    if (data == nullptr || size < minSize) {
        return false;
    }
    (void)OH_Usb_SendPipeRequest(nullptr, nullptr);

    UsbRequestPipe pipe = {};
    if (memcpy_s(&pipe.interfaceHandle, sizeof(pipe.interfaceHandle), data, INTERFACE_HANDLE_SIZE) != EOK) {
        return false;
    }
    size_t offset = INTERFACE_HANDLE_SIZE;
    if (memcpy_s(&pipe.timeout, sizeof(pipe.timeout), data + offset, TIMEOUT_SIZE) != EOK) {
        return false;
    }
    pipe.timeout = pipe.timeout % (MAX_PIPE_TIMEOUT_MS + 1);
    offset += TIMEOUT_SIZE;
    pipe.endpoint = data[offset];
    offset += ENDPOINT_SIZE;

    (void)OH_Usb_SendPipeRequest(&pipe, nullptr);

    uint32_t memOffset = 0;
    uint32_t bufferLength = 0;
    if (memcpy_s(&memOffset, sizeof(memOffset), data + offset, MEM_OFFSET_SIZE) != EOK) {
        return false;
    }
    offset += MEM_OFFSET_SIZE;
    if (memcpy_s(&bufferLength, sizeof(bufferLength), data + offset, BUFFER_LENGTH_SIZE) != EOK) {
        return false;
    }

    uint8_t buffer[PIPE_FUZZ_BUFFER_SIZE] = {0};
    UsbDeviceMemMap devMmap = {buffer, PIPE_FUZZ_BUFFER_SIZE, memOffset, bufferLength, 0};
    (void)OH_Usb_SendPipeRequest(&pipe, &devMmap);
    return true;
}

static bool UsbDdkPipeRequestLifecycleFuzz(const uint8_t *data, size_t size)
{
    constexpr size_t minSize =
        DEVICE_ID_SIZE + INTERFACE_HANDLE_SIZE + TIMEOUT_SIZE + ENDPOINT_SIZE + MMAP_SIZE_FIELD_SIZE;
    if (data == nullptr || size < minSize) {
        return false;
    }
    (void)OH_Usb_Init();

    uint64_t deviceId = 0;
    if (memcpy_s(&deviceId, sizeof(deviceId), data, DEVICE_ID_SIZE) != EOK) {
        return false;
    }
    size_t offset = DEVICE_ID_SIZE;

    UsbRequestPipe pipe = {};
    if (memcpy_s(&pipe.interfaceHandle, sizeof(pipe.interfaceHandle), data + offset, INTERFACE_HANDLE_SIZE) != EOK) {
        return false;
    }
    offset += INTERFACE_HANDLE_SIZE;
    if (memcpy_s(&pipe.timeout, sizeof(pipe.timeout), data + offset, TIMEOUT_SIZE) != EOK) {
        return false;
    }
    pipe.timeout = pipe.timeout % (MAX_PIPE_TIMEOUT_MS + 1);
    offset += TIMEOUT_SIZE;
    pipe.endpoint = data[offset];
    offset += ENDPOINT_SIZE;

    uint64_t mmapSize = 0;
    if (memcpy_s(&mmapSize, sizeof(mmapSize), data + offset, MMAP_SIZE_FIELD_SIZE) != EOK) {
        return false;
    }
    mmapSize = mmapSize % (MAX_MEM_MAP_SIZE + 1);

    UsbDeviceMemMap *devMmap = nullptr;
    if (OH_Usb_CreateDeviceMemMap(deviceId, static_cast<size_t>(mmapSize), &devMmap) == USB_DDK_SUCCESS) {
        (void)OH_Usb_SendPipeRequest(&pipe, devMmap);
        OH_Usb_DestroyDeviceMemMap(devMmap);
    }
    return true;
}

static bool UsbDdkCreateDeviceMemMapFuzz(const uint8_t *data, size_t size)
{
    constexpr size_t minSize = DEVICE_ID_SIZE + MMAP_SIZE_FIELD_SIZE;
    if (data == nullptr || size < minSize) {
        return false;
    }
    (void)OH_Usb_Init();

    uint64_t deviceId = 0;
    if (memcpy_s(&deviceId, sizeof(deviceId), data, DEVICE_ID_SIZE) != EOK) {
        return false;
    }
    uint64_t mmapSize = 0;
    if (memcpy_s(&mmapSize, sizeof(mmapSize), data + DEVICE_ID_SIZE, MMAP_SIZE_FIELD_SIZE) != EOK) {
        return false;
    }
    mmapSize = mmapSize % (MAX_MEM_MAP_SIZE + 1);

    (void)OH_Usb_CreateDeviceMemMap(deviceId, static_cast<size_t>(mmapSize), nullptr);

    UsbDeviceMemMap *devMmap = nullptr;
    if (OH_Usb_CreateDeviceMemMap(deviceId, static_cast<size_t>(mmapSize), &devMmap) == USB_DDK_SUCCESS) {
        OH_Usb_DestroyDeviceMemMap(devMmap);
    }
    OH_Usb_DestroyDeviceMemMap(nullptr);
    return true;
}

static bool UsbDdkParseConfigDescriptorFuzz(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    std::vector<uint8_t> configBuffer(data, data + size);
    UsbDdkConfigDescriptor *config = nullptr;
    int32_t ret = ParseUsbConfigDescriptor(configBuffer, &config);
    if (ret >= USB_DDK_SUCCESS && config != nullptr) {
        FreeUsbConfigDescriptor(config);
    }
    return true;
}

static bool UsbDdkReadPipeFuzz(const uint8_t *data, size_t size)
{
    constexpr size_t minSize =
        INTERFACE_HANDLE_SIZE + TIMEOUT_SIZE + ENDPOINT_SIZE + MEM_OFFSET_SIZE + BUFFER_LENGTH_SIZE;
    if (data == nullptr || size < minSize) {
        return false;
    }
    (void)OH_Usb_ReadPipe(nullptr, nullptr, 0, 0, 0, nullptr);

    UsbRequestPipe pipe = {};
    size_t offset = 0;
    if (memcpy_s(&pipe.interfaceHandle, sizeof(pipe.interfaceHandle), data + offset, INTERFACE_HANDLE_SIZE) != EOK) {
        return false;
    }
    offset += INTERFACE_HANDLE_SIZE;
    if (memcpy_s(&pipe.timeout, sizeof(pipe.timeout), data + offset, TIMEOUT_SIZE) != EOK) {
        return false;
    }
    pipe.timeout = pipe.timeout % (MAX_PIPE_TIMEOUT_MS + 1);
    offset += TIMEOUT_SIZE;
    pipe.endpoint = data[offset];
    offset += ENDPOINT_SIZE;

    uint32_t memOffset = 0;
    uint32_t bufferLength = 0;
    if (memcpy_s(&memOffset, sizeof(memOffset), data + offset, MEM_OFFSET_SIZE) != EOK) {
        return false;
    }
    offset += MEM_OFFSET_SIZE;
    if (memcpy_s(&bufferLength, sizeof(bufferLength), data + offset, BUFFER_LENGTH_SIZE) != EOK) {
        return false;
    }

    uint8_t buffer[PIPE_FUZZ_BUFFER_SIZE] = {0};
    uint32_t transferredLength = 0;
    (void)OH_Usb_ReadPipe(&pipe, buffer, PIPE_FUZZ_BUFFER_SIZE, memOffset, bufferLength, &transferredLength);
    return true;
}

static bool UsbDdkWritePipeFuzz(const uint8_t *data, size_t size)
{
    constexpr size_t minSize =
        INTERFACE_HANDLE_SIZE + TIMEOUT_SIZE + ENDPOINT_SIZE + MEM_OFFSET_SIZE + BUFFER_LENGTH_SIZE;
    if (data == nullptr || size < minSize) {
        return false;
    }
    (void)OH_Usb_WritePipe(nullptr, nullptr, 0, 0, 0, nullptr);

    UsbRequestPipe pipe = {};
    size_t offset = 0;
    if (memcpy_s(&pipe.interfaceHandle, sizeof(pipe.interfaceHandle), data + offset, INTERFACE_HANDLE_SIZE) != EOK) {
        return false;
    }
    offset += INTERFACE_HANDLE_SIZE;
    if (memcpy_s(&pipe.timeout, sizeof(pipe.timeout), data + offset, TIMEOUT_SIZE) != EOK) {
        return false;
    }
    pipe.timeout = pipe.timeout % (MAX_PIPE_TIMEOUT_MS + 1);
    offset += TIMEOUT_SIZE;
    pipe.endpoint = data[offset];
    offset += ENDPOINT_SIZE;

    uint32_t memOffset = 0;
    uint32_t bufferLength = 0;
    if (memcpy_s(&memOffset, sizeof(memOffset), data + offset, MEM_OFFSET_SIZE) != EOK) {
        return false;
    }
    offset += MEM_OFFSET_SIZE;
    if (memcpy_s(&bufferLength, sizeof(bufferLength), data + offset, BUFFER_LENGTH_SIZE) != EOK) {
        return false;
    }

    uint8_t buffer[PIPE_FUZZ_BUFFER_SIZE] = {0};
    uint32_t transferredLength = 0;
    (void)OH_Usb_WritePipe(&pipe, buffer, PIPE_FUZZ_BUFFER_SIZE, memOffset, bufferLength, &transferredLength);
    return true;
}

using FuzzTestFunc = bool (*)(const uint8_t *data, size_t size);

static FuzzTestFunc g_fuzzTestFuncs[] = {
    UsbDdkInitFuzz,
    UsbDdkReleaseFuzz,
    UsbDdkClaimInterfaceFuzz,
    UsbDdkReleaseInterfaceFuzz,
    UsbDdkInterfaceLifecycleFuzz,
    UsbDdkSendPipeRequestFuzz,
    UsbDdkPipeRequestLifecycleFuzz,
    UsbDdkCreateDeviceMemMapFuzz,
    UsbDdkParseConfigDescriptorFuzz,
    UsbDdkReadPipeFuzz,
    UsbDdkWritePipeFuzz,
};

static bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OPERATOR_INDEX_SIZE) {
        return false;
    }
    (void)OH_Usb_Init();
    uint8_t index = data[0];
    size_t funcCount = sizeof(g_fuzzTestFuncs) / sizeof(g_fuzzTestFuncs[0]);
    FuzzTestFunc func = g_fuzzTestFuncs[index % funcCount];
    if (func == nullptr) {
        return false;
    }
    return func(data + OPERATOR_INDEX_SIZE, size - OPERATOR_INDEX_SIZE);
}
} // namespace ExternalDeviceManager
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::ExternalDeviceManager::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
