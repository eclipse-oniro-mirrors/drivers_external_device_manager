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
#ifndef TEST_DEV_CHANGE_CALLBACK_H
#define TEST_DEV_CHANGE_CALLBACK_H
#include "ibus_extension.h"
#include "iostream"
#include "hilog_wrapper.h"
#include "cinttypes"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
class TestDevChangeCallback : public IDevChangeCallback {
public:
    map<uint64_t, shared_ptr<DeviceInfo>> devInfoMap_;
    TestDevChangeCallback() { };
    int32_t OnDeviceAdd(shared_ptr<DeviceInfo> device) override
    {
        devInfoMap_[device->GetDeviceId()] = device;
        EDM_LOGI(MODULE_COMMON, "OnDeviceAdd: Id = %{public}016" PRIx64 ", Desc = %{public}s",
            device->GetDeviceId(), device->GetDeviceDescription().c_str());
        PrintAllDevice();
        return 0;
    };
    int32_t OnDeviceRemove(shared_ptr<DeviceInfo> device) override
    {
        devInfoMap_.erase(device->GetDeviceId());
        EDM_LOGI(MODULE_COMMON, "OnDeviceRemove: Id = %{public}016" PRIx64 ", Desc = %{public}s",
            device->GetDeviceId(), device->GetDeviceDescription().c_str());
        PrintAllDevice();
        return 0;
    };
    void PrintAllDevice(void)
    {
        cout << "++++++++, all usb device, count = " << devInfoMap_.size() << " detail:" << endl;
        for (auto &devItem : devInfoMap_) {
            cout << devItem.second->GetDeviceDescription().c_str() << endl;
        }
        cout << "--------" << endl;
    }
};
}
}
#endif