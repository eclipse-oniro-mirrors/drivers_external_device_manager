/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef USB_DDK_SERVICE_MOCK_H
#define USB_DDK_SERVICE_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "v1_1/iusb_ddk.h"

namespace OHOS {
namespace USB {
using namespace OHOS::HDI::Usb::Ddk;
class UsbDdkServiceMock : public V1_1::IUsbDdk {
public:
    UsbDdkServiceMock() = default;
    ~UsbDdkServiceMock() = default;
    int32_t GetConfigDescriptor(uint64_t deviceId, uint8_t configIndex, std::vector<uint8_t> &configDesc) override;
    MOCK_METHOD0(Init, int32_t());
    MOCK_METHOD0(Release, int32_t());
    MOCK_METHOD2(GetDeviceDescriptor, int32_t(uint64_t deviceId, V1_1::UsbDeviceDescriptor &desc));
    MOCK_METHOD3(ClaimInterface, int32_t(uint64_t deviceId, uint8_t interfaceIndex, uint64_t &interfaceHandle));
    MOCK_METHOD1(ReleaseInterface, int32_t(uint64_t interfaceHandle));
    MOCK_METHOD2(SelectInterfaceSetting, int32_t(uint64_t interfaceHandle, uint8_t settingIndex));
    MOCK_METHOD2(GetCurrentInterfaceSetting, int32_t(uint64_t interfaceHandle, uint8_t &settingIndex));
    MOCK_METHOD4(SendControlReadRequest, int32_t(uint64_t interfaceHandle, const V1_1::UsbControlRequestSetup &setup,
        uint32_t timeout, std::vector<uint8_t> &data));
    MOCK_METHOD4(SendControlWriteRequest, int32_t(uint64_t interfaceHandle, const V1_1::UsbControlRequestSetup &setup,
        uint32_t timeout, const std::vector<uint8_t> &data));
    MOCK_METHOD5(SendPipeRequest, int32_t(const V1_1::UsbRequestPipe &pipe, uint32_t size, uint32_t offset,
        uint32_t length, uint32_t &transferedLength));
    MOCK_METHOD2(GetDeviceMemMapFd, int32_t(uint64_t deviceId, int &fd));
    MOCK_METHOD3(SendPipeRequestWithAshmem, int32_t(const V1_1::UsbRequestPipe &pipe, const V1_1::UsbAshmem &ashmem,
        uint32_t &transferredLength));
    MOCK_METHOD1(GetDevices, int32_t(std::vector<uint64_t> &deviceIds));
    MOCK_METHOD1(UpdateDriverInfo, int32_t(const V1_1::DriverAbilityInfo &driverInfo));
    MOCK_METHOD1(RemoveDriverInfo, int32_t(const std::string &driverUid));
};
} // namespace USB
} // namespace OHOS
#endif