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
#include "driver_ext_mgr_client.h"
#include "hilog_wrapper.h"
#include "hid_ddk_api.h"

using namespace OHOS::ExternalDeviceManager;
namespace {
static DriverExtMgrClient &g_edmClient = DriverExtMgrClient::GetInstance();
}
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
int32_t OH_Hid_CreateDevice(Hid_Device *hidDevice, Hid_EventProperties *hidEventProperties)
{
    if (hidDevice == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "hidDevice is null");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "hidEventProperties is null");
        return HID_DDK_INVALID_PARAMETER;
    }

    auto ret = g_edmClient.CreateDevice(hidDevice, hidEventProperties);
    if (ret < 0) {
        EDM_LOGE(MODULE_HID_DDK, "create device failed:%{public}d", ret);
    }
    return ret;
}

int32_t OH_Hid_EmitEvent(int32_t deviceId, const Hid_EmitItem items[], uint16_t length)
{
    if (length > MAX_EMIT_ITEM_NUM) {
        EDM_LOGE(MODULE_HID_DDK, "length out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (items == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "items is null");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<Hid_EmitItem> itemsTmp(items, items + length);
    if (auto ret = g_edmClient.EmitEvent(deviceId, itemsTmp); ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_USB_DDK, "emit event failed:%{public}d", ret);
        return ret;
    }
    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_DestroyDevice(int32_t deviceId)
{
    if (auto ret = g_edmClient.DestroyDevice(deviceId); ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_USB_DDK, "destroy device failed:%{public}d", ret);
        return ret;
    }
    return HID_DDK_SUCCESS;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
