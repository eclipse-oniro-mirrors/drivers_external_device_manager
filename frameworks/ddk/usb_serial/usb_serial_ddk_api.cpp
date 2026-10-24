/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "usb_serial_api.h"
#include <cerrno>
#include <memory.h>
#include <mutex>
#include <securec.h>
#include <shared_mutex>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>

#include "hilog_wrapper.h"
#include "v1_0/iusb_serial_ddk.h"
#include "ipc_error_code.h"
#include <iproxy_broker.h>

using namespace OHOS;
using namespace OHOS::ExternalDeviceManager;
namespace {
OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> g_serialDdk = nullptr;
static OHOS::sptr<IRemoteObject::DeathRecipient> recipient_ = nullptr;
std::shared_mutex g_mutex;
static std::mutex g_lifecycleMutex;
static bool g_initialized = false;

OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> GetDdk()
{
    std::shared_lock<std::shared_mutex> lock(g_mutex);
    return g_serialDdk;
}

#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
constexpr uint32_t MAX_BUFFER_SIZE = 4096;
#endif

} // namespace

struct UsbSerial_Device {
    OHOS::HDI::Usb::UsbSerialDdk::V1_0::UsbSerialDeviceHandle impl;

    UsbSerial_Device()
    {
        impl.fd = -1;
    }
} __attribute__ ((aligned(8)));

UsbSerial_Device *NewSerialDeviceHandle(void)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    return new UsbSerial_Device;
#else
    return nullptr;
#endif
}

void DeleteUsbSerialDeviceHandle(UsbSerial_Device **dev)
{
#ifndef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    (void)dev;
#else
    if (*dev != nullptr) {
        delete *dev;
        *dev = nullptr;
    }
#endif
}

void SetDdk(OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> &ddk)
{
    g_serialDdk = ddk;
}

#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
static int32_t TransToUsbSerialCode(int32_t ret)
{
    if (ret == HDF_SUCCESS) {
        return USB_SERIAL_DDK_SUCCESS;
    }
    if (ret >= OH_IPC_ERROR_CODE_BASE && ret <= OH_IPC_ERROR_CODE_MAX) {
        return USB_SERIAL_DDK_SERVICE_ERROR;
    }
    return ret;
}
#endif

class UsbSerialDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};

static void UnregisterDeathRecipient(
    const OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> &ddk,
    const OHOS::sptr<IRemoteObject::DeathRecipient> &recipient)
{
    if (ddk == nullptr || recipient == nullptr) {
        return;
    }
    auto remote =
        OHOS::HDI::hdi_objcast<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk>(ddk);
    if (remote != nullptr) {
        remote->RemoveDeathRecipient(recipient);
    }
}

void UsbSerialDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    (void)object;
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = nullptr;
    OHOS::sptr<IRemoteObject::DeathRecipient> recipient = nullptr;
    {
        std::unique_lock<std::shared_mutex> lock(g_mutex);
        EDM_LOGI(MODULE_USB_SERIAL_DDK, "usbSerial_ddk remote died");
        g_initialized = false;
        if (g_serialDdk == nullptr) {
            return;
        }
        ddk = g_serialDdk;
        recipient = recipient_;
        recipient_.clear();
        g_serialDdk = nullptr;
    }

    UnregisterDeathRecipient(ddk, recipient);
    EDM_LOGI(MODULE_USB_SERIAL_DDK, "remove death recipient success");
}

