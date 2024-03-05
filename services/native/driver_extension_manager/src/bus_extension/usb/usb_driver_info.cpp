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

#include "iostream"
#include "hilog_wrapper.h"
#include "edm_errors.h"
#include "usb_driver_info.h"
namespace OHOS {
namespace ExternalDeviceManager {

bool UsbDriverInfo::ArrayHandle(cJSON *root, cJSON *array, const string key)
{
    for (auto it : (key == "vids" ? vids_ : pids_)) {
        if (!cJSON_AddItemToArray(array, cJSON_CreateNumber(static_cast<double>(it)))) {
            EDM_LOGE(MODULE_BUS_USB,  "Additem to %{public}s error, it = %{public}d", key.c_str(), it);
            return false;
        }
    };

    if (!cJSON_AddItemToObject(root, key.c_str(), array)) {
        EDM_LOGE(MODULE_BUS_USB,  "Add %{public}s to jsonRoot error", key.c_str());
        return false;
    }
    return true;
}

int32_t UsbDriverInfo::ArrayInit(const string key, cJSON *root)
{
    cJSON* array = cJSON_CreateArray();
    if (!array) {
        EDM_LOGE(MODULE_BUS_USB,  "Create %{public}s error", key.c_str());
        return EDM_ERR_JSON_OBJ_ERR;
    }
    if (!ArrayHandle(root, array, key)) {
        cJSON_Delete(array);
        return EDM_NOK;
    }
    return EDM_OK;
}

int32_t UsbDriverInfo::Serialize(string &driverStr)
{
    cJSON* jsonRoot = cJSON_CreateObject();
    if (!jsonRoot) {
        EDM_LOGE(MODULE_BUS_USB,  "Create jsonRoot error");
        return EDM_ERR_JSON_OBJ_ERR;
    }

    int32_t ret = 0;
    vector<string> keys = {"vids", "pids"};
    for (auto key : keys) {
        ret = ArrayInit(key, jsonRoot);
        if (ret != EDM_OK) {
            break;
        }
    }

    if (ret == EDM_OK) {
        driverStr = cJSON_PrintUnformatted(jsonRoot);
    }

    cJSON_Delete(jsonRoot);
    return ret;
}

int32_t UsbDriverInfo::FillArray(const string key, vector<uint16_t> &array, cJSON *jsonObj)
{
    EDM_LOGD(MODULE_BUS_USB,  "Enter FillArray");
    cJSON* item = cJSON_GetObjectItem(jsonObj, key.c_str());
    if (!item) {
        EDM_LOGE(MODULE_BUS_USB,  "json member error, need menbers: %{public}s", key.c_str());
        return EDM_ERR_JSON_OBJ_ERR;
    }
    if (item->type != cJSON_Array) {
        EDM_LOGE(MODULE_BUS_USB,  "json member type error, %{public}s type is : %{public}d", \
            key.c_str(), item->type);
        return EDM_ERR_JSON_OBJ_ERR;
    }
    EDM_LOGD(MODULE_BUS_USB,  "arraysize:%{public}d", cJSON_GetArraySize(item));
    for (int i = 0; i < cJSON_GetArraySize(item); i++) {
        cJSON* it =  cJSON_GetArrayItem(item, i);
        if (!it) {
            EDM_LOGE(MODULE_BUS_USB,  "GetArrayItem fail");
            return EDM_ERR_JSON_OBJ_ERR;
        }
        if (it->type != cJSON_Number) {
            EDM_LOGE(MODULE_BUS_USB,  "json %{public}s type error, error type is: %{public}d", \
                key.c_str(), it->type);
            return EDM_ERR_JSON_OBJ_ERR;
        }
        array.push_back(static_cast<uint16_t>(it->valuedouble));
    }
    return EDM_OK;
}

int32_t UsbDriverInfo::UnSerialize(const string &driverStr)
{
    EDM_LOGD(MODULE_BUS_USB,  "UsbDrvInfo UnSerialize begin");
    cJSON* jsonObj = cJSON_Parse(driverStr.c_str());
    if (!jsonObj) {
        EDM_LOGE(MODULE_BUS_USB,  "UnSeiralize error, parse json string error, str is : %{public}s",\
            driverStr.c_str());
        EDM_LOGE(MODULE_BUS_USB,  "JsonErr:%{public}s", cJSON_GetErrorPtr());
        return EDM_ERR_JSON_PARSE_FAIL;
    }
    if (cJSON_GetArraySize(jsonObj) == 0) {
        EDM_LOGE(MODULE_BUS_USB,  "Json size error");
        return EDM_ERR_JSON_PARSE_FAIL;
    }
    EDM_LOGD(MODULE_BUS_USB,  "parse json sucess");

    vector<uint16_t> vids_;
    vector<uint16_t> pids_;
    int32_t ret = 0;

    ret = FillArray("vids", vids_, jsonObj);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB,  "Fill vids_ error");
        return ret;
    }
    ret = FillArray("pids", pids_, jsonObj);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB,  "Fill pids_ error");
        return ret;
    }

    EDM_LOGD(MODULE_BUS_USB,  "member type check sucess");
    this->pids_ = pids_;
    this->vids_ = vids_;
    return ret;
}
}
}