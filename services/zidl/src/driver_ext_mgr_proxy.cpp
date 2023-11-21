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

#include "driver_ext_mgr_proxy.h"
#include <cinttypes>

#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "securec.h"

namespace OHOS {
namespace ExternalDeviceManager {
UsbErrCode DriverExtMgrProxy::QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (!data.WriteUint32(busType)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write busType");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::QUERY_DEVICE), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return static_cast<UsbErrCode>(ret);
    }

    uint64_t deviceInfoSize = 0;
    if (!reply.ReadUint64(deviceInfoSize)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read size of DeviceData");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (deviceInfoSize > devices.max_size()) {
        EDM_LOGE(MODULE_FRAMEWORK, "invalid size of DeviceData");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    for (uint64_t i = 0; i < deviceInfoSize; i++) {
        std::shared_ptr<DeviceData> device = DeviceData::UnMarshalling(reply);
        if (device == nullptr) {
            EDM_LOGE(MODULE_FRAMEWORK, "failed to read %{public}016" PRIX64 " device", i);
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }
        devices.push_back(std::move(device));
    }

    return UsbErrCode::EDM_OK;
}

UsbErrCode DriverExtMgrProxy::BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (!data.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write deviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (connectCallback == nullptr || !data.WriteRemoteObject(connectCallback->AsObject())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write connectCallback object");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::BIND_DEVICE), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return static_cast<UsbErrCode>(ret);
    }

    return UsbErrCode::EDM_OK;
}

UsbErrCode DriverExtMgrProxy::UnBindDevice(uint64_t deviceId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (!data.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write deviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::UNBIND_DEVICE), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return static_cast<UsbErrCode>(ret);
    }

    return UsbErrCode::EDM_OK;
}

int32_t DriverExtMgrProxy::CreateDevice(Hid_Device *hidDevice, Hid_EventProperties *hidEventProperties)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return HID_DDK_FAILURE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (!HidDeviceMarshalling(hidDevice, data)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to marshall HidDevice");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (!HidEventPropertiesMarshalling(hidEventProperties, data)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to marshall HidEventProperties");
        return HID_DDK_INVALID_PARAMETER;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::INPUT_CREATE_DEVICE), data, reply, option);
    if (ret < HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
    }
    return ret;
}

int32_t DriverExtMgrProxy::EmitEvent(int32_t deviceId, const std::vector<Hid_EmitItem> &items)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return HID_DDK_FAILURE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (!EmitItemMarshalling(deviceId, items, data)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to marshall EmitItem");
        return HID_DDK_INVALID_PARAMETER;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::INPUT_EMIT_EVENT), data, reply, option);
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return ret;
    }
    return HID_DDK_SUCCESS;
}

int32_t DriverExtMgrProxy::DestroyDevice(int32_t deviceId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return HID_DDK_FAILURE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (!data.WriteUint32(deviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write device id");
        return HID_DDK_INVALID_PARAMETER;
    }

    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DriverExtMgrInterfaceCode::INPUT_DESTROY_DEVICE), data, reply, option);
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return ret;
    }
    return HID_DDK_SUCCESS;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
