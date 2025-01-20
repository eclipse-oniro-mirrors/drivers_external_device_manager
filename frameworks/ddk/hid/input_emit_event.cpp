/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iproxy_broker.h>
#include <memory.h>
#include <securec.h>
#include <unordered_map>
#include <vector>

#include "hid_ddk_api.h"
#include "hid_ddk_types.h"
#include "v1_1/ihid_ddk.h"
#include "hilog_wrapper.h"
#include "ipc_error_code.h"

using namespace OHOS;
using namespace OHOS::ExternalDeviceManager;
namespace {
static OHOS::sptr<OHOS::HDI::Input::Ddk::V1_1::IHidDdk> g_ddk = nullptr;
static OHOS::sptr<IRemoteObject::DeathRecipient> recipient_ = nullptr;
std::mutex g_mutex;

constexpr uint32_t MAX_EMIT_ITEM_NUM = 20;
constexpr uint32_t MAX_HID_DEVICE_PROP_LEN = 7;
constexpr uint32_t MAX_HID_EVENT_TYPES_LEN = 5;
constexpr uint32_t MAX_HID_KEYS_LEN = 100;
constexpr uint32_t MAX_HID_ABS_LEN = 26;
constexpr uint32_t MAX_HID_REL_BITS_LEN = 13;
constexpr uint32_t MAX_HID_MISC_EVENT_LEN = 6;
constexpr uint32_t MAX_NAME_LENGTH = 80;

}
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
static std::unordered_map<int32_t, std::shared_ptr<struct TempDevice>> g_deviceMap;

struct TempDevice {
    OHOS::HDI::Input::Ddk::V1_0::Hid_Device tempDevice;
    OHOS::HDI::Input::Ddk::V1_0::Hid_EventProperties tempProperties;
    uint32_t realId;
};

struct Hid_DeviceHandle {
    OHOS::HDI::Input::Ddk::V1_1::HidDeviceHandle impl;

    Hid_DeviceHandle()
    {
        impl.fd = -1;
        impl.nonBlock = 0;
    }
} __attribute__ ((aligned(8)));

Hid_DeviceHandle *NewHidDeviceHandle()
{
    return new Hid_DeviceHandle;
}

void DeleteHidDeviceHandle(Hid_DeviceHandle **dev)
{
    if (*dev != nullptr) {
        delete *dev;
        *dev = nullptr;
    }
}

static int32_t TransToHidCode(int32_t ret)
{
    if (ret >= OH_IPC_ERROR_CODE_BASE && ret <= OH_IPC_ERROR_CODE_MAX) {
        return HID_DDK_SERVICE_ERROR;
    }
    return ret;
}

class HidDeathRecipient : public IRemoteObject::DeathRecipient {
public:
        void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};

void HidDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    EDM_LOGI(MODULE_HID_DDK, "hid_ddk remote died");
    if (g_ddk != nullptr) {
        sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<OHOS::HDI::Input::Ddk::V1_0::IHidDdk>(g_ddk);
        remote->RemoveDeathRecipient(recipient_);
        recipient_.clear();
        g_ddk = nullptr;
        EDM_LOGI(MODULE_HID_DDK, "remove death recipient success");
    }
}

static uint32_t GetRealDeviceId(int32_t deviceId)
{
    if (g_deviceMap.find(deviceId) != g_deviceMap.end()) {
        if (g_deviceMap[deviceId] != nullptr) {
            return g_deviceMap[deviceId]->realId;
        }
    }
    return static_cast<uint32_t>(deviceId);
}

static int32_t Connect()
{
    if (g_ddk == nullptr) {
        g_ddk = OHOS::HDI::Input::Ddk::V1_1::IHidDdk::Get();
        if (g_ddk == nullptr) {
            EDM_LOGE(MODULE_HID_DDK, "get hid ddk faild");
            return HID_DDK_FAILURE;
        }
        if (g_deviceMap.size() > 0) {
            for (const auto &[_, value] : g_deviceMap) {
                (void)g_ddk->CreateDevice(value->tempDevice, value->tempProperties, value->realId);
            }
        }
        recipient_ = new HidDeathRecipient();
        sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<OHOS::HDI::Input::Ddk::V1_1::IHidDdk>(g_ddk);
        if (!remote->AddDeathRecipient(recipient_)) {
            EDM_LOGE(MODULE_HID_DDK, "add DeathRecipient failed");
            return HID_DDK_FAILURE;
        }
    }
    return HID_DDK_SUCCESS;
}

