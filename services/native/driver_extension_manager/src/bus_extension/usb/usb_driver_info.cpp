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
#include "cJSON.h"
#include "hilog_wrapper.h"
#include "edm_errors.h"
#include "usb_driver_info.h"
namespace OHOS {
namespace ExternalDeviceManager {
int32_t UsbDriverInfo::Serialize(string &driverStr)
{
    cJSON* jsonRoot = cJSON_CreateObject();
    if (!jsonRoot) {
        EDM_LOGE(MODULE_BUS_USB,  "Create jsonRoot error");
        return EDM_ERR_JSON_OBJ_ERR;
    }
    do {
        cJSON* jsonVids = cJSON_CreateArray();
        if (!jsonVids) {
            EDM_LOGE(MODULE_BUS_USB,  "Create jsonVids error");
            break;
        }
        cJSON* jsonPids = cJSON_CreateArray();
        if (!jsonPids) {
            EDM_LOGE(MODULE_BUS_USB,  "Create jsonPids error");
            cJSON_Delete(jsonVids);
            break;
        }

        for (auto vid : vids_) {
            if (!cJSON_AddItemToArray(jsonVids, cJSON_CreateNumber(static_cast<double>(vid)))) {
                EDM_LOGE(MODULE_BUS_USB,  "Additem to jsonVids error, vid = %{public}d", vid);
            }
        };
        for (auto pid : pids_) {
            if (!cJSON_AddItemToArray(jsonPids, cJSON_CreateNumber(static_cast<double>(pid)))) {
                EDM_LOGE(MODULE_BUS_USB,  "Additem to jsonPids error, pid = %{public}d", pid);
            }
        };

        if (!cJSON_AddItemToObject(jsonRoot, "vids", jsonVids)) {
            EDM_LOGE(MODULE_BUS_USB,  "Add jsonVids to jsonRoot error");
            cJSON_Delete(jsonVids);
            break;
        }
        if (!cJSON_AddItemToObject(jsonRoot, "pids", jsonPids)) {
            EDM_LOGE(MODULE_BUS_USB,  "Add jsonPids to jsonRoot error");
            cJSON_Delete(jsonPids);
            break;
        }
        driverStr = cJSON_PrintUnformatted(jsonRoot);
    } while (0);

    cJSON_Delete(jsonRoot);
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
    cJSON* jsonVids = cJSON_GetObjectItem(jsonObj, "vids");
    cJSON* jsonPids = cJSON_GetObjectItem(jsonObj, "pids");
    if (!jsonVids || !jsonPids) {
        EDM_LOGE(MODULE_BUS_USB,  "json member error, need menbers: vids, pids");
        return EDM_ERR_JSON_OBJ_ERR;
    }
    if (jsonVids->type != cJSON_Array || jsonPids->type != cJSON_Array) {
        EDM_LOGE(MODULE_BUS_USB,  "json member type error, pids type is : %{public}d, vids type is %{public}d", \
            jsonPids->type, jsonVids->type);
        return EDM_ERR_JSON_OBJ_ERR;
    }
    EDM_LOGD(MODULE_BUS_USB,  "menber type check sucess");
    vector<uint16_t> vids_;
    vector<uint16_t> pids_;
    for (int i = 0; i < cJSON_GetArraySize(jsonVids); i++) {
        cJSON* vid =  cJSON_GetArrayItem(jsonVids, i);
        if (vid->type != cJSON_Number) {
            EDM_LOGE(MODULE_BUS_USB,  "json vids type error, %{public}d", vid->type);
            return EDM_ERR_JSON_OBJ_ERR;
        }
        vids_.push_back(static_cast<uint16_t>(vid->valuedouble));
    }
    for (int i = 0; i < cJSON_GetArraySize(jsonPids); i++) {
        cJSON* pid =  cJSON_GetArrayItem(jsonPids, i);
        if (pid->type != cJSON_Number) {
            EDM_LOGE(MODULE_BUS_USB,  "json pid type error, %{public}d", pid->type);
            return EDM_ERR_JSON_OBJ_ERR;
        }
        pids_.push_back(static_cast<uint16_t>(pid->valuedouble));
    }
    this->pids_ = pids_;
    this->vids_ = vids_;
    return EDM_OK;
}
}
}