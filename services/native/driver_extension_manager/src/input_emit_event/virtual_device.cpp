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

    errno_t ret = strncpy_s(dev_.name, MAX_NAME_LENGTH, deviceName_, sizeof(dev_.name));
    if (ret != EOK) {
        EDM_LOGE(MODULE_USB_DDK, "Failed to copy deviceName");
        return false;
    }
    dev_.id.bustype = busType_;
    dev_.id.vendor = vendorId_;
    dev_.id.product = productId_;
    dev_.id.version = version_;
    if (write(fd_, &dev_, sizeof(dev_)) < 0) {
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
    static const std::vector<uint32_t> evtTypes {};
    return evtTypes;
}

const std::vector<uint32_t> &VirtualDevice::GetKeys() const
{
    static const std::vector<uint32_t> keys {};
    return keys;
}

const std::vector<uint32_t> &VirtualDevice::GetProperties() const
{
    static const std::vector<uint32_t> properties {};
    return properties;
}

const std::vector<uint32_t> &VirtualDevice::GetAbs() const
{
    static const std::vector<uint32_t> abs {};
    return abs;
}

const std::vector<uint32_t> &VirtualDevice::GetRelBits() const
{
    static const std::vector<uint32_t> relBits {};
    return relBits;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