static OHOS::HDI::Input::Ddk::V1_0::Hid_Device ParseHidDevice(Hid_Device *hidDevice)
{
    OHOS::HDI::Input::Ddk::V1_0::Hid_Device tempDevice = {
        .deviceName = hidDevice->deviceName,
        .vendorId = hidDevice->vendorId,
        .productId = hidDevice->productId,
        .version = hidDevice->version,
        .bustype = hidDevice->bustype
    };

    if (hidDevice->properties != nullptr) {
        std::transform(hidDevice->properties, hidDevice->properties + hidDevice->propLength,
            std::back_inserter(tempDevice.properties), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_DeviceProp>(n);
            });
    }

    return tempDevice;
}

static OHOS::HDI::Input::Ddk::V1_0::Hid_EventProperties ParseHidEventProperties(Hid_EventProperties *hidEventProperties)
{
    const uint16_t absLength = 64;
    OHOS::HDI::Input::Ddk::V1_0::Hid_EventProperties tempProperties = {
        .hidAbsMax = std::vector<int32_t>(hidEventProperties->hidAbsMax, hidEventProperties->hidAbsMax + absLength),
        .hidAbsMin = std::vector<int32_t>(hidEventProperties->hidAbsMin, hidEventProperties->hidAbsMin + absLength),
        .hidAbsFuzz = std::vector<int32_t>(hidEventProperties->hidAbsFuzz, hidEventProperties->hidAbsFuzz + absLength),
        .hidAbsFlat = std::vector<int32_t>(hidEventProperties->hidAbsFlat, hidEventProperties->hidAbsFlat + absLength)
    };

    if (hidEventProperties->hidEventTypes.hidEventType != nullptr) {
        std::transform(hidEventProperties->hidEventTypes.hidEventType,
            hidEventProperties->hidEventTypes.hidEventType + hidEventProperties->hidEventTypes.length,
            std::back_inserter(tempProperties.hidEventTypes), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_EventType>(n);
            });
    }

    if (hidEventProperties->hidKeys.hidKeyCode != nullptr) {
        std::transform(hidEventProperties->hidKeys.hidKeyCode,
            hidEventProperties->hidKeys.hidKeyCode + hidEventProperties->hidKeys.length,
            std::back_inserter(tempProperties.hidKeys), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_KeyCode>(n);
            });
    }
    
    if (hidEventProperties->hidAbs.hidAbsAxes != nullptr) {
        std::transform(hidEventProperties->hidAbs.hidAbsAxes,
            hidEventProperties->hidAbs.hidAbsAxes + hidEventProperties->hidAbs.length,
            std::back_inserter(tempProperties.hidAbs), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_AbsAxes>(n);
            });
    }

    if (hidEventProperties->hidRelBits.hidRelAxes != nullptr) {
        std::transform(hidEventProperties->hidRelBits.hidRelAxes,
            hidEventProperties->hidRelBits.hidRelAxes + hidEventProperties->hidRelBits.length,
            std::back_inserter(tempProperties.hidRelBits), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_RelAxes>(n);
            });
    }

    if (hidEventProperties->hidMiscellaneous.hidMscEvent != nullptr) {
        std::transform(hidEventProperties->hidMiscellaneous.hidMscEvent,
            hidEventProperties->hidMiscellaneous.hidMscEvent + hidEventProperties->hidMiscellaneous.length,
            std::back_inserter(tempProperties.hidMiscellaneous), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_MscEvent>(n);
            });
    }

    return tempProperties;
}

static int32_t CacheDeviceInfor(OHOS::HDI::Input::Ddk::V1_0::Hid_Device tempDevice,
    OHOS::HDI::Input::Ddk::V1_0::Hid_EventProperties tempProperties, uint32_t deviceId)
{
    EDM_LOGD(MODULE_HID_DDK, "enter CacheDeviceInfor");
    int32_t id = static_cast<int32_t>(deviceId);
    std::shared_ptr<struct TempDevice> device = std::make_shared<struct TempDevice>();
    device->tempDevice = tempDevice;
    device->tempProperties = tempProperties;
    device->realId = deviceId;

    g_deviceMap[id] = device;
    return id;
}

