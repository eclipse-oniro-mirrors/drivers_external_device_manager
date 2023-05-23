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
namespace OHOS {
namespace ExternalDeviceManager {
struct UsbDriverInfo : public DriverInfoExt {
public:
    std::vector<uint16_t> pids;
    std::vector<uint16_t> vids;
    int Serialize(string &metaData)  override;
    int UnSerialize(const string &metaData) override;
};
}
}
#endif