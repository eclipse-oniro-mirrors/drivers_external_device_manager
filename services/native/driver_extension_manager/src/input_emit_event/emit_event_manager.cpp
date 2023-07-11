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

#include "emit_event_manager.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "virtual_keyboard.h"
#include "virtual_touch_pad.h"

static const int MAX_DEVICE_SIZE = 2;

namespace OHOS {
namespace ExternalDeviceManager {
using namespace Security::AccessToken;
IMPLEMENT_SINGLE_INSTANCE(EmitEventManager);
int32_t EmitEventManager::CreateDevice(uint32_t maxX, uint32_t maxY, uint32_t maxPressure)
{
    if (!HasPermission()) {
        EDM_LOGE(MODULE_USB_DDK, "permission denied");
        return EDM_ERR_NO_PERM;
    }

    DestroyDevice();
    vitualDeviceList_.push_back(std::make_unique<VirtualDeviceInject>(std::make_shared<VirtualKeyboard>()));
    vitualDeviceList_.push_back(std::make_unique<VirtualDeviceInject>
        (std::make_shared<VirtualTouchPad>(maxX, maxY, maxPressure)));
    return EDM_OK;
}

int32_t EmitEventManager::EmitEvent(int32_t deviceId, const std::vector<EmitItem> &items)
{
    if (deviceId < 0 || deviceId >= MAX_DEVICE_SIZE) {
        EDM_LOGE(MODULE_USB_DDK, "error deviceId");
    }
    vitualDeviceList_[deviceId]->EmitEvent(items);
    return EDM_OK;
}

int32_t EmitEventManager::DestroyDevice()
{
    vitualDeviceList_.clear();
    return EDM_OK;
}

bool EmitEventManager::CheckHapPermission(uint32_t tokenId)
{
    OHOS::Security::AccessToken::HapTokenInfo findInfo;
    if (OHOS::Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, findInfo) != 0) {
        EDM_LOGE(MODULE_USB_DDK, "GetHapTokenInfo failed");
        return false;
    }
    if ((findInfo.apl == ATokenAplEnum::APL_SYSTEM_CORE) || (findInfo.apl == ATokenAplEnum::APL_SYSTEM_BASIC)) {
        return true;
    }
    return false;
}

bool EmitEventManager::HasPermission(void)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenType = OHOS::Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == OHOS::Security::AccessToken::TOKEN_HAP) {
        return CheckHapPermission(tokenId);
    } else if (tokenType == OHOS::Security::AccessToken::TOKEN_NATIVE) {
        return true;
    } else if (tokenType == OHOS::Security::AccessToken::TOKEN_SHELL) {
        return true;
    }
    return false;
}
} // namespace ExternalDeviceManager
} // namespace OHOS