static bool CheckHidDevice(Hid_Device *hidDevice)
{
    if (hidDevice == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "hidDevice is null");
        return false;
    }
    
    if (hidDevice->propLength > MAX_HID_DEVICE_PROP_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "properties length is out of range");
        return false;
    }
    
    if (hidDevice->deviceName == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "hidDevice->deviceName is nullpointer");
        return false;
    }
    
    if (strlen(hidDevice->deviceName) == 0 || strlen(hidDevice->deviceName) > MAX_NAME_LENGTH - 1) {
        EDM_LOGE(MODULE_HID_DDK, "length of hidDevice->deviceName is out of range");
        return false;
    }
    return true;
}

int32_t OH_Hid_CreateDevice(Hid_Device *hidDevice, Hid_EventProperties *hidEventProperties)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (Connect() != HID_DDK_SUCCESS) {
        return HID_DDK_INVALID_OPERATION;
    }

    if (!CheckHidDevice(hidDevice)) {
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "hidEventProperties is null");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidEventTypes.length > MAX_HID_EVENT_TYPES_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidEventTypes length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidKeys.length > MAX_HID_KEYS_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidKeys length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidAbs.length > MAX_HID_ABS_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidAbs length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidRelBits.length > MAX_HID_REL_BITS_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidRelBits length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidMiscellaneous.length > MAX_HID_MISC_EVENT_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidMiscellaneous length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    auto tempDevice = ParseHidDevice(hidDevice);
    auto tempEventProperties = ParseHidEventProperties(hidEventProperties);

    uint32_t deviceId = 0;
    auto ret = g_ddk->CreateDevice(tempDevice, tempEventProperties, deviceId);
    ret = (ret == HDF_ERR_NOPERM) ? HID_DDK_NO_PERM : ret;
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "create device failed:%{public}d", ret);
        return ret;
    }
    return CacheDeviceInfor(tempDevice, tempEventProperties, deviceId);
}

int32_t OH_Hid_EmitEvent(int32_t deviceId, const Hid_EmitItem items[], uint16_t length)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (Connect() != HID_DDK_SUCCESS) {
        return HID_DDK_INVALID_OPERATION;
    }

    if (deviceId < 0) {
        EDM_LOGE(MODULE_HID_DDK, "device id is invaild");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (length > MAX_EMIT_ITEM_NUM) {
        EDM_LOGE(MODULE_HID_DDK, "items length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (items == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "items is null");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<OHOS::HDI::Input::Ddk::V1_0::Hid_EmitItem> itemsTemp;
    std::transform(items, items + length, std::back_inserter(itemsTemp), [](Hid_EmitItem item) {
        return *reinterpret_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_EmitItem *>(&item);
    });

    auto ret = g_ddk->EmitEvent(GetRealDeviceId(deviceId), itemsTemp);
    ret = (ret == HDF_ERR_NOPERM) ? HID_DDK_NO_PERM : ret;
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "emit event failed:%{public}d", ret);
        return ret;
    }
    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_DestroyDevice(int32_t deviceId)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (Connect() != HID_DDK_SUCCESS) {
        return HID_DDK_INVALID_OPERATION;
    }

    auto ret = g_ddk->DestroyDevice(GetRealDeviceId(deviceId));
    ret = (ret == HDF_ERR_NOPERM) ? HID_DDK_NO_PERM : ret;
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "destroy device failed:%{public}d", ret);
        return ret;
    }

    g_deviceMap.erase(deviceId);
    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_Init(void)
{
    g_ddk = OHOS::HDI::Input::Ddk::V1_1::IHidDdk::Get();
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "get ddk failed");
        return HID_DDK_INIT_ERROR;
    }

    return TransToHidCode(g_ddk->Init());
}

int32_t OH_Hid_Release(void)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "ddk is null");
        return HID_DDK_INIT_ERROR;
    }
    int32_t ret = g_ddk->Release();
    g_ddk.clear();

    return TransToHidCode(ret);
}

