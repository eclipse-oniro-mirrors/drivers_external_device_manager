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
#ifndef OHOS_DRIVER_DEVICE_MANAGER_IMPL_H
#define OHOS_DRIVER_DEVICE_MANAGER_IMPL_H

#include <cinttypes>

#include "hilog_wrapper.h"
#include "idriver_ext_mgr_callback.h"
#include "driver_ext_mgr_callback_stub.h"
#include "driver_ext_mgr_client.h"
#include "event_handler.h"
#include <ani.h>

namespace OHOS {
namespace ExternalDeviceManager {
enum ErrorCode : int32_t {
    PERMISSION_DENIED = 201, // Use this error code when permission is denied.
    PERMISSION_NOT_SYSTEM_APP = 202,
    PARAMETER_ERROR = 401, // Use this error code when the input parameter type or range does not match.
    SERVICE_EXCEPTION = 22900001, // Use this error code when the service is exception.
    SERVICE_EXCEPTION_NEW = 26300001, // Use this error code when the service is exception.
    SERVICE_NOT_ALLOW_ACCESS = 26300002, // Use this error code when the service does not allow access.
    SERVICE_NOT_BOUND = 26300003, // Use this error code when the service has no binding relationship.
};

ani_object BindDriverWithDeviceIdSync([[maybe_unused]] ani_env *env, ani_long deviceId, ani_object onDisconnect);

class AsyncData : public RefBase {
    public:
        uint64_t deviceId;
        ani_vm *vm = nullptr;
        ani_env *env  = nullptr;
        ani_ref onDisconnect;
        ani_resolver bindDeferred;
        ErrMsg unBindErrMsg;

        AsyncData(ani_vm *vm, ani_env *env): vm(vm), env(env) {}
        ~AsyncData()
        {
            EDM_LOGD(MODULE_DEV_MGR, "Release callback data: %{public}016" PRIX64, deviceId);
        }
};

class DeviceManagerCallback : public DriverExtMgrCallbackStub {
public:
    ErrCode OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg) override;

    ErrCode OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg) override;

    ErrCode OnUnBind(uint64_t deviceId, const ErrMsg &errMsg) override;
};
}
}
#endif
