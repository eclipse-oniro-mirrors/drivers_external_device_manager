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

#ifndef DRIVER_EXTENSION_MANAGER_CLIENT_H
#define DRIVER_EXTENSION_MANAGER_CLIENT_H
#include <singleton.h>

#include "driver_ext_mgr_types.h"
#include "idriver_ext_mgr.h"

namespace OHOS {
namespace ExternalDeviceManager {
class DriverExtMgrClient final : public DelayedRefSingleton<DriverExtMgrClient> {
    DECLARE_DELAYED_REF_SINGLETON(DriverExtMgrClient)

public:
    DISALLOW_COPY_AND_MOVE(DriverExtMgrClient);
    UsbErrCode QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices);
    UsbErrCode BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback);
    UsbErrCode UnBindDevice(uint64_t deviceId);
    int32_t CreateDevice(Hid_Device *hidDevice, Hid_EventProperties *hidEventProperties);
    int32_t EmitEvent(int32_t deviceId, const std::vector<Hid_EmitItem> &items);
    int32_t DestroyDevice(int32_t deviceId);

private:
    UsbErrCode Connect();
    void DisConnect(const wptr<IRemoteObject> &remote);

    class DriverExtMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DriverExtMgrDeathRecipient() = default;
        ~DriverExtMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote);

    private:
        DISALLOW_COPY_AND_MOVE(DriverExtMgrDeathRecipient);
    };

    std::mutex mutex_;
    sptr<IDriverExtMgr> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_EXTENSION_MANAGER_CLIENT_H