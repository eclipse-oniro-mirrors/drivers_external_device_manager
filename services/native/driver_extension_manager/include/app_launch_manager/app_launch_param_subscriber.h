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

#ifndef APP_LAUNCH_PARAM_SUBSCRIBER_H
#define APP_LAUNCH_PARAM_SUBSCRIBER_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include "common_event_subscriber.h"
#include "want.h"

namespace OHOS {
namespace ExternalDeviceManager {

class AppLaunchParamSubscriber {
public:
    AppLaunchParamSubscriber();
    ~AppLaunchParamSubscriber();

    void SubscribeEvent(std::function<void()> updateCallback);
    void UnsubscribeEvent();
    void OnReceiveEvent(const AAFwk::Want &want);

private:
    class ParamEventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        explicit ParamEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscriberInfo,
            AppLaunchParamSubscriber &registry)
            : CommonEventSubscriber(subscriberInfo), registry_(registry)
        {}
        ~ParamEventSubscriber() = default;
        void OnReceiveEvent(const EventFwk::CommonEventData &data) override
        {
            registry_.OnReceiveEvent(data.GetWant());
        }
    private:
        AppLaunchParamSubscriber &registry_;
    };

    void HandleParamUpdate(const AAFwk::Want &want);

    std::shared_ptr<ParamEventSubscriber> subscriber_ = nullptr;
    std::function<void()> updateCallback_ = nullptr;
    std::mutex callbackMutex_;

    static constexpr const char *EVENT_ACTION = "usual.event.DUE_SA_CFG_UPDATED";
    static constexpr const char *EVENT_INFO_TYPE = "type";
    static constexpr const char *EVENT_INFO_TYPE_VALUE = "UsbWirelessDisplayAdapterApp";
    static constexpr const char *EVENT_INFO_SUBTYPE = "subtype";
    static constexpr const char *EVENT_INFO_SUBTYPE_VALUE = "generic";
    static constexpr int32_t RETRY_SUBSCRIBER = 3;
    static constexpr int32_t RETRY_INTERVAL_US = 100000; //100ms
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // APP_LAUNCH_PARAM_SUBSCRIBER_H
