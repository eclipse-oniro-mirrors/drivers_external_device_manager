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

#ifndef NOTIFICATION_PERIPHERAL_H
#define NOTIFICATION_PERIPHERAL_H

#define API __attribute__((visibility("default")))

#include <mutex>
#include <string>

#include "event_config.h"
#include "image_source.h"
#include "pixel_map.h"

namespace OHOS {
namespace ExternalDeviceManager {

class DeviceNotification {
public:
    static DeviceNotification &GetInstance(void);

    bool HandleNotification(const FaultInfo &notifCfg);

private:
    DeviceNotification() = default;
    virtual ~DeviceNotification() = default;

    bool PeripheralDeviceNotification(const FaultInfo &notifCfg);
    FaultInfo FillNotificationCfg(const FaultInfo &notifCfg);
    bool GetPixelMap(const std::string &path);

    std::shared_ptr<Media::PixelMap> iconPixelMap_ {};
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DEVICE_NOTIFICATION_H
