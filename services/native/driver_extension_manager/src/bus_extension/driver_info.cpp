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
int32_t DriverInfo::Serialize(string &str)
{
    string extInfo;
    if (this->driverInfoExt_ == nullptr) {
        EDM_LOGE(MODULE_COMMON, "Serialize error, this->driverInfoExt_ is nullptr");
        return EDM_ERR_INVALID_OBJECT;
    }
    this->driverInfoExt_->Serialize(extInfo);
    Json::Value root;
    root["bus"] = this->bus_;
    root["vendor"] = this->vendor_;
    root["version"] = this->version_;
    root["ext_info"] = extInfo;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    str = Json::writeString(builder, root);
    EDM_LOGI(MODULE_COMMON, "DriverInfo Serialize Done, %{public}s", str.c_str());
    return EDM_OK;
}
static bool IsJsonObjValid(const Json::Value &jsonObj, const string &member, uint32_t type)
{
    if (!jsonObj.isMember(member)) {
        EDM_LOGE(MODULE_COMMON, "the json obj do not has menber :%{public}s", member.c_str());
        return false;
    }
    auto realType = jsonObj[member].type();
    if (realType != type) {
        EDM_LOGE(MODULE_COMMON, "the json obj member[%{public}s] type error, need:%{public}d, which is:%{public}d",
            member.c_str(), type, realType);
        return false;
    }
    return true;
}

int32_t DriverInfo::UnSerialize(const string &str)
{
    Json::CharReaderBuilder builder;
    const unique_ptr<Json::CharReader> reader(builder.newCharReader());
    JSONCPP_STRING err;
    Json::Value jsonObj;
    string extInfo;
    auto rawJsonLength = static_cast<int32_t>(str.length());
    EDM_LOGD(MODULE_COMMON, "UnSeiralize, input str is : [%{public}s], length = %{public}d", \
        str.c_str(), rawJsonLength);
    bool ret = reader->parse(str.c_str(), \
                             str.c_str() + rawJsonLength, \
                             &jsonObj,\
                             &err);
    if (ret == false) {
        EDM_LOGE(MODULE_COMMON, "UnSeiralize error, parse json string error, ret = %{public}d, str is : %{public}s", \
            ret, str.c_str());
        EDM_LOGE(MODULE_COMMON, "JsonErr:%{public}s", err.c_str());
        return EDM_ERR_JSON_PARSE_FAIL;
    }
    if (jsonObj.size() == 0) {
        EDM_LOGE(MODULE_COMMON, "JsonObj size is 0");
        return EDM_ERR_JSON_PARSE_FAIL;
    }
    if (!IsJsonObjValid(jsonObj, "bus", Json::stringValue)\
        ||!IsJsonObjValid(jsonObj, "vendor", Json::stringValue)\
        ||!IsJsonObjValid(jsonObj, "version", Json::stringValue)\
        ||!IsJsonObjValid(jsonObj, "ext_info", Json::stringValue)) {
            EDM_LOGE(MODULE_COMMON, "json member or member type error");
            return EDM_ERR_JSON_OBJ_ERR;
        }
    auto busType = jsonObj["bus"].asString();
    if (LowerStr(busType) == "usb") {
        this->driverInfoExt_ = make_shared<UsbDriverInfo>();
        if (this->driverInfoExt_ == nullptr) {
            EDM_LOGE(MODULE_COMMON, "error, this->driverInfoExt_ is nullptr");
            return EDM_EER_MALLOC_FAIL;
        }
    } else {
        EDM_LOGE(MODULE_COMMON, "unknow bus type");
        return EDM_ERR_NOT_SUPPORT;
    }
    extInfo = jsonObj["ext_info"].asString();
    ret = this->driverInfoExt_->UnSerialize(extInfo);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_COMMON, "parse ext_info error");
        return ret;
    }
    this->bus_ = jsonObj["bus"].asString();
    this->vendor_ = jsonObj["vendor"].asString();
    this->version_ = jsonObj["version"].asString();
    return EDM_OK;
}
}
}