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
#include "json/json.h"
#include "hilog_wrapper.h"
#include "edm_errors.h"
#include "usb_driver_info.h"
namespace OHOS {
namespace ExternalDeviceManager {
int32_t UsbDriverInfo::Serialize(string &driverStr)
{
    Json::Value valueRoot;
    Json::Value valueVids;
    Json::Value valuePids;
    for (auto vid : vids_) {
        valueVids.append(Json::Value(vid));
    };
    for (auto pid : pids_) {
        valuePids.append(Json::Value(pid));
    };
    valueRoot["vids"] = valueVids;
    valueRoot["pids"] = valuePids;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    driverStr = Json::writeString(builder, valueRoot);
    return EDM_OK;
}

int32_t UsbDriverInfo::UnSerialize(const string &driverStr)
{
    EDM_LOGD(MODULE_BUS_USB,  "UsbDrvInfo UnSerialize begin");
    Json::CharReaderBuilder builder;
    unique_ptr<Json::CharReader> const reader(builder.newCharReader());
    JSONCPP_STRING err;
    Json::Value jsonObj;
    bool ret = reader->parse(driverStr.c_str(), driverStr.c_str() + driverStr.length(), &jsonObj, &err);
    if (ret == false) {
        EDM_LOGE(MODULE_BUS_USB,  "UnSeiralize error, parse json string error, ret = %{public}d, str is : %{public}s",\
            ret, driverStr.c_str());
        EDM_LOGE(MODULE_BUS_USB,  "JsonErr:%{public}s", err.c_str());
        return EDM_ERR_JSON_PARSE_FAIL;
    }
    if (jsonObj.size() == 0) {
        EDM_LOGE(MODULE_BUS_USB,  "Json size error");
        return EDM_ERR_JSON_PARSE_FAIL;
    }
    EDM_LOGD(MODULE_BUS_USB,  "parse json sucess");
    if (!jsonObj.isMember("vids") || !jsonObj.isMember("pids")) {
        EDM_LOGE(MODULE_BUS_USB,  "json member error, need menbers: vids, pids");
        return EDM_ERR_JSON_OBJ_ERR;
    }
    if (jsonObj["pids"].type() != Json::arrayValue || jsonObj["vids"].type() != Json::arrayValue) {
        EDM_LOGE(MODULE_BUS_USB,  "json member type error, pids type is : %{public}d, vids type is %{public}d", \
            jsonObj["pids"].type(), jsonObj["vids"].type());
        return EDM_ERR_JSON_OBJ_ERR;
    }
    EDM_LOGD(MODULE_BUS_USB,  "menber type check sucess");
    vector<uint16_t> vids_;
    vector<uint16_t> pids_;
    for (auto vid : jsonObj["vids"]) {
        if (vid.type() != Json::intValue) {
            EDM_LOGE(MODULE_BUS_USB,  "json vids type error, %{public}d", vid.type());
            return EDM_ERR_JSON_OBJ_ERR;
        }
        vids_.push_back(vid.asUInt());
    }
    for (auto pid : jsonObj["pids"]) {
        if (pid.type() != Json::intValue) {
            EDM_LOGE(MODULE_BUS_USB,  "json pid type error, %{public}d", pid.type());
            return EDM_ERR_JSON_OBJ_ERR;
        }
        pids_.push_back(pid.asUInt());
    }
    this->pids_ = pids_;
    this->vids_ = vids_;
    return EDM_OK;
}
}
}