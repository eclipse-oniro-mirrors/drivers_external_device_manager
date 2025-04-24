/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "driver_report_sys_event.h"
#include "hilog_wrapper.h"
#include "hisysevent.h"
#include "edm_errors.h"
#include "ext_object.h"
#include "usb_device_info.h"
#include "usb_driver_info.h"
#include "pkg_tables.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace ExternalDeviceManager {
constexpr int32_t LAST_FIVE = 5;

const std::map<ExtDevReportSysEvent::EventErrCode, std::string> ExtDevReportSysEvent::ErrMsgs = {
    {ExtDevReportSysEvent::EventErrCode::SUCCESS, "Success"},
    {ExtDevReportSysEvent::EventErrCode::BIND_JS_CALLBACK_FAILED, "Failed to register JS callback"},
    {ExtDevReportSysEvent::EventErrCode::CONNECT_DRIVER_EXTENSION_FAILED,
        "Failed to connect DriverExtensionAbility"},
    {ExtDevReportSysEvent::EventErrCode::BIND_ACCESS_NOT_ALLOWED, "Bind access is not allowed"},
    {ExtDevReportSysEvent::EventErrCode::UNBIND_DRIVER_EMPTY, "No driver matched for the device"},
    {ExtDevReportSysEvent::EventErrCode::UNBIND_RELATION_NOT_FOUND,
        "Binding relationship between client and driver not found"},
    {ExtDevReportSysEvent::EventErrCode::DISCONNECT_DRIVER_EXTENSION_FAILED,
        "Failed to disconnect DriverExtensionAbility"},
    {ExtDevReportSysEvent::EventErrCode::QUERY_DRIVER_EXTENSION_FAILED,
        "Failed to query DriverExtensionAbility"},
    {ExtDevReportSysEvent::EventErrCode::UPDATE_DATABASE_FAILED, "Failed to update database"},
    {ExtDevReportSysEvent::EventErrCode::LIFECYCLE_FUNCTION_FAILED,
        "Lifecycle function execution failed"},
    {ExtDevReportSysEvent::EventErrCode::OPEN_DEVICE_FAILED, "Failed to open device"},
    {ExtDevReportSysEvent::EventErrCode::GET_DEVICE_DESCRIPTOR_FAILED,
        "Failed to get device descriptor"},
    {ExtDevReportSysEvent::EventErrCode::DEVICE_DESCRIPTOR_LENGTH_INVALID,
        "Device descriptor length is invalid"},
    {ExtDevReportSysEvent::EventErrCode::GET_INTERFACE_DESCRIPTOR_FAILED,
        "Failed to get interface descriptor"},
    {ExtDevReportSysEvent::EventErrCode::STOP_DRIVER_EXTENSION_FAILED,
        "Failed to stop DriverExtensionAbility"},
    {ExtDevReportSysEvent::EventErrCode::QUERY_DRIVER_INFO_FAILED,
        "Failed to query driver information"},
    {ExtDevReportSysEvent::EventErrCode::NO_MATCHING_DRIVER_FOUND,
        "No matching driver found for the device"}
};

void ExtDevReportSysEvent::ReportExternalDeviceEvent(const std::shared_ptr<ExtDevEvent> &extDevEvent)
{
    EDM_LOGI(MODULE_PKG_MGR, "report external device event");
    if (extDevEvent == nullptr) {
        EDM_LOGI(MODULE_PKG_MGR, "%{public}s, extDevEvent is null", __func__);
        return;
    }
    std::string snNum = "";
    if (extDevEvent->snNum.length() > LAST_FIVE) {
        snNum = extDevEvent->snNum.substr(extDevEvent->snNum.length() - LAST_FIVE);
    }

    int32_t hiRet = HiSysEventWrite(HiSysEvent::Domain::EXTERNAL_DEVICE, "EXT_DEVICE_EVENT",
        HiSysEvent::EventType::STATISTIC, "DEVICE_CLASS", extDevEvent->deviceClass, "DEVICE_SUBCLASS",
        extDevEvent->deviceSubClass, "DEVICE_PROTOCOL", extDevEvent->deviceProtocol, "SN_NUM", snNum,
        "VENDOR_ID", extDevEvent->vendorId, "PRODUCT_ID", extDevEvent->productId,
        "DEVICE_ID", extDevEvent->deviceId, "DRIVER_UID", extDevEvent->driverUid, "DRIVER_NAME",
        extDevEvent->driverName, "VERSION_CODE", extDevEvent->versionCode, "VIDS", extDevEvent->vids,
        "PIDS", extDevEvent->pids, "USER_ID", extDevEvent->userId, "BUNDLE_NAME", extDevEvent->bundleName,
        "OPERAT_TYPE", extDevEvent->operatType, "INTERFACE_NAME", extDevEvent->interfaceName, "MESSAGE",
        extDevEvent->message, "ERR_CODE", extDevEvent->errCode);
    if (hiRet != EDM_OK) {
        EDM_LOGI(MODULE_PKG_MGR, "HiSysEventWrite ret: %{public}d", hiRet);
    }
}

