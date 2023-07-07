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
#include "usb_ddk_api.h"

using namespace OHOS::ExternalDeviceManager;
namespace {
static DriverExtMgrClient &g_edmClient = DriverExtMgrClient::GetInstance();
}
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
int32_t OH_Usb_CreateDevice(uint32_t maxX, uint32_t maxY, uint32_t maxPressure)
{
    if (auto ret = g_edmClient.CreateDevice(maxX, maxY, maxPressure); ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_USB_DDK, "create device failed:%{public}d", ret);
        return ret;
    }
    return EDM_OK;
}

int32_t OH_Usb_EmitEvent(const EmitItem items[], uint32_t length)
{
    if (length > MAX_EMIT_ITEM_NUM) {
        EDM_LOGE(MODULE_DEV_MGR, "length out of range");
        return EDM_ERR_INVALID_PARAM;
    }

    std::vector<EmitItem> itemsTmp(items, items + length);
    if (auto ret = g_edmClient.EmitEvent(itemsTmp); ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_USB_DDK, "emit event failed:%{public}d", ret);
        return ret;
    }
    return EDM_OK;
}

int32_t OH_Usb_DestroyDevice(int32_t deviceId)
{
    if (auto ret = g_edmClient.DestroyDevice(); ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_USB_DDK, "destroy device failed:%{public}d", ret);
        return ret;
    }
    return EDM_OK;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
