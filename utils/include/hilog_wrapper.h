/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef HILOG_WRAPPER_H
#define HILOG_WRAPPER_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"
namespace OHOS {
namespace ExternalDeviceManager {
#define FORMATED_EDM(fmt, ...) "[%{public}s]" fmt, __FUNCTION__, ##__VA_ARGS__

#ifdef EDM_LOGF
#undef EDM_LOGF
#endif

#ifdef EDM_LOGE
#undef EDM_LOGE
#endif

#ifdef EDM_LOGW
#undef EDM_LOGW
#endif

#ifdef EDM_LOGI
#undef EDM_LOGI
#endif

#ifdef EDM_LOGD
#undef EDM_LOGD
#endif

// param of log interface, such as EDM_LOGF.
enum UsbMgrSubModule {
    MODULE_SERVICE = 0,
    MODULE_DEV_MGR,
    MODULE_PKG_MGR,
    MODULE_EA_MGR,
    MODULE_BUS_USB,
    MODULE_COMMON,
    EDM_MODULE_BUTT,
};

// 0xD002550: part:ExternalDeviceManager module:Edm.
constexpr unsigned int BASE_EDM_DOMAIN_ID = 0xD002550;

enum UsbMgrDomainId {
    EDM_SERVICE_DOMAIN = BASE_EDM_DOMAIN_ID + MODULE_SERVICE,
    EDM_DEV_MGR_DOMAIN,
    EDM_PKG_MGR_DOMAIN,
    EDM_EA_MGR_DOMAIN,
    EDM_BUS_USB_DOMAIN,
    EDM_COMMON_DOMAIN,
    EDM_BUTT,
};

constexpr OHOS::HiviewDFX::HiLogLabel EDM_MGR_LABEL[EDM_MODULE_BUTT] = {
    {LOG_CORE, EDM_SERVICE_DOMAIN, "EdmService"},
    {LOG_CORE, EDM_DEV_MGR_DOMAIN, "EdmDevMgr"},
    {LOG_CORE, EDM_PKG_MGR_DOMAIN, "EdmPkgMgr"},
    {LOG_CORE, EDM_EA_MGR_DOMAIN, "EdmEaMgr"},
    {LOG_CORE, EDM_BUS_USB_DOMAIN, "EdmBusUsbMgr"},
    {LOG_CORE, EDM_COMMON_DOMAIN, "EdmCommon"},
};

// In order to improve performance, do not check the module range, module should less than EDM_MODULE_BUTT.
#define EDM_LOGF(module, ...) (void)OHOS::HiviewDFX::HiLog::Fatal(EDM_MGR_LABEL[module], FORMATED_EDM(__VA_ARGS__))
#define EDM_LOGE(module, ...) (void)OHOS::HiviewDFX::HiLog::Error(EDM_MGR_LABEL[module], FORMATED_EDM(__VA_ARGS__))
#define EDM_LOGW(module, ...) (void)OHOS::HiviewDFX::HiLog::Warn(EDM_MGR_LABEL[module], FORMATED_EDM(__VA_ARGS__))
#define EDM_LOGI(module, ...) (void)OHOS::HiviewDFX::HiLog::Info(EDM_MGR_LABEL[module], FORMATED_EDM(__VA_ARGS__))
#define EDM_LOGD(module, ...) (void)OHOS::HiviewDFX::HiLog::Debug(EDM_MGR_LABEL[module], FORMATED_EDM(__VA_ARGS__))
} // namespace ExternalDeviceManager
} // namespace OHOS

#else

#define EDM_LOGF(...)
#define EDM_LOGE(...)
#define EDM_LOGW(...)
#define EDM_LOGI(...)
#define EDM_LOGD(...)

#endif // CONFIG_HILOG

#endif // HILOG_WRAPPER_H