void ExtDevReportSysEvent::ReportExternalDeviceEvent(const std::shared_ptr<ExtDevEvent> &extDevEvent,
    const ExtDevReportSysEvent::EventErrCode errCode)
{
    EDM_LOGI(MODULE_PKG_MGR, "report external device event with error code");
    if (extDevEvent == nullptr) {
        EDM_LOGI(MODULE_PKG_MGR, "%{public}s, extDevEvent is null", __func__);
        return;
    }
    extDevEvent->errCode = static_cast<int32_t>(errCode);
    auto it = ExtDevReportSysEvent::ErrMsgs.find(errCode);
    if (it != ExtDevReportSysEvent::ErrMsgs.end()) {
        extDevEvent->message = it->second;
    }
    ReportExternalDeviceEvent(extDevEvent);
}

void ExtDevReportSysEvent::ParseToExtDevEvent(const std::shared_ptr<DeviceInfo> &deviceInfo,
    const std::shared_ptr<ExtDevEvent> &eventObj)
{
    if (deviceInfo == nullptr || eventObj == nullptr) {
        return;
    }
    switch (deviceInfo->GetBusType()) {
        case BusType::BUS_TYPE_USB:{
            std::shared_ptr<UsbDeviceInfo> usbDeviceInfo = std::static_pointer_cast<UsbDeviceInfo>(deviceInfo);
            eventObj->deviceId = usbDeviceInfo->GetDeviceId();
            eventObj->deviceClass = usbDeviceInfo->GetDeviceClass();
            eventObj->deviceSubClass = usbDeviceInfo->GetDeviceSubClass();
            eventObj->deviceProtocol = usbDeviceInfo->GetDeviceProtocol();
            eventObj->snNum = usbDeviceInfo->GetSnNum();
            eventObj->vendorId = usbDeviceInfo->GetVendorId();
            eventObj->productId = usbDeviceInfo->GetProductId();
            break;
        }
        default:
            break;
    }
}

void ExtDevReportSysEvent::ParseToExtDevEvent(const std::shared_ptr<DriverInfo> &driverInfo,
    const std::shared_ptr<ExtDevEvent> &eventObj)
{
    if (driverInfo == nullptr || eventObj == nullptr) {
        return;
    }
    switch (driverInfo->GetBusType()) {
        case BusType::BUS_TYPE_USB:{
            std::shared_ptr<UsbDriverInfo> usbDriverInfo =
                std::static_pointer_cast<UsbDriverInfo>(driverInfo->GetInfoExt());
            std::vector<uint16_t> productIds = usbDriverInfo->GetProductIds();
            std::vector<uint16_t> vendorIds = usbDriverInfo->GetVendorIds();
            eventObj->vids = ParseIdVector(vendorIds);
            eventObj->pids = ParseIdVector(productIds);
            eventObj->driverUid = driverInfo->GetDriverUid();
            eventObj->userId = driverInfo->GetUserId();
            eventObj->driverName = driverInfo->GetDriverName();
            eventObj->versionCode = driverInfo->GetVersion();
            eventObj->bundleName = driverInfo->GetBundleName();
            break;
        }
        default:
            break;
    }
}

void ExtDevReportSysEvent::ParseToExtDevEvent(const std::shared_ptr<DeviceInfo> &deviceInfo,
    const std::shared_ptr<DriverInfo> &driverInfo, const std::shared_ptr<ExtDevEvent> &eventObj)
{
    ExtDevReportSysEvent::ParseToExtDevEvent(deviceInfo, eventObj);
    ExtDevReportSysEvent::ParseToExtDevEvent(driverInfo, eventObj);
}

std::string ExtDevReportSysEvent::ParseIdVector(std::vector<uint16_t> ids)
{
    if (ids.size() < 1) {
        return "";
    }
    std::string str = "";
    auto it = ids.begin();
    for (uint16_t id : ids) {
        if (it + 1 == ids.end()) {
            std::string copy = std::to_string(id);
            str.append(copy);
        } else {
            std::string copy = std::to_string(id);
            str.append(copy);
            str.append(",");
        }
    }
    return str;
}
}
}