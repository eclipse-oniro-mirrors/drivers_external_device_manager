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

#ifndef INJECT_THREAD_H
#define INJECT_THREAD_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "emit_event_types.h"
#include "virtual_touch_pad.h"

namespace OHOS {
namespace ExternalDeviceManager {
struct InjectInputEvent {
    uint16_t type {0};
    uint16_t code {0};
    uint32_t value {0};
};

class InjectThread {
public:
    InjectThread(uint32_t maxX, uint32_t maxY, uint32_t maxPressure);
    virtual ~InjectThread() = default;
    void InjectFunc() const;
    void WaitFunc(const std::vector<EmitItem> &items) const;
    void Stop();

private:
    static std::mutex mutex_;
    static bool threadRun_;
    static std::condition_variable conditionVariable_;
    static std::vector<InjectInputEvent> injectQueue_;
    static std::unique_ptr<VirtualTouchPad> touchPad_;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // INJECT_THREAD_H