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

#include "string_ex.h"
#include "cJSON.h"
#include "hilog_wrapper.h"
#include "ibus_extension.h"
#include "bus_extension_core.h"
#include "usb_driver_info.h"
namespace OHOS {
namespace ExternalDeviceManager {
int32_t DriverInfo::Serialize(string &str)
{
    string extInfo;
    if (this->driverInfoExt_ == nullptr) {
        EDM_LOGE(MODULE_COMMON, "Serialize error, this->driverInfoExt_ is nullptr");
        return EDM_ERR_INVALID_OBJECT;
    }
    this->driverInfoExt_->Serialize(extInfo);
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        EDM_LOGE(MODULE_COMMON, "Create jsonRoot error");
    }
    cJSON_AddStringToObject(root, "bus", this->bus_.c_str());
    cJSON_AddStringToObject(root, "vendor", this->vendor_.c_str());
    cJSON_AddStringToObject(root, "version", this->version_.c_str());
    cJSON_AddStringToObject(root, "ext_info", extInfo.c_str());
    str = cJSON_PrintUnformatted(root);
    EDM_LOGI(MODULE_COMMON, "DriverInfo Serialize Done, %{public}s", str.c_str());
    cJSON_Delete(root);
    return EDM_OK;
}
static bool IsJsonObjValid(const cJSON *jsonObj, const string &member)
{
    cJSON* item = cJSON_GetObjectItem(jsonObj, member.c_str());
    if (!item) {
        EDM_LOGE(MODULE_COMMON, "the json obj do not has menber :%{public}s", member.c_str());
        return false;
    }
    auto realType = item->type;
    if (realType != cJSON_String) {
        EDM_LOGE(MODULE_COMMON, "the json obj member[%{public}s] type error, need:%{public}d, which is:%{public}d",
            member.c_str(), cJSON_String, realType);
        return false;
    }
    return true;
}

static int32_t checkJsonObj(const cJSON *jsonObj)
{
    if (cJSON_GetArraySize(jsonObj) == 0) {
        EDM_LOGE(MODULE_COMMON, "JsonObj size is 0");
        return EDM_ERR_JSON_OBJ_ERR;
    }
    if (!IsJsonObjValid(jsonObj, "bus") || !IsJsonObjValid(jsonObj, "vendor") || !IsJsonObjValid(jsonObj, "version")
        || !IsJsonObjValid(jsonObj, "ext_info")) {
        EDM_LOGE(MODULE_COMMON, "json member or member type error");
        return EDM_ERR_JSON_OBJ_ERR;
    }
    return EDM_OK;
}

int32_t DriverInfo::UnSerialize(const string &str)
{
    auto rawJsonLength = static_cast<int32_t>(str.length());
    EDM_LOGD(MODULE_COMMON, "UnSeiralize, input str is : [%{public}s], length = %{public}d", \
        str.c_str(), rawJsonLength);

    cJSON* jsonObj = cJSON_Parse(str.c_str());
    if (!jsonObj) {
        EDM_LOGE(MODULE_COMMON, "UnSeiralize error, parse json string error, str is : %{public}s", \
            str.c_str());
        EDM_LOGE(MODULE_COMMON, "JsonErr:%{public}s", cJSON_GetErrorPtr());
        return EDM_ERR_JSON_PARSE_FAIL;
    }
    int32_t retCode = checkJsonObj(jsonObj);
    if (retCode != EDM_OK) {
        return retCode;
    }
    cJSON* jsonBus = cJSON_GetObjectItem(jsonObj, "bus");
    cJSON* jsonVendor = cJSON_GetObjectItem(jsonObj, "vendor");
    cJSON* jsonVersion = cJSON_GetObjectItem(jsonObj, "version");
    cJSON* jsonExtInfo = cJSON_GetObjectItem(jsonObj, "ext_info");
    string busType = jsonBus->valuestring;
    auto busExt = BusExtensionCore::GetInstance().GetBusExtensionByName(LowerStr(busType));
    if (busExt == nullptr) {
        EDM_LOGE(MODULE_COMMON, "unknow bus type. %{public}s", busType.c_str());
        return EDM_ERR_NOT_SUPPORT;
    }
    this->driverInfoExt_ = busExt->GetNewDriverInfoExtObject();
    if (this->driverInfoExt_ == nullptr) {
        EDM_LOGE(MODULE_COMMON, "error, this->driverInfoExt_ is nullptr");
        return EDM_EER_MALLOC_FAIL;
    }

    string extInfo = jsonExtInfo->valuestring;
    int32_t ret = this->driverInfoExt_->UnSerialize(extInfo);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_COMMON, "parse ext_info error");
        return ret;
    }
    this->bus_ = jsonBus->valuestring;
    this->vendor_ = jsonVendor->valuestring;
    this->version_ = jsonVersion->valuestring;
    return EDM_OK;
}
}
}