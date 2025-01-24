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

#include "usb_serial_api.h"
#include <cerrno>
#include <iproxy_broker.h>
#include <memory.h>
#include <securec.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>

#include "hilog_wrapper.h"
#include "v1_0/iusb_serial_ddk.h"
#include "ipc_error_code.h"

using namespace OHOS;
using namespace OHOS::ExternalDeviceManager;
namespace {
OHOS::sptr<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk> g_serialDdk = nullptr;
static OHOS::sptr<IRemoteObject::DeathRecipient> recipient_ = nullptr;
std::mutex g_mutex;

constexpr uint32_t MAX_BUFFER_SIZE = 4096;
} // namespace

struct UsbSerial_Device {
    OHOS::HDI::Usb::UsbSerialDdk::V1_0::UsbSerialDeviceHandle impl;

    UsbSerial_Device()
    {
        impl.fd = -1;
    }
} __attribute__ ((aligned(8)));

UsbSerial_Device *NewSerialDeviceHandle()
{
    return new UsbSerial_Device;
}

void DeleteUsbSerialDeviceHandle(UsbSerial_Device **dev)
{
    if (*dev != nullptr) {
        delete *dev;
        *dev = nullptr;
    }
}

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

class UsbSerialDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};

void UsbSerialDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    EDM_LOGI(MODULE_HID_DDK, "hid_ddk remote died");
    if (g_serialDdk == nullptr) {
        return;
    }
    auto remote = OHOS::HDI::hdi_objcast<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk>(g_serialDdk);
    remote->RemoveDeathRecipient(recipient_);
    recipient_.clear();
    g_serialDdk = nullptr;
    EDM_LOGI(MODULE_USB_SERIAL_DDK, "remove death recipient success");
}

int32_t OH_UsbSerial_Init()
{
    g_serialDdk = OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk::Get();
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "get ddk failed");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    recipient_ = new UsbSerialDeathRecipient();
    sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<OHOS::HDI::Usb::UsbSerialDdk::V1_0::IUsbSerialDdk>(g_serialDdk);
    if (!remote->AddDeathRecipient(recipient_)) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "add DeathRecipient failed");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    return TransToUsbSerialCode(g_serialDdk->Init());
}

int32_t OH_UsbSerial_Release()
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    int32_t ret = g_serialDdk->Release();
    g_serialDdk.clear();
    return TransToUsbSerialCode(ret);
}

int32_t OH_UsbSerial_Open(uint64_t deviceId, uint8_t interfaceIndex, UsbSerial_Device **dev)
{
    if (g_serialDdk == nullptr) {
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
    return TransToUsbSerialCode(g_serialDdk->Open(deviceId, interfaceIndex, (*dev)->impl));
}

int32_t OH_UsbSerial_Close(UsbSerial_Device **dev)
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || *dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    int32_t ret = g_serialDdk->Close((*dev)->impl);
    DeleteUsbSerialDeviceHandle(dev);
    return TransToUsbSerialCode(ret);
}

int32_t OH_UsbSerial_Read(UsbSerial_Device *dev, uint8_t *buff, uint32_t bufferSize, uint32_t *bytesRead)
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || buff == nullptr || bufferSize == 0 || bytesRead == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    uint32_t readBufferSize = bufferSize;
    if (readBufferSize > MAX_BUFFER_SIZE) {
        readBufferSize = MAX_BUFFER_SIZE;
    }
    std::vector<uint8_t> readBuff;
    int32_t ret = g_serialDdk->Read(dev->impl, readBufferSize, readBuff);
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
}

int32_t OH_UsbSerial_Write(UsbSerial_Device *dev, uint8_t *buff, uint32_t bufferSize, uint32_t *bytesWritten)
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || buff == nullptr || bufferSize == 0 || bytesWritten == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    std::vector<uint8_t> writeBuff(buff, buff + bufferSize);
    int32_t ret = g_serialDdk->Write(dev->impl, writeBuff, *bytesWritten);
    if (ret != HDF_SUCCESS) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "write error.");
        *bytesWritten = 0;
        return TransToUsbSerialCode(ret);
    }
    return USB_SERIAL_DDK_SUCCESS;
}

int32_t OH_UsbSerial_SetBaudRate(UsbSerial_Device *dev, uint32_t baudRate)
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(g_serialDdk->SetBaudRate(dev->impl, baudRate));
}

int32_t OH_UsbSerial_SetParams(UsbSerial_Device *dev, UsbSerial_Params *params)
{
    if (g_serialDdk == nullptr) {
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
    return TransToUsbSerialCode(g_serialDdk->SetParams(dev->impl, hidParams));
}

int32_t OH_UsbSerial_SetTimeout(UsbSerial_Device *dev, int timeout)
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(g_serialDdk->SetTimeout(dev->impl, timeout));
}

int32_t OH_UsbSerial_SetFlowControl(UsbSerial_Device *dev, UsbSerial_FlowControl flowControl)
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    auto hdiFlowControl = (OHOS::HDI::Usb::UsbSerialDdk::V1_0::UsbSerialFlowControl)flowControl;
    return TransToUsbSerialCode(g_serialDdk->SetFlowControl(dev->impl, hdiFlowControl));
}

int32_t OH_UsbSerial_Flush(UsbSerial_Device *dev)
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(g_serialDdk->Flush(dev->impl));
}

int32_t OH_UsbSerial_FlushInput(UsbSerial_Device *dev)
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(g_serialDdk->FlushInput(dev->impl));
}

int32_t OH_UsbSerial_FlushOutput(UsbSerial_Device *dev)
{
    if (g_serialDdk == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "ddk is null");
        return USB_SERIAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_USB_SERIAL_DDK, "param is null");
        return USB_SERIAL_DDK_INVALID_PARAMETER;
    }
    return TransToUsbSerialCode(g_serialDdk->FlushOutput(dev->impl));
}
