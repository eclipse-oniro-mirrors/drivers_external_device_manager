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

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace ExternalDeviceManager {
std::map<uint32_t, std::shared_ptr<struct ExtDevEvent>> g_matchMap_;
std::map<uint32_t, std::shared_ptr<struct ExtDevEvent>> g_deviceMap_;
std::map<std::string, std::shared_ptr<struct ExtDevEvent>> g_driverMap_;
std::mutex hisyseventMutex_;
constexpr int LAST_FIVE = 5;
    void ExtDevReportSysEvent::ReportDriverPackageCycleManageSysEvent(const PkgInfoTable &pkgInfoTable,
        std::string pids, std::string vids, uint32_t versionCode, std::string driverEventName)
    {
        EDM_LOGI(MODULE_PKG_MGR, "report driver package cycle sys event");
        int32_t hiRet = HiSysEventWrite(HiSysEvent::Domain::EXTERNAL_DEVICE, "DRIVER_PACKAGE_CYCLE_MANAGER",
            HiSysEvent::EventType::STATISTIC, "BUNDLE_NAME", pkgInfoTable.bundleName, "USER_ID", pkgInfoTable.userId,
            "DRIVER_UID", pkgInfoTable.driverUid, "VERSION_CODE", versionCode,
            "VENDOR_ID", vids, "PRODUCT_ID", pids, "DRIVER_EVENT_NAME", driverEventName);
        if (hiRet != EDM_OK) {
            EDM_LOGI(MODULE_PKG_MGR, "HiSysEventWrite ret: %{public}d", hiRet);
        }
    }

    void ExtDevReportSysEvent::ReportExternalDeviceEvent(const std::shared_ptr<ExtDevEvent>& extDevEvent)
    {
        EDM_LOGI(MODULE_PKG_MGR, "report external device event");
        if (extDevEvent == nullptr) {
            EDM_LOGI(MODULE_PKG_MGR, "extDevEvent is null");
            return;
        }
        std::string snNum = "";
        if (extDevEvent->snNum.length() > LAST_FIVE) {
            snNum = extDevEvent->snNum.substr(extDevEvent->snNum.length() - LAST_FIVE);
        }

        int32_t hiRet = HiSysEventWrite(HiSysEvent::Domain::EXTERNAL_DEVICE, "EXTERNAL_DEVICE_EVENT",
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

    void ExtDevReportSysEvent::ReportExternalDeviceSaEvent(const PkgInfoTable &pkgInfoTable, std::string pids,
        std::string vids, uint32_t versionCode, std::string driverEventName)
    {
        EDM_LOGI(MODULE_PKG_MGR, "report external device sa event");
        int32_t hiRet = HiSysEventWrite(HiSysEvent::Domain::EXTERNAL_DEVICE, "DRIVER_PACKAGE_CYCLE_MANAGER",
            HiSysEvent::EventType::STATISTIC, "BUNDLE_NAME", pkgInfoTable.bundleName, "USER_ID", pkgInfoTable.userId,
            "DRIVER_UID", pkgInfoTable.driverUid, "VERSION_CODE", versionCode,
            "VENDOR_ID", vids, "PRODUCT_ID", pids, "DRIVER_EVENT_NAME", driverEventName);
        if (hiRet != EDM_OK) {
            EDM_LOGI(MODULE_PKG_MGR, "HiSysEventWrite ret: %{public}d", hiRet);
        }
    }


    std::shared_ptr<ExtDevEvent> ExtDevReportSysEvent::ExtDevEventInit(const std::shared_ptr<DeviceInfo> &deviceInfo,
        const std::shared_ptr<DriverInfo> &driverInfo, std::shared_ptr<ExtDevEvent> eventObj)
    {
        if (deviceInfo != nullptr) {
            auto busType = deviceInfo->GetBusType();
            switch (busType) {
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

        if (driverInfo != nullptr) {
            auto busType = driverInfo->GetBusType();
            switch (busType) {
                case BusType::BUS_TYPE_USB:{
                    std::shared_ptr<UsbDriverInfo> usbDriverInfo =
                        std::static_pointer_cast<UsbDriverInfo>(driverInfo->GetInfoExt());
                    std::vector<uint16_t> productIds = usbDriverInfo->GetProductIds();
                    std::vector<uint16_t> vendorIds = usbDriverInfo->GetVendorIds();
                    eventObj->vids = ParseIdVector(vendorIds);
                    eventObj->pids = ParseIdVector(productIds);
                    eventObj->driverUid = driverInfo->GetDriverUid();
                    eventObj->driverName = driverInfo->GetDriverName();
                    eventObj->versionCode = driverInfo->GetVersion();
                    eventObj->bundleName = driverInfo->GetBundleName();
                    break;
                }
                default:
                    break;
            }
        }
        return eventObj;
    }

    bool ExtDevReportSysEvent::IsMatched(const std::shared_ptr<DeviceInfo> &deviceInfo,
        const std::shared_ptr<DriverInfo> &driverInfo)
    {
        if (deviceInfo != nullptr && driverInfo != nullptr) {
            std::lock_guard<std::mutex> lock(hisyseventMutex_);
            std::shared_ptr<ExtDevEvent> matchPtr = std::make_shared<ExtDevEvent>();
            auto device = g_deviceMap_.find(deviceInfo->GetDeviceId());
            auto driver = g_driverMap_.find(driverInfo->GetDriverUid());
            if (device != g_deviceMap_.end() && driver != g_driverMap_.end()) {
                matchPtr = ExtDevReportSysEvent::ExtDevEventInit(deviceInfo, driverInfo, matchPtr);
                g_matchMap_[deviceInfo->GetDeviceId()] = matchPtr;
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<ExtDevEvent> ExtDevReportSysEvent::DeviceEventReport(const uint32_t deviceId)
    {
        std::lock_guard<std::mutex> lock(hisyseventMutex_);
        std::shared_ptr<ExtDevEvent> matchPtr = std::make_shared<ExtDevEvent>();
        auto device = g_deviceMap_.find(deviceId);
        if (device != g_deviceMap_.end()) {
            matchPtr = device->second;
            return matchPtr;
        }
        return nullptr;
    }

    std::shared_ptr<ExtDevEvent> ExtDevReportSysEvent::DriverEventReport(const std::string driverUid)
    {
        std::lock_guard<std::mutex> lock(hisyseventMutex_);
        std::shared_ptr<ExtDevEvent> matchPtr = std::make_shared<ExtDevEvent>();
        auto driver = g_driverMap_.find(driverUid);
        if (driver != g_driverMap_.end()) {
            matchPtr = driver->second;
            return matchPtr;
        }
        return nullptr;
    }

    std::shared_ptr<ExtDevEvent> ExtDevReportSysEvent::MatchEventReport(const uint32_t deviceId)
    {
        std::lock_guard<std::mutex> lock(hisyseventMutex_);
        std::shared_ptr<ExtDevEvent> matchPtr = std::make_shared<ExtDevEvent>();
        auto match = g_matchMap_.find(deviceId);
        if (match != g_matchMap_.end()) {
            matchPtr = match->second;
            return matchPtr;
        }
        return nullptr;
    }

    void ExtDevReportSysEvent::SetEventValue(const std::string interfaceName, const int32_t operatType,
        const int32_t errCode, std::shared_ptr<ExtDevEvent> eventPtr)
    {
        if (eventPtr != nullptr) {
        eventPtr->interfaceName = interfaceName;
        eventPtr->operatType = operatType;
        eventPtr->errCode = errCode;
        ReportExternalDeviceEvent(eventPtr);
        }
    }

    void ExtDevReportSysEvent::DriverMapInsert(const std::string driverUid, std::shared_ptr<ExtDevEvent> eventPtr)
    {
        if (eventPtr != nullptr) {
            std::lock_guard<std::mutex> lock(hisyseventMutex_);
            g_driverMap_[driverUid] = eventPtr;
        }
    }

    void ExtDevReportSysEvent::DeviceMapInsert(const uint32_t deviceId, std::shared_ptr<ExtDevEvent> eventPtr)
    {
        if (eventPtr != nullptr) {
            std::lock_guard<std::mutex> lock(hisyseventMutex_);
            g_deviceMap_[deviceId] = eventPtr;
        }
    }

    void ExtDevReportSysEvent::DriverMapErase(const std::string driverUid)
    {
            std::lock_guard<std::mutex> lock(hisyseventMutex_);
            g_driverMap_.erase(driverUid);
    }

    void ExtDevReportSysEvent::DeviceMapErase(const uint32_t deviceId)
    {
            std::lock_guard<std::mutex> lock(hisyseventMutex_);
            g_deviceMap_.erase(deviceId);
    }

    void ExtDevReportSysEvent::MatchMapErase(const uint32_t deviceId)
    {
            std::lock_guard<std::mutex> lock(hisyseventMutex_);
            g_matchMap_.erase(deviceId);
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