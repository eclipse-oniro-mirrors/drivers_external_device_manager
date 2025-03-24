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
    void ExtDevReportSysEvent::ReportDriverPackageCycleMangeSysEvent(const PkgInfoTable &pkgInfoTable, std::string pids,
        std::string vids, uint32_t versionCode, std::string driverEventName)
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
}
}