int32_t OH_UsbSerial_Init(void)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    std::lock_guard<std::mutex> lifecycleLock(g_lifecycleMutex);
    {
        std::shared_lock<std::shared_mutex> lock(g_mutex);
        if (g_initialized && g_serialDdk != nullptr) {
            EDM_LOGW(MODULE_USB_SERIAL_DDK, "already initialized");
            return USB_SERIAL_DDK_SUCCESS;
        }
    }

    auto ddk = OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk::Get();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "get ddk failed");
        return USB_SERIAL_DDK_INIT_ERROR;
    }

    sptr<IRemoteObject::DeathRecipient> recipient = new UsbSerialDeathRecipient();
    {
        std::unique_lock<std::shared_mutex> lock(g_mutex);
        g_serialDdk = ddk;
        recipient_ = recipient;
    }

    sptr<IRemoteObject> remote =
        OHOS::HDI::hdi_objcast<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk>(ddk);
    if (remote == nullptr || !remote->AddDeathRecipient(recipient)) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "add DeathRecipient failed");
        return USB_SERIAL_DDK_INIT_ERROR;
    }

    int32_t ret = TransToUsbSerialCode(ddk->Init());
    if (ret == USB_SERIAL_DDK_SUCCESS) {
        std::unique_lock<std::shared_mutex> lock(g_mutex);
        g_initialized = true;
    }
    return ret;
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_Release(void)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    std::lock_guard<std::mutex> lifecycleLock(g_lifecycleMutex);
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = nullptr;
    OHOS::sptr<IRemoteObject::DeathRecipient> recipient = nullptr;
    {
        std::unique_lock<std::shared_mutex> lock(g_mutex);
        if (g_serialDdk == nullptr) {
            EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
            return USB_SERIAL_DDK_INIT_ERROR;
        }
        ddk = g_serialDdk;
        recipient = recipient_;
    }
    int32_t ret = TransToUsbSerialCode(ddk->Release());
    if (ret == USB_SERIAL_DDK_SUCCESS) {
        {
            std::unique_lock<std::shared_mutex> lock(g_mutex);
            recipient_.clear();
            g_serialDdk.clear();
            g_initialized = false;
        }
        UnregisterDeathRecipient(ddk, recipient);
    }
    return ret;
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_Open(uint64_t deviceId, uint8_t interfaceIndex, UsbSerial_Device **dev)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "dev is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    *dev = NewSerialDeviceHandle();
    if (*dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "Failed to allocate memory for device, errno=%{public}d", errno);
        return USB_SERIAL_DDK_MEMORY_ERROR;
    }
    return TransToUsbSerialCode(ddk->Open(deviceId, interfaceIndex, (*dev)->impl));
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_Close(UsbSerial_Device **dev)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || *dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    int32_t ret = ddk->Close((*dev)->impl);
    DeleteUsbSerialDeviceHandle(dev);
    return TransToUsbSerialCode(ret);
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_Read(UsbSerial_Device *dev, uint8_t *buff, uint32_t bufferSize, uint32_t *bytesRead)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || buff == nullptr || bufferSize == 0 || bytesRead == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    uint32_t readBufferSize = bufferSize;
    if (readBufferSize > MAX_BUFFER_SIZE) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "bufferSize %{public}u exceeds max %{public}u, truncated", bufferSize,
            MAX_BUFFER_SIZE);
        readBufferSize = MAX_BUFFER_SIZE;
    }
    std::vector<uint8_t> readBuff;
    int32_t ret = ddk->Read(dev->impl, readBufferSize, readBuff);
    if (ret != HDF_SUCCESS) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "read error.");
        return TransToUsbSerialCode(ret);
    }
    *bytesRead = readBuff.size();
    if (readBuff.empty()) {
        return USB_SERIAL_DDK_SUCCESS;
    }
    errno_t err = memcpy_s(buff, bufferSize, readBuff.data(), readBuff.size());
    if (err != 0) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "memcpy_s error: %{public}d.", err);
        return USB_SERIAL_DDK_MEMORY_ERROR;
    }
    return USB_SERIAL_DDK_SUCCESS;
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_Write(UsbSerial_Device *dev, uint8_t *buff, uint32_t bufferSize, uint32_t *bytesWritten)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || buff == nullptr || bufferSize == 0 || bytesWritten == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    std::vector<uint8_t> writeBuff(buff, buff + bufferSize);
    int32_t ret = ddk->Write(dev->impl, writeBuff, *bytesWritten);
    if (ret != HDF_SUCCESS) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "write error.");
        *bytesWritten = 0;
        return TransToUsbSerialCode(ret);
    }
    return USB_SERIAL_DDK_SUCCESS;
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_SetBaudRate(UsbSerial_Device *dev, uint32_t baudRate)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(ddk->SetBaudRate(dev->impl, baudRate));
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_SetParams(UsbSerial_Device *dev, UsbSerial_Params *params)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || params == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    OHOS::HDI::Usb::UsbSerialDdk::V1_0::UsbSerialParams hidParams;
    hidParams.baudRate = params->baudRate;
    hidParams.nDataBits = params->nDataBits;
    hidParams.nStopBits = params->nStopBits;
    hidParams.parity = static_cast<OHOS::HDI::Usb::UsbSerialDdk::V1_0::UsbSerialParity>(params->parity);
    return TransToUsbSerialCode(ddk->SetParams(dev->impl, hidParams));
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_SetTimeout(UsbSerial_Device *dev, int timeout)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(ddk->SetTimeout(dev->impl, timeout));
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_SetFlowControl(UsbSerial_Device *dev, UsbSerial_FlowControl flowControl)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    auto hdiFlowControl = (OHOS::HDI::Usb::UsbSerialDdk::V1_0::UsbSerialFlowControl)flowControl;
    return TransToUsbSerialCode(ddk->SetFlowControl(dev->impl, hdiFlowControl));
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_Flush(UsbSerial_Device *dev)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(ddk->Flush(dev->impl));
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_FlushInput(UsbSerial_Device *dev)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(ddk->FlushInput(dev->impl));
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}

int32_t OH_UsbSerial_FlushOutput(UsbSerial_Device *dev)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> ddk = GetDdk();
    if (ddk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(ddk->FlushOutput(dev->impl));
#else
    return USB_SERIAL_DDK_INVALID_OPERATION;
#endif
}
