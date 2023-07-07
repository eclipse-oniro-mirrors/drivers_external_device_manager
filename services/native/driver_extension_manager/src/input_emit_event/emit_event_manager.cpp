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

    injectThd_ = std::make_shared<InjectThread>(maxX, maxY, maxPressure);
    if (injectThd_ == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "crearte InjectThread failed");
        return EDM_NOK;
    }

    thread_ = std::thread(&InjectThread::InjectFunc, injectThd_);
    return EDM_OK;
}

int32_t EmitEventManager::EmitEvent(const std::vector<EmitItem> &items)
{
    if (injectThd_ == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "please CreateDevice first");
        return EDM_ERR_INVALID_OBJECT;
    }

    injectThd_->WaitFunc(items);
    return EDM_OK;
}

int32_t EmitEventManager::DestroyDevice()
{
    if (injectThd_ == nullptr) {
        return EDM_OK;
    }

    if (thread_.joinable()) {
        injectThd_->Stop();
        thread_.join();
        injectThd_ = nullptr;
    }
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