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
static std::map<uint32_t id, sptr<ExtDevEvent> event> g_matchMap_;
static std::map<uint32_t id, sptr<ExtDevEvent> event> g_deviceMap_;
static std::map<std::string id, sptr<ExtDevEvent> event> g_driverMap_;
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
        if (extDevEvent -- nullptr) {
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
            "OPERAT_TYPE", extDevEvent->operatType, "INTERFACE_NAME", extDevEvent->interfaceName, "FAIL_MESSAGE",
            extDevEvent->failMessage, "ERR_CODE", extDevEvent->errCode);
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


    shared_ptr<ExtDevEvent> ExtDevReportSysEvent::ExtDevEventInit(const std::shared_ptr<DeviceInfo> &deviceInfo,
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
                    eventObj->vids = usbDriverInfo->GetVendorIds();
                    eventObj->pids = usbDriverInfo->GetProductIds();
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
        std::lock_guard<std::mutex> lock(hisyseventMutex_);
        std::shared_ptr<ExtDevEvent> matchPtr = std::make_shared<ExtDevEvent>();
        auto device = g_deviceMap_.find(deviceInfo->GetDeviceId());
        auto driver = g_driverMap_.find(driverInfo->GetDriverUid());
        if (device != g_deviceMap_.end() && driver != g_driverMap_.end()) {
            matchPtr = ExtDevReportSysEvent::ExtDevEventInit(deviceInfo, driverInfo, matchPtr);
            g_matchMap_.insert(deviceInfo->GetDeviceId(), matchPtr);
            return true;
        }
        return false;
    }

    shared_ptr<ExtDevEvent> ExtDevReportSysEvent::DeviceEventReport(const uint32_t deviceId)
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

    shared_ptr<ExtDevEvent> ExtDevReportSysEvent::DriverEventReport(const std::string driverUid)
    {
        std::lock_guard<std::mutex> lock(hisyseventMutex_);
        std::shared_ptr<ExtDevEvent> matchPtr = std::make_shared<ExtDevEvent>();
        auto driver = g_driverMap_.find(driverUid);
        if (driver != driverUidMap_.end()) {
            matchPtr = driver->second;
            return matchPtr;
        }
        return nullptr;
    }

    shared_ptr<ExtDevEvent> ExtDevReportSysEvent::MatchEventReport(const uint32_t deviceId)
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
            eventPtr->interfaceName = interfaceName;
            eventPtr->operatType = operatType;
            eventPtr->errCode = errCode;
            ReportExternalDeviceEvent(eventPtr);
        }
}
}