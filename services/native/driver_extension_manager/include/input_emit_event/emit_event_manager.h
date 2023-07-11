/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef INPUT_EMIT_EVENT_EMIT_EVENT_MANAGER_H
#define INPUT_EMIT_EVENT_EMIT_EVENT_MANAGER_H
#include <inttypes.h>

#include "emit_event_types.h"
#include "single_instance.h"
#include "virtual_device_inject.h"

namespace OHOS {
namespace ExternalDeviceManager {
class EmitEventManager final {
    DECLARE_SINGLE_INSTANCE_BASE(EmitEventManager);

public:
    int32_t CreateDevice(uint32_t maxX, uint32_t maxY, uint32_t maxPressure);
    int32_t EmitEvent(int32_t deviceId, const std::vector<EmitItem> &items);
    int32_t DestroyDevice();

private:
    EmitEventManager() = default;
    bool HasPermission(void);
    bool CheckHapPermission(uint32_t tokenId);
    std::vector<std::unique_ptr<VirtualDeviceInject>> vitualDeviceList_;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // INPUT_EMIT_EVENT_EMIT_EVENT_MANAGER_H