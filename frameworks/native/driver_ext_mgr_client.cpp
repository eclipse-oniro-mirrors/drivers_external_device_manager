/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "hilog_wrapper.h"

#ifndef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
static constexpr int32_t SERVICE_EXCEPTION = 22900001;
#endif

namespace OHOS {
namespace ExternalDeviceManager {
DriverExtMgrClient::DriverExtMgrClient() {}

DriverExtMgrClient::~DriverExtMgrClient()
{
    if (proxy_ != nullptr) {
        auto remote = proxy_->AsObject();
        if (remote != nullptr) {
            remote->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
static UsbErrCode ProxyRetTranslate(int32_t proxyRet)
{
    UsbErrCode ret = UsbErrCode::EDM_OK;
    switch (proxyRet) {
        case ERR_INVALID_VALUE:
            ret = UsbErrCode::EDM_ERR_INVALID_OBJECT;
            break;
        case ERR_INVALID_DATA:
            ret = UsbErrCode::EDM_ERR_INVALID_PARAM;
            break;
        default:
            ret = static_cast<UsbErrCode>(proxyRet);
            break;
    }
    return ret;
}
#endif

UsbErrCode DriverExtMgrClient::Connect(void)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return UsbErrCode::EDM_OK;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "Failed to obtain SystemAbilityMgr");
        return UsbErrCode::EDM_ERR_GET_SYSTEM_ABILITY_MANAGER_FAILED;
    }

    sptr<IRemoteObject> remote = sam->CheckSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (remote == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "Check SystemAbility failed");
        return UsbErrCode::EDM_ERR_GET_SERVICE_FAILED;
    }

    deathRecipient_ = new DriverExtMgrDeathRecipient();
    if (deathRecipient_ == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "Failed to create DriverExtMgrDeathRecipient");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        EDM_LOGF(MODULE_FRAMEWORK, "Failed to add death recipient to DriverExtMgrProxy service");
        return EDM_ERR_NOT_SUPPORT;
    }

    proxy_ = iface_cast<IDriverExtMgr>(remote);
    if (proxy_ == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "Failed to cast DriverExtMgrProxy object");
        return UsbErrCode::EDM_ERR_GET_SERVICE_FAILED;
    }
    EDM_LOGI(MODULE_FRAMEWORK, "Connecting DriverExtMgrProxy success");
    return UsbErrCode::EDM_OK;
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

void DriverExtMgrClient::DisConnect(const wptr<IRemoteObject> &remote)
{
#ifndef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    (void)remote;
#else
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
#endif
}

void DriverExtMgrClient::DriverExtMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
#ifndef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    (void)remote;
#else
    if (remote == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "OnRemoteDied failed, remote is nullptr");
        return;
    }

    DriverExtMgrClient::GetInstance().DisConnect(remote);
    EDM_LOGF(MODULE_FRAMEWORK, "received death notification of remote and finished to disconnect");
#endif
}

UsbErrCode DriverExtMgrClient::QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->QueryDevice(ret, busType, devices);
    if (proxyRet != ERR_OK) {
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

UsbErrCode DriverExtMgrClient::BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->BindDevice(ret, deviceId, connectCallback);
    if (proxyRet != ERR_OK) {
        EDM_LOGI(MODULE_FRAMEWORK, "proxyRet = %{public}d", proxyRet);
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

UsbErrCode DriverExtMgrClient::UnBindDevice(uint64_t deviceId)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->UnBindDevice(ret, deviceId);
    if (proxyRet != ERR_OK) {
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

UsbErrCode DriverExtMgrClient::BindDriverWithDeviceId(uint64_t deviceId,
    const sptr<IDriverExtMgrCallback> &connectCallback)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->BindDriverWithDeviceId(ret, deviceId, connectCallback);
    if (proxyRet != ERR_OK) {
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

UsbErrCode DriverExtMgrClient::UnbindDriverWithDeviceId(uint64_t deviceId)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->UnBindDriverWithDeviceId(ret, deviceId);
    if (proxyRet != ERR_OK) {
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

UsbErrCode DriverExtMgrClient::QueryDeviceInfo(std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->QueryDeviceInfo(ret, deviceInfos, false, 0);
    if (proxyRet != ERR_OK) {
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

UsbErrCode DriverExtMgrClient::QueryDeviceInfo(const uint64_t deviceId,
    std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->QueryDeviceInfo(ret, deviceInfos, true, deviceId);
    if (proxyRet != ERR_OK) {
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

UsbErrCode DriverExtMgrClient::QueryDriverInfo(std::vector<std::shared_ptr<DriverInfoData>> &driverInfos)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->QueryDriverInfo(ret, driverInfos, false, "");
    if (proxyRet != ERR_OK) {
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

UsbErrCode DriverExtMgrClient::QueryDriverInfo(const std::string &driverUid,
    std::vector<std::shared_ptr<DriverInfoData>> &driverInfos)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->QueryDriverInfo(ret, driverInfos, true, driverUid);
    if (proxyRet != ERR_OK) {
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

UsbErrCode DriverExtMgrClient::NotifyUsbPeripheralFault(const std::string &domain, const std::string &faultName)
{
#ifdef ENABLE_EXTERNAL_DEVICE_DDK_SERVICE
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }

    int32_t ret = EDM_OK;
    int32_t proxyRet = proxy_->NotifyUsbPeripheralFault(domain, faultName);
    if (proxyRet != ERR_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "Usb peripheral fault notify failed.");
        return ProxyRetTranslate(proxyRet);
    }
    return static_cast<UsbErrCode>(ret);
#else
    return static_cast<UsbErrCode>(SERVICE_EXCEPTION);
#endif
}

} // namespace ExternalDeviceManager
} // namespace OHOS
