/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "virtual_device.h"
#include <map>

#include <fcntl.h>
#include <securec.h>
#include <unistd.h>

#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
namespace {
using namespace OHOS::HiviewDFX;
constexpr uint32_t MAX_NAME_LENGTH = 80;

bool DoIoctl(int32_t fd, int32_t request, const uint32_t value)
{
    int32_t rc = ioctl(fd, request, value);
    if (rc < 0) {
        EDM_LOGE(MODULE_USB_DDK, "Failed to ioctl");
        return false;
    }
    return true;
}
} // namespace

VirtualDevice::VirtualDevice(const char *deviceName, uint16_t productId)
    : deviceName_(deviceName), busType_(BUS_USB), vendorId_(0x6006), productId_(productId), version_(1)
{
}

VirtualDevice::VirtualDevice(Hid_Device *hidDevice, Hid_EventProperties *hidEventProperties)
    : deviceName_(hidDevice->deviceName),
    busType_(hidDevice->bustype),
    vendorId_(hidDevice->vendorId),
    productId_(hidDevice->productId),
    version_(hidDevice->version)
{
    properties_ = std::vector<uint32_t>(hidDevice->properties, hidDevice->properties + hidDevice->propLength);
    eventTypes_ = std::vector<uint32_t>(hidEventProperties->hidEventTypes.hidEventType,
        hidEventProperties->hidEventTypes.hidEventType + hidEventProperties->hidEventTypes.length);
    keys_ = std::vector<uint32_t>(hidEventProperties->hidKeys.hidKeyCode,
        hidEventProperties->hidKeys.hidKeyCode + hidEventProperties->hidKeys.length);
    abs_ = std::vector<uint32_t>(hidEventProperties->hidAbs.hidAbsAxes,
        hidEventProperties->hidAbs.hidAbsAxes + hidEventProperties->hidAbs.length);
    relBits_ = std::vector<uint32_t>(hidEventProperties->hidRelBits.hidRelAxes,
        hidEventProperties->hidRelBits.hidRelAxes + hidEventProperties->hidRelBits.length);
    miscellaneous_ = std::vector<uint32_t>(hidEventProperties->hidMiscellaneous.hidMscEvent,
        hidEventProperties->hidMiscellaneous.hidMscEvent + hidEventProperties->hidMiscellaneous.length);
    const int absLength = 64;
    std::copy(hidEventProperties->hidAbsMax, hidEventProperties->hidAbsMax + absLength, uinputDev_.absmax);
    std::copy(hidEventProperties->hidAbsMin, hidEventProperties->hidAbsMin + absLength, uinputDev_.absmin);
    std::copy(hidEventProperties->hidAbsFuzz, hidEventProperties->hidAbsFuzz + absLength, uinputDev_.absfuzz);
    std::copy(hidEventProperties->hidAbsFlat, hidEventProperties->hidAbsFlat + absLength, uinputDev_.absflat);
}

VirtualDevice::~VirtualDevice()
{
    if (fd_ >= 0) {
        ioctl(fd_, UI_DEV_DESTROY);
        close(fd_);
        fd_ = -1;
    }
}

bool VirtualDevice::SetUp()
{
    fd_ = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd_ < 0) {
        EDM_LOGE(MODULE_USB_DDK, "Failed to open uinput");
        return false;
    }

    if (!SetAttribute()) {
        EDM_LOGE(MODULE_USB_DDK, "Failed to set attribute");
        return false;
    }

    if (!CreateKey()) {
        EDM_LOGE(MODULE_USB_DDK, "Failed to create uinput KeyValue");
        return false;
    }

    errno_t ret = strncpy_s(uinputDev_.name, MAX_NAME_LENGTH, deviceName_, sizeof(uinputDev_.name));
    if (ret != EOK) {
        EDM_LOGE(MODULE_USB_DDK, "Failed to copy deviceName");
        return false;
    }
    uinputDev_.id.bustype = busType_;
    uinputDev_.id.vendor = vendorId_;
    uinputDev_.id.product = productId_;
    uinputDev_.id.version = version_;
    if (write(fd_, &uinputDev_, sizeof(uinputDev_)) < 0) {
        EDM_LOGE(MODULE_USB_DDK, "Unable to set input device info");
        return false;
    }
    if (ioctl(fd_, UI_DEV_CREATE) < 0) {
        EDM_LOGE(MODULE_USB_DDK, "Unable to create input device");
        return false;
    }
    return true;
}

