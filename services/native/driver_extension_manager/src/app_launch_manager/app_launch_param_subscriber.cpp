/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "app_launch_param_subscriber.h"

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_support.h>

#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {

AppLaunchParamSubscriber::AppLaunchParamSubscriber()
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
}

AppLaunchParamSubscriber::~AppLaunchParamSubscriber()
{
    UnsubscribeEvent();
}

void AppLaunchParamSubscriber::SubscribeEvent(std::function<void()> updateCallback)
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (subscriber_) {
            EDM_LOGI(MODULE_BUS_USB, "Common Event is already subscribed!");
            return;
        }

        updateCallback_ = updateCallback;

        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EVENT_ACTION);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission("ohos.permission.RECEIVE_UPDATE_MESSAGE");
        subscriber_ = std::make_shared<ParamEventSubscriber>(subscribeInfo, *this);

        for (int32_t retry = 0; retry < RETRY_SUBSCRIBER; retry++) {
            bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
            if (subscribeResult) {
                EDM_LOGI(MODULE_BUS_USB, "SubscribeEvent success.");
                return;
            }
            EDM_LOGI(MODULE_BUS_USB, "SubscribeEvent failed, retry=%{public}d/%{public}d",
                retry + 1, RETRY_SUBSCRIBER);
            if (retry < RETRY_SUBSCRIBER - 1) {
                usleep(RETRY_INTERVAL_US);
            }
        }

        EDM_LOGE(MODULE_BUS_USB, "SubscribeEvent failed after all retries.");
        subscriber_ = nullptr;
    }
}

void AppLaunchParamSubscriber::UnsubscribeEvent()
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (subscriber_) {
            bool subscribeResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
            EDM_LOGI(MODULE_BUS_USB, "subscribeResult = %{public}d", subscribeResult);
            subscriber_ = nullptr;
        }
        updateCallback_ = nullptr;
    }
}

void AppLaunchParamSubscriber::OnReceiveEvent(const AAFwk::Want &want)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    std::string action = want.GetAction();
    if (action != EVENT_ACTION) {
        EDM_LOGI(MODULE_BUS_USB, "Ignore event: %{public}s", action.c_str());
        return;
    }
    HandleParamUpdate(want);
}

void AppLaunchParamSubscriber::HandleParamUpdate(const AAFwk::Want &want)
{
    std::string type = want.GetStringParam(EVENT_INFO_TYPE);
    std::string subtype = want.GetStringParam(EVENT_INFO_SUBTYPE);
    EDM_LOGI(MODULE_BUS_USB, "receive param update event: %{public}s, %{public}s",
        type.c_str(), subtype.c_str());
    if (type != EVENT_INFO_TYPE_VALUE || subtype != EVENT_INFO_SUBTYPE_VALUE) {
        EDM_LOGW(MODULE_BUS_USB, "Invalid type or subtype!");
        return;
    }
    if (updateCallback_) {
        updateCallback_();
    }
}

} // namespace ExternalDeviceManager
} // namespace OHOS
