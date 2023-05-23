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
#include "json.h"
#include "hilog_wrapper.h"
#include "usb_driver_info.h"
namespace OHOS {
namespace ExternalDeviceManager {
int UsbDriverInfo::Serialize(string &driverStr)
{
    Json::Value valueRoot;
    Json::Value valueVids;
    Json::Value valuePids;
    for (auto vid : vids) {
        valueVids.append(Json::Value(vid));
    };
    for (auto pid : pids) {
        valuePids.append(Json::Value(pid));
    };
    valueRoot["vids"] = valueVids;
    valueRoot["pids"] = valuePids;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    driverStr = Json::writeString(builder, valueRoot);
    return 0;
}

int UsbDriverInfo::UnSerialize(const string &driverStr)
{
    EDM_LOGD(MODULE_BUS_USB,  "UsbDrvInfo UnSerialize begin");
    Json::CharReaderBuilder builder;
    unique_ptr<Json::CharReader> const reader(builder.newCharReader());
    JSONCPP_STRING err;
    Json::Value jsonObj;
    bool ret = reader->parse(driverStr.c_str(), driverStr.c_str() + driverStr.length(), &jsonObj, &err);
    if (ret == false) {
        EDM_LOGE(MODULE_BUS_USB,  "UnSeiralize error, parse json string error, ret = %d, str is : %s",\
            ret, driverStr.c_str());
        EDM_LOGE(MODULE_BUS_USB,  "JsonErr:%s", err.c_str());
        return -1;
    }
    if (jsonObj.size() == 0) {
        EDM_LOGE(MODULE_BUS_USB,  "Json size error");
        return -1;
    }
    EDM_LOGD(MODULE_BUS_USB,  "parse json sucess");
    if (!jsonObj.isMember("vids") || !jsonObj.isMember("pids")) {
        EDM_LOGE(MODULE_BUS_USB,  "json member error, need menbers: vids, pids");
        return -1;
    }
    if (jsonObj["pids"].type() != Json::arrayValue || jsonObj["vids"].type() != Json::arrayValue) {
        EDM_LOGE(MODULE_BUS_USB,  "json member type error, pids type is : %d, vids type is %d", \
            jsonObj["pids"].type(), jsonObj["vids"].type());
        return -1;
    }
    EDM_LOGD(MODULE_BUS_USB,  "menber type check sucess");
    vector<uint16_t> vids_;
    vector<uint16_t> pids_;
    for (auto vid : jsonObj["vids"]) {
        if (vid.type() != Json::intValue) {
            EDM_LOGE(MODULE_BUS_USB,  "json vids type error, %d", vid.type());
            return -1;
        }
        vids_.push_back(vid.asUInt());
    }
    for (auto pid : jsonObj["pids"]) {
        if (pid.type() != Json::intValue) {
            EDM_LOGE(MODULE_BUS_USB,  "json pid type error, %d", pid.type());
            return -1;
        }
        pids_.push_back(pid.asUInt());
    }
    this->pids = pids_;
    this->vids = vids_;
    return 0;
}
}
}