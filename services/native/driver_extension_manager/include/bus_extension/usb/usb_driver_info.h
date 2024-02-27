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

#ifndef USB_DRIVER_INFO_H
#define USB_DRIVER_INFO_H
#include <vector>
#include "ibus_extension.h"
#include "cJSON.h"
namespace OHOS {
namespace ExternalDeviceManager {
class UsbDriverInfo : public DriverInfoExt {
public:
    int32_t Serialize(string &metaData)  override;
    int32_t UnSerialize(const string &metaData) override;
    bool ArrayHandle(cJSON *root, cJSON *array, const string key);
    int32_t ArrayInit(const string key, cJSON *root);
    int32_t FillArray(const string key, vector<uint16_t> &array, cJSON* jsonObj);
private:
    friend class UsbBusExtension;
    std::vector<uint16_t> pids_;
    std::vector<uint16_t> vids_;
};
}
}
#endif