int32_t OH_Hid_Open(uint64_t deviceId, uint8_t interfaceIndex, Hid_DeviceHandle **dev)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    *dev = NewHidDeviceHandle();
    if (*dev == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "malloc failed, errno=%{public}d", errno);
        return HID_DDK_MEMORY_ERROR;
    }

    return TransToHidCode(g_ddk->Open(deviceId, interfaceIndex, (*dev)->impl));
}

int32_t OH_Hid_Close(Hid_DeviceHandle **dev)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }
    if (dev == nullptr || *dev == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    int32_t ret = g_ddk->Close((*dev)->impl);
    DeleteHidDeviceHandle(dev);

    return TransToHidCode(ret);
}

int32_t OH_Hid_Write(Hid_DeviceHandle *dev, uint8_t *data, uint32_t length, uint32_t *bytesWritten)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || data == nullptr || length == 0 || length > HID_MAX_REPORT_BUFFER_SIZE ||
        bytesWritten == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<uint8_t> writeData(data, data + length);
    int32_t ret =  TransToHidCode(g_ddk->Write(dev->impl, writeData, *bytesWritten));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "write failed");
        *bytesWritten = 0;
        return ret;
    }
    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_ReadTimeout(Hid_DeviceHandle *dev, uint8_t *data, uint32_t bufSize, int timeout, uint32_t *bytesRead)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || data == nullptr || bufSize == 0 || bufSize > HID_MAX_REPORT_BUFFER_SIZE ||
        bytesRead == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<uint8_t> readData(bufSize);
    int32_t ret = TransToHidCode(g_ddk->ReadTimeout(dev->impl, readData, bufSize, timeout, *bytesRead));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "read timeout failed");
        return ret;
    }
    errno_t err = memcpy_s(data, bufSize, readData.data(), *bytesRead);
    if (err != EOK) {
        EDM_LOGE(MODULE_HID_DDK, "memcpy_s failed");
        return HID_DDK_MEMORY_ERROR;
    }

    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_Read(Hid_DeviceHandle *dev, uint8_t *data, uint32_t bufSize, uint32_t *bytesRead)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || data == nullptr || bufSize == 0 || bufSize > HID_MAX_REPORT_BUFFER_SIZE ||
        bytesRead == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<uint8_t> readData(bufSize);
    int32_t ret = TransToHidCode(g_ddk->ReadTimeout(dev->impl, readData, bufSize, (dev->impl.nonBlock) ? 0 : -1,
        *bytesRead));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "read failed");
        return ret;
    }
    errno_t err = memcpy_s(data, bufSize, readData.data(), *bytesRead);
    if (err != EOK) {
        EDM_LOGE(MODULE_HID_DDK, "memcpy_s failed");
        return HID_DDK_MEMORY_ERROR;
    }

    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_SetNonBlocking(Hid_DeviceHandle *dev, int nonBlock)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || !(nonBlock == 0 || nonBlock == 1)) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    int32_t ret = g_ddk->SetNonBlocking(dev->impl, nonBlock);
    if (ret == HID_DDK_SUCCESS) {
        dev->impl.nonBlock = nonBlock;
    }
    return TransToHidCode(ret);
}

