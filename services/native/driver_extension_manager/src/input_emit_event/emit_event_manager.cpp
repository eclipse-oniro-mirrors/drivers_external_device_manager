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

namespace OHOS {
namespace ExternalDeviceManager {
using namespace Security::AccessToken;
IMPLEMENT_SINGLE_INSTANCE(EmitEventManager);
const uint16_t MAX_VIRTUAL_DEVICE_NUM = 200;
int32_t EmitEventManager::CreateDevice(Hid_Device *hidDevice, Hid_EventProperties *hidEventProperties)
{
    if (!HasPermission()) {
        EDM_LOGE(MODULE_HID_DDK, "permission denied");
        return HID_DDK_NO_PERM;
    }
    // check device number
    if (virtualDeviceMap_.size() >= MAX_VIRTUAL_DEVICE_NUM) {
        EDM_LOGE(MODULE_HID_DDK, "device num exceeds maximum %{public}d", MAX_VIRTUAL_DEVICE_NUM);
        return HID_DDK_FAILURE;
    }
    // get device id
    int32_t id = GetCurDeviceId();
    if (id < 0) {
        EDM_LOGE(MODULE_HID_DDK, "faild to generate device id");
        return HID_DDK_FAILURE;
    }
    // create device
    virtualDeviceMap_[id] =
        std::make_unique<VirtualDeviceInject>(std::make_shared<VirtualDevice>(hidDevice, hidEventProperties));
    return id;
}

int32_t EmitEventManager::EmitEvent(int32_t deviceId, const std::vector<Hid_EmitItem> &items)
{
    if (deviceId < 0) {
        EDM_LOGE(MODULE_HID_DDK, "error deviceId");
        return HID_DDK_FAILURE;
    }

    if (virtualDeviceMap_.count(deviceId) == 0) {
        EDM_LOGE(MODULE_HID_DDK, "device is not exit");
        return HID_DDK_FAILURE;
    }

    virtualDeviceMap_[deviceId]->EmitEvent(items);
    return HID_DDK_SUCCESS;
}

int32_t EmitEventManager::DestroyDevice(int32_t deviceId)
{
    if (deviceId < 0) {
        EDM_LOGE(MODULE_HID_DDK, "error deviceId");
        return HID_DDK_FAILURE;
    }

    if (virtualDeviceMap_.count(deviceId) == 0) {
        EDM_LOGE(MODULE_HID_DDK, "device is not exit");
        return HID_DDK_FAILURE;
    }
    virtualDeviceMap_.erase(deviceId);
    lastDeviceId_ = deviceId;
    return HID_DDK_SUCCESS;
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

int32_t EmitEventManager::GetCurDeviceId(void)
{
    if (virtualDeviceMap_.count(lastDeviceId_) == 0) {
        return lastDeviceId_;
    }
    int32_t id = virtualDeviceMap_.size();
    while (virtualDeviceMap_.count(id) != 0 && virtualDeviceMap_.size() < MAX_VIRTUAL_DEVICE_NUM) {
        id++;
    }
    return virtualDeviceMap_.size() < MAX_VIRTUAL_DEVICE_NUM ? id : -1;
}
} // namespace ExternalDeviceManager
} // namespace OHOS