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

#include "app_launch_notifier.h"

#include <unistd.h>

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "image_source.h"
#include "pixel_map.h"
#include "notification_helper.h"
#include "notification_locale.h"
#include "os_account_manager.h"
#include "want_agent_helper.h"
#include "want_agent_info.h"

namespace OHOS {
namespace ExternalDeviceManager {

namespace {
const int32_t NOTIFICATION_CONTROL_DIALOG_FLAG = 1 << 9;
constexpr const char *APP_LAUNCH_ICON_PATH = "system/etc/peripheral/resources/peripheral_fault_icon.png";
} // namespace

static void SetTitleAndText(std::shared_ptr<Notification::NotificationNormalContent> content,
    const std::string &title, const std::string &text)
{
    if (content == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "Notification normal content nullptr");
        return;
    }
    content->SetTitle(title);
    content->SetText(text);
}

static bool SetBasicOption(Notification::NotificationRequest &request)
{
    int32_t uid = static_cast<int32_t>(getuid());
    request.SetCreatorUid(uid);
    int32_t userId = 0;
    ErrCode getAccountRet = AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    if (getAccountRet != 0) {
        EDM_LOGE(MODULE_BUS_USB, "GetOsAccountLocalIdFromUid failed, uid=%{public}d, ret=%{public}d", uid,
            getAccountRet);
        return false;
    }
    request.SetCreatorPid(getpid());
    request.SetCreatorUserId(userId);
    request.SetInProgress(true);
    request.SetUnremovable(true);
    request.SetTapDismissed(true);
    request.SetSlotType(OHOS::Notification::NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    request.SetNotificationControlFlags(NOTIFICATION_CONTROL_DIALOG_FLAG);
    return true;
}

static bool SetWantAgent(Notification::NotificationRequest &request, const std::string &uri)
{
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetAction("ohos.want.action.appdetail");
    want->SetUri(uri);
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(want);

    std::vector<AbilityRuntime::WantAgent::WantAgentConstant::Flags> flags;
    flags.push_back(AbilityRuntime::WantAgent::WantAgentConstant::Flags::CONSTANT_FLAG);

    AbilityRuntime::WantAgent::WantAgentInfo wantAgentInfo(
        0, AbilityRuntime::WantAgent::WantAgentConstant::OperationType::START_ABILITY, flags, wants, nullptr);
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent =
        AbilityRuntime::WantAgent::WantAgentHelper::GetWantAgent(wantAgentInfo);
    if (wantAgent == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "GetWantAgent failed");
        return false;
    }
    request.SetWantAgent(wantAgent);
    return true;
}

bool AppLaunchNotifier::GetPixelMap(const std::string &path)
{
    if (access(path.c_str(), F_OK) != 0) {
        EDM_LOGE(MODULE_BUS_USB, "Icon file path not exists: %{public}s", path.c_str());
        iconPixelMap_ = nullptr;
        return false;
    }
    uint32_t errorCode = 0;
    Media::SourceOptions opts;
    opts.formatHint = "image/png";
    std::unique_ptr<Media::ImageSource> imageSource = Media::ImageSource::CreateImageSource(path, opts, errorCode);
    if (imageSource == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "ImageSource nullptr, errorCode=%{public}u", errorCode);
        iconPixelMap_ = nullptr;
        return false;
    }
    Media::DecodeOptions decodeOpts;
    std::unique_ptr<Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (pixelMap == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "CreatePixelMap failed, errorCode=%{public}u", errorCode);
        return false;
    }
    iconPixelMap_ = std::shared_ptr<Media::PixelMap>(pixelMap.release());
    return true;
}

int32_t AppLaunchNotifier::SendNotification(const AppLaunchEntry &entry)
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter, bundleName=%{public}s", __func__, entry.bundleName.c_str());

    auto &localeConfig = NotificationLocale::GetInstance();
    localeConfig.ParseLocaleCfg();
    localeConfig.UpdateStringMap();
    std::string title = localeConfig.GetValueByKey("usb_app_launch_title");
    std::string msg = localeConfig.GetValueByKey("usb_app_launch_message");
    if (title.empty() || msg.empty()) {
        EDM_LOGE(MODULE_BUS_USB, "Failed to get localized strings for app launch notification");
        return EDM_NOK;
    }

    std::shared_ptr<Notification::NotificationNormalContent> content =
        std::make_shared<Notification::NotificationNormalContent>();
    if (content == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "Failed to create NotificationNormalContent");
        return EDM_NOK;
    }

    SetTitleAndText(content, title, msg);

    std::shared_ptr<Notification::NotificationContent> notificationContent =
        std::make_shared<Notification::NotificationContent>(content);
    if (notificationContent == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "Failed to create NotificationContent");
        return EDM_NOK;
    }

    int32_t notificationId = static_cast<int32_t>(std::hash<std::string>()(entry.bundleName));
    Notification::NotificationRequest request(notificationId);
    if (GetPixelMap(APP_LAUNCH_ICON_PATH) && iconPixelMap_ != nullptr) {
        request.SetLittleIcon(iconPixelMap_);
        request.SetBadgeIconStyle(Notification::NotificationRequest::BadgeStyle::LITTLE);
    }
    request.SetContent(notificationContent);
    if (!SetBasicOption(request)) {
        return EDM_NOK;
    }
    if (!SetWantAgent(request, entry.appStoreUri)) {
        return EDM_NOK;
    }

    ErrCode publishResult = Notification::NotificationHelper::PublishNotification(request);
    if (publishResult != 0) {
        EDM_LOGE(MODULE_BUS_USB, "PublishNotification failed for %{public}s, ret=%{public}d",
            entry.bundleName.c_str(), publishResult);
        return EDM_NOK;
    }
    EDM_LOGI(MODULE_BUS_USB, "SendNotification success for %{public}s", entry.bundleName.c_str());
    return EDM_OK;
}

} // namespace ExternalDeviceManager
} // namespace OHOS