int32_t OH_Hid_GetRawInfo(Hid_DeviceHandle *dev, Hid_RawDevInfo *rawDevInfo)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || rawDevInfo == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    OHOS::HDI::Input::Ddk::V1_1::HidRawDevInfo tmpRawDevInfo;
    int32_t ret = TransToHidCode(g_ddk->GetRawInfo(dev->impl, tmpRawDevInfo));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "get raw info failed");
        return ret;
    }

    rawDevInfo->busType = tmpRawDevInfo.busType;
    rawDevInfo->vendor = tmpRawDevInfo.vendor;
    rawDevInfo->product = tmpRawDevInfo.product;

    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_GetRawName(Hid_DeviceHandle *dev, char *data, uint32_t bufSize)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || data == nullptr || bufSize == 0 || bufSize > HID_MAX_REPORT_BUFFER_SIZE) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<uint8_t> tmpData(bufSize);
    int32_t ret = TransToHidCode(g_ddk->GetRawName(dev->impl, tmpData, bufSize));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "get raw name failed");
        return ret;
    }

    errno_t err = memcpy_s(data, bufSize, tmpData.data(), tmpData.size());
    if (err != EOK) {
        EDM_LOGE(MODULE_HID_DDK, "memcpy_s failed");
        return HID_DDK_MEMORY_ERROR;
    }

    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_GetPhysicalAddress(Hid_DeviceHandle *dev, char *data, uint32_t bufSize)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || data == nullptr || bufSize == 0 || bufSize > HID_MAX_REPORT_BUFFER_SIZE) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<uint8_t> tmpData(bufSize);
    int32_t ret = TransToHidCode(g_ddk->GetPhysicalAddress(dev->impl, tmpData, bufSize));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "get physical address failed");
        return ret;
    }

    errno_t err = memcpy_s(data, bufSize, tmpData.data(), tmpData.size());
    if (err != EOK) {
        EDM_LOGE(MODULE_HID_DDK, "memcpy_s failed");
        return HID_DDK_MEMORY_ERROR;
    }

    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_GetRawUniqueId(Hid_DeviceHandle *dev, uint8_t *data, uint32_t bufSize)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || data == nullptr || bufSize == 0 || bufSize > HID_MAX_REPORT_BUFFER_SIZE) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<uint8_t> tmpData(bufSize);
    int32_t ret = TransToHidCode(g_ddk->GetRawUniqueId(dev->impl, tmpData, bufSize));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "get raw unique id address failed");
        return ret;
    }

    errno_t err = memcpy_s(data, tmpData.size(), tmpData.data(), tmpData.size());
    if (err != EOK) {
        EDM_LOGE(MODULE_HID_DDK, "memcpy_s failed");
        return HID_DDK_MEMORY_ERROR;
    }

    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_SendReport(Hid_DeviceHandle *dev, Hid_ReportType reportType, const uint8_t *data, uint32_t length)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || data == nullptr || length == 0 || length > HID_MAX_REPORT_BUFFER_SIZE) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    const std::vector<uint8_t> tmpData(data, data + length);
    auto tmpReportType = static_cast<OHOS::HDI::Input::Ddk::V1_1::HidReportType>(reportType);
    int32_t ret = TransToHidCode(g_ddk->SendReport(dev->impl, tmpReportType, tmpData));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "send report failed");
        return ret;
    }

    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_GetReport(Hid_DeviceHandle *dev, Hid_ReportType reportType, uint8_t *data, uint32_t bufSize)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || data == nullptr || bufSize == 0 || bufSize > HID_MAX_REPORT_BUFFER_SIZE) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<uint8_t> tmpData(bufSize);
    auto reportNumber = data[0];
    auto tmpReportType = static_cast<OHOS::HDI::Input::Ddk::V1_1::HidReportType>(reportType);
    int32_t ret = TransToHidCode(g_ddk->GetReport(dev->impl, tmpReportType, reportNumber, tmpData, bufSize));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "get report failed");
        return ret;
    }

    errno_t err = memcpy_s(data, bufSize, tmpData.data(), tmpData.size());
    if (err != EOK) {
        EDM_LOGE(MODULE_HID_DDK, "memcpy_s failed");
        return HID_DDK_MEMORY_ERROR;
    }

    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_GetReportDescriptor(Hid_DeviceHandle *dev, uint8_t *buf, uint32_t bufSize, uint32_t *bytesRead)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid obj");
        return HID_DDK_INIT_ERROR;
    }

    if (dev == nullptr || buf == nullptr || bufSize == 0 || bufSize > HID_MAX_REPORT_BUFFER_SIZE ||
        bytesRead == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "invalid param");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<uint8_t> tmpBuf(bufSize);
    int32_t ret = TransToHidCode(g_ddk->GetReportDescriptor(dev->impl, tmpBuf, bufSize, *bytesRead));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "get report descriptor failed");
        return ret;
    }

    errno_t err = memcpy_s(buf, bufSize, tmpBuf.data(), *bytesRead);
    if (err != EOK) {
        EDM_LOGE(MODULE_HID_DDK, "memcpy_s failed");
        return HID_DDK_MEMORY_ERROR;
    }

    return HID_DDK_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