bool VirtualDevice::SetAttribute()
{
    for (const auto &item : GetEventTypes()) {
        if (!DoIoctl(fd_, UI_SET_EVBIT, item)) {
            EDM_LOGE(MODULE_USB_DDK, "Error setting event type:%{public}u", item);
            return false;
        }
    }
    for (const auto &item : GetKeys()) {
        if (!DoIoctl(fd_, UI_SET_KEYBIT, item)) {
            EDM_LOGE(MODULE_USB_DDK, "Error setting key:%{public}u", item);
            return false;
        }
    }
    for (const auto &item : GetProperties()) {
        if (!DoIoctl(fd_, UI_SET_PROPBIT, item)) {
            EDM_LOGE(MODULE_USB_DDK, "Error setting property:%{public}u", item);
            return false;
        }
    }
    for (const auto &item : GetAbs()) {
        if (!DoIoctl(fd_, UI_SET_ABSBIT, item)) {
            EDM_LOGE(MODULE_USB_DDK, "Error setting property:%{public}u", item);
            return false;
        }
    }
    for (const auto &item : GetRelBits()) {
        if (!DoIoctl(fd_, UI_SET_RELBIT, item)) {
            EDM_LOGE(MODULE_USB_DDK, "Error setting rel:%{public}u", item);
            return false;
        }
    }

    return true;
}

bool VirtualDevice::EmitEvent(uint16_t type, uint16_t code, uint32_t value) const
{
    struct input_event event {};
    event.type = type;
    event.code = code;
    event.value = value;
    EDM_LOGW(MODULE_USB_DDK, "type:%{public}d code:%{public}d value:%{public}d",
        event.type, event.code, event.value);
#ifndef __MUSL__
    gettimeofday(&event.time, nullptr);
#endif
    if (write(fd_, &event, sizeof(event)) < static_cast<ssize_t>(sizeof(event))) {
        EDM_LOGE(MODULE_USB_DDK, "Event write failed, fd:%{public}d, errno:%{public}d", fd_, errno);
        return false;
    }
    return true;
}

const std::vector<uint32_t> &VirtualDevice::GetEventTypes() const
{
    return eventTypes_;
}

const std::vector<uint32_t> &VirtualDevice::GetKeys() const
{
    return keys_;
}

const std::vector<uint32_t> &VirtualDevice::GetProperties() const
{
    return properties_;
}

const std::vector<uint32_t> &VirtualDevice::GetAbs() const
{
    return abs_;
}

const std::vector<uint32_t> &VirtualDevice::GetRelBits() const
{
    return relBits_;
}

const std::vector<uint32_t> &VirtualDevice::GetLeds() const
{
    return leds_;
}

const std::vector<uint32_t> &VirtualDevice::GetRepeats() const
{
    return repeats_;
}

const std::vector<uint32_t> &VirtualDevice::GetMiscellaneous() const
{
    return miscellaneous_;
}

const std::vector<uint32_t> &VirtualDevice::GetSwitches() const
{
    return switches_;
}

bool VirtualDevice::CreateKey()
{
    auto fun = [&](int32_t uiSet, const std::vector<uint32_t> &list) -> bool {
        for (const auto &item : list) {
            if (ioctl(fd_, uiSet, item) < 0) {
                EDM_LOGE(
                    MODULE_USB_DDK, "not setting event type: %{public}d, deviceName:%{public}s", item, deviceName_);
                return false;
            }
        }
        return true;
    };
    std::map<int32_t, std::vector<uint32_t>> uinputTypes;
    uinputTypes[UI_SET_EVBIT] = GetEventTypes();
    uinputTypes[UI_SET_KEYBIT] = GetKeys();
    uinputTypes[UI_SET_PROPBIT] = GetProperties();
    uinputTypes[UI_SET_ABSBIT] = GetAbs();
    uinputTypes[UI_SET_RELBIT] = GetRelBits();

    uinputTypes[UI_SET_MSCBIT] = GetMiscellaneous();
    uinputTypes[UI_SET_LEDBIT] = GetLeds();
    uinputTypes[UI_SET_SWBIT] = GetSwitches();
    uinputTypes[UI_SET_FFBIT] = GetRepeats();

    for (const auto &item : uinputTypes) {
        if (!fun(item.first, item.second)) {
            return false;
        }
    }
    return true;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
