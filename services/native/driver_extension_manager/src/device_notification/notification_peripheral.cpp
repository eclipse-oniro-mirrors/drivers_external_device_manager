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

#define API __attribute__((visibility("default")))

#include "notification_peripheral.h"
#include "edm_errors.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "locale_config.h"
#include "locale_info.h"
#include "notification_helper.h"
#include "notification_locale.h"
#include "os_account_manager.h"
#include "securec.h"
#include "want_agent_helper.h"
#include "want_agent_info.h"
#include <cJSON.h>
#include <map>
#include <unistd.h>

namespace OHOS {
namespace ExternalDeviceManager {
static const std::string ENTITIES = "entity.system.browsable";
static const std::string ACTION = "ohos.want.action.viewData";
const int32_t NOTIFICATION_SERVICE_SYS_ABILITY_ID = 3085;
const int32_t NOTIFICATION_CONTROL_DIALOG_FLAG = 1 << 9;
constexpr const char *PERIPHERAL_ICON_PATH = "system/etc/peripheral/resources/peripheral_fault_icon.png";

DeviceNotification &DeviceNotification::GetInstance()
{
    static DeviceNotification instance;
    return instance;
}

bool DeviceNotification::HandleNotification(const FaultInfo &faultInfo)
{
    NotificationLocale::GetInstance().ParseLocaleCfg();
    NotificationLocale::GetInstance().UpdateStringMap();
    FaultInfo notifCfg = FillNotificationCfg(faultInfo);
    if (notifCfg.title.empty() || notifCfg.msg.empty()) {
        EDM_LOGE(MODULE_SERVICE, "Invalid notification: missing title or message");
        return false;
    }

    if (!PeripheralDeviceNotification(notifCfg)) {
        EDM_LOGE(MODULE_SERVICE, "Failed to send peripheral notification");
        return false;
    }
    return true;
}

static bool SetTitleAndText(std::shared_ptr<Notification::NotificationNormalContent> content,
                            const std::string &title, const std::string &text)
{
    if (content == nullptr) {
        EDM_LOGE(MODULE_SERVICE, "Notification normal content nullptr");
        return false;
    }

    content->SetTitle(title);
    content->SetText(text);
    return true;
}

static void SetBasicOption(Notification::NotificationRequest &request)
{
    request.SetCreatorUid(NOTIFICATION_SERVICE_SYS_ABILITY_ID);
    int32_t userId = 0;
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(NOTIFICATION_SERVICE_SYS_ABILITY_ID, userId);
    request.SetCreatorPid(getpid());
    request.SetCreatorUserId(userId);
    request.SetInProgress(true);
    request.SetUnremovable(true);
    request.SetTapDismissed(true);
    request.SetSlotType(OHOS::Notification::NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    request.SetNotificationControlFlags(NOTIFICATION_CONTROL_DIALOG_FLAG);
}

static void SetWantAgent(OHOS::Notification::NotificationRequest &request, const std::string &uri)
{
    auto want = std::make_shared<AAFwk::Want>();
    want->SetAction(ACTION);
    want->SetUri(uri);
    want->AddEntity(ENTITIES);
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(want);

    std::vector<AbilityRuntime::WantAgent::WantAgentConstant::Flags> flags;
    flags.push_back(AbilityRuntime::WantAgent::WantAgentConstant::Flags::CONSTANT_FLAG);

    AbilityRuntime::WantAgent::WantAgentInfo wantAgentInfo(
        0, AbilityRuntime::WantAgent::WantAgentConstant::OperationType::START_ABILITY, flags, wants, nullptr);
    auto wantAgent = AbilityRuntime::WantAgent::WantAgentHelper::GetWantAgent(wantAgentInfo);
    request.SetWantAgent(wantAgent);
}

bool DeviceNotification::GetPixelMap(const std::string &path)
{
    if (access(path.c_str(), F_OK) != 0) {
        EDM_LOGE(MODULE_DEV_MGR, "Peripheral icon file path not exists.");
        iconPixelMap_ = nullptr;
        return false;
    }
    uint32_t errorCode = 0;
    Media::SourceOptions opts;
    opts.formatHint = "image/png";
    std::unique_ptr<Media::ImageSource> imageSource = Media::ImageSource::CreateImageSource(path, opts, errorCode);
    if (imageSource == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "ImageSource nullptr");
        iconPixelMap_ = nullptr;
        return false;
    }
    Media::DecodeOptions decodeOpts;
    std::unique_ptr<Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    iconPixelMap_ = std::move(pixelMap);
    return true;
}

FaultInfo DeviceNotification::FillNotificationCfg(const FaultInfo &faultInfo)
{
    auto &localeConfig = NotificationLocale::GetInstance();
    FaultInfo temp(faultInfo);

    temp.title = localeConfig.GetValueByKey(faultInfo.title);
    temp.msg = localeConfig.GetValueByKey(faultInfo.msg);
    return temp;
}

bool DeviceNotification::PeripheralDeviceNotification(const FaultInfo &faultInfo)
{
    EDM_LOGD(MODULE_SERVICE, "Start PeripheralDeviceNotification %{public}s", faultInfo.GetInfo().c_str());
    std::shared_ptr<Notification::NotificationNormalContent> content =
        std::make_shared<Notification::NotificationNormalContent>();
    if (content == nullptr) {
        EDM_LOGE(MODULE_SERVICE, "Failed to create NotificationNormalContent");
        return false;
    }

    if (!SetTitleAndText(content, faultInfo.title, faultInfo.msg)) {
        EDM_LOGE(MODULE_SERVICE, "Failed to set title and text");
        return false;
    }

    std::shared_ptr<Notification::NotificationContent> notificationContent =
        std::make_shared<Notification::NotificationContent>(content);
    if (notificationContent == nullptr) {
        EDM_LOGE(MODULE_SERVICE, "Failed to create NotificationContent");
        return false;
    }

    if (!GetPixelMap(PERIPHERAL_ICON_PATH)) {
        EDM_LOGE(MODULE_SERVICE, "Failed to get peripheral icon pixel map");
        return false;
    }

    int32_t notificationId = static_cast<int32_t>(std::hash<std::string>()(faultInfo.title));
    Notification::NotificationRequest request(notificationId);
    if (iconPixelMap_ != nullptr) {
        request.SetLittleIcon(iconPixelMap_);
        request.SetBadgeIconStyle(Notification::NotificationRequest::BadgeStyle::LITTLE);
    }

    request.SetContent(notificationContent);
    SetBasicOption(request);
    SetWantAgent(request, faultInfo.uri);
    Notification::NotificationHelper::PublishNotification(request);
    return true;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
