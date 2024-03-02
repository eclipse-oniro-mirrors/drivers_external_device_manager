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
#define FORMATED_EDM(module, fmt, ...) "%{pubilic}s: [%{public}s]" fmt, \
                        EDM_MGR_LABEL[module], __FUNCTION__, ##__VA_ARGS__

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

#ifdef LOG_DOMAIN
#undef LOG_DOMAIN
#endif
#define LOG_DOMAIN 0xD002550

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EDM"

#define TAG_LENGTH 20

// param of log interface, such as EDM_LOGF.
enum UsbMgrSubModule {
    MODULE_FRAMEWORK = 0,
    MODULE_SERVICE,
    MODULE_DEV_MGR,
    MODULE_PKG_MGR,
    MODULE_EA_MGR,
    MODULE_BUS_USB,
    MODULE_COMMON,
    MODULE_USB_DDK,
    EDM_MODULE_TEST,
    MODULE_HID_DDK,
    EDM_MODULE_BUTT,
};

enum PkgErrCode {
    PKG_OK = 0,
    PKG_FAILURE = -1,
    PKG_RDB_EXECUTE_FAILTURE = -2,
    PKG_RDB_NO_INIT = -3,
    PKG_RDB_EMPTY = -4,
    PKG_PERMISSION_DENIED = -5,
    PKG_NOP = -6,
    PKG_OVERFLOW = -7,
};

constexpr char EDM_MGR_LABEL[EDM_MODULE_BUTT][TAG_LENGTH] = {
    "EdmFwk",
    "EdmService",
    "EdmDevMgr",
    "EdmPkgMgr",
    "EdmEaMgr",
    "EdmBusUsbMgr",
    "EdmCommon",
    "EdmUsbDdk",
    "EdmTest",
    "EdmHidDdk",
};

// In order to improve performance, do not check the module range, module should less than EDM_MODULE_BUTT.
#define EDM_LOGF(module, ...) HILOG_FATAL(LOG_CORE, FORMATED_EDM(module, __VA_ARGS__))
#define EDM_LOGE(module, ...) HILOG_ERROR(LOG_CORE, FORMATED_EDM(module, __VA_ARGS__))
#define EDM_LOGW(module, ...) HILOG_WARN(LOG_CORE, FORMATED_EDM(module, __VA_ARGS__))
#define EDM_LOGI(module, ...) HILOG_INFO(LOG_CORE, FORMATED_EDM(module, __VA_ARGS__))
#define EDM_LOGD(module, ...) HILOG_DEBUG(LOG_CORE, FORMATED_EDM(module, __VA_ARGS__))
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
