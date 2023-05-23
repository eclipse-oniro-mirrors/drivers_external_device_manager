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
#include "json.h"
#include "hilog_wrapper.h"
#include "ibus_extension.h"
#include "usb_driver_info.h"
namespace OHOS {
namespace ExternalDeviceManager {
int DriverInfo::Serialize(string &str)
{
    string extInfo;
    if (this->driverInfoExt == nullptr) {
        EDM_LOGE(MODULE_COMMON, "Serialize error, this->driverInfoExt is nullptr");
        return -1;
    }
    this->driverInfoExt->Serialize(extInfo);
    Json::Value root;
    root["bus"] = this->bus;
    root["vendor"] = this->vendor;
    root["version"] = this->version;
    root["ext_info"] = extInfo;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    str = Json::writeString(builder, root);
    EDM_LOGI(MODULE_COMMON, "DriverInfo Serialize Done, %s", str.c_str());
    return 0;
}
static bool IsJsonObjValid(Json::Value jsonObj, string member, uint32_t type)
{
    if (!jsonObj.isMember(member)) {
        EDM_LOGE(MODULE_COMMON, "the json obj do not has menber :%s", member.c_str());
        return false;
    }
    auto realType = jsonObj[member].type();
    if (realType != type) {
        EDM_LOGE(MODULE_COMMON, "the json obj member[%s] type error, need:%d, which is:%d",
            member.c_str(), type, realType);
        return false;
    }
    return true;
}

int DriverInfo::UnSerialize(const string &str)
{
    Json::CharReaderBuilder builder;
    const unique_ptr<Json::CharReader> reader(builder.newCharReader());
    JSONCPP_STRING err;
    Json::Value jsonObj;
    string extInfo;
    auto rawJsonLength = static_cast<int>(str.length());
    EDM_LOGD(MODULE_COMMON, "UnSeiralize, input str is : [%s], length = %d", str.c_str(), rawJsonLength);
    bool ret = reader->parse(str.c_str(), \
                             str.c_str() + rawJsonLength, \
                             &jsonObj,\
                             &err);
    if (ret == false) {
        EDM_LOGE(MODULE_COMMON, "UnSeiralize error, parse json string error, ret = %d, str is : %s", ret, str.c_str());
        EDM_LOGE(MODULE_COMMON, "JsonErr:%s", err.c_str());
        return -1;
    }
    if (jsonObj.size() == 0) {
        EDM_LOGE(MODULE_COMMON, "JsonObj size is 0");
        return 0;
    }
    if (!IsJsonObjValid(jsonObj, "bus", Json::stringValue)\
        ||!IsJsonObjValid(jsonObj, "vendor", Json::stringValue)\
        ||!IsJsonObjValid(jsonObj, "version", Json::stringValue)\
        ||!IsJsonObjValid(jsonObj, "ext_info", Json::stringValue)) {
            EDM_LOGE(MODULE_COMMON, "json member or member type error");
            return -1;
        }
    auto busType = jsonObj["bus"].asString();
    if (LowerStr(busType) == "usb") {
        this->driverInfoExt = make_shared<UsbDriverInfo>();
    } else {
        EDM_LOGE(MODULE_COMMON, "unknow bus type");
        return -1;
    }
    extInfo = jsonObj["ext_info"].asString();
    if (this->driverInfoExt == nullptr) {
        EDM_LOGE(MODULE_COMMON, "error, this->driverInfoExt is nullptr");
        return -1;
    }
    ret = this->driverInfoExt->UnSerialize(extInfo);
    if (ret != 0) {
        EDM_LOGE(MODULE_COMMON, "parse ext_info error");
        return -1;
    }
    this->bus = jsonObj["bus"].asString();
    this->vendor = jsonObj["vendor"].asString();
    this->version = jsonObj["version"].asString();
    return 0;
}
}
}