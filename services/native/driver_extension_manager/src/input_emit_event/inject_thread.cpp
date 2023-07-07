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

#include "inject_thread.h"

#include <sys/prctl.h>

#include "hilog_wrapper.h"
namespace OHOS {
namespace ExternalDeviceManager {
std::mutex InjectThread::mutex_;
bool InjectThread::threadRun_ = true;
std::condition_variable InjectThread::conditionVariable_;
std::vector<InjectInputEvent> InjectThread::injectQueue_;
std::unique_ptr<VirtualTouchPad> InjectThread::touchPad_ = nullptr;

InjectThread::InjectThread(uint32_t maxX, uint32_t maxY, uint32_t maxPressure)
{
    touchPad_ = std::make_unique<VirtualTouchPad>(maxX, maxY, maxPressure);
    if (touchPad_ == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "create touchpad failed");
    }
    touchPad_->SetUp();
    threadRun_ = true;
}

void InjectThread::InjectFunc() const
{
    prctl(PR_SET_NAME, "mmi-inject");
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    while (threadRun_) {
        conditionVariable_.wait(uniqueLock, [this] {
            return (injectQueue_.size() > 0 || !threadRun_);
        });

        while (injectQueue_.size() > 0) {
            touchPad_->EmitEvent(injectQueue_[0].type, injectQueue_[0].code, injectQueue_[0].value);
            injectQueue_.erase(injectQueue_.begin());
        }
    }
}

void InjectThread::WaitFunc(const std::vector<EmitItem> &items) const
{
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        for (auto &ele : items) {
            injectQueue_.push_back({ele.type, ele.code, ele.value});
        }
    }
    conditionVariable_.notify_one();
}

void InjectThread::Stop()
{
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        threadRun_ = false;
    }
    conditionVariable_.notify_all();
}
} // namespace ExternalDeviceManager
} // namespace OHOS