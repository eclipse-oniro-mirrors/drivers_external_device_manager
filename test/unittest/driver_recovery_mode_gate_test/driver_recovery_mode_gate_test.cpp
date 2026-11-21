/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "bus_extension/usb/usb_device_info.h"
#include "device.h"
#include "driver_extension_controller.h"
#include "edm_errors.h"
#include "recovery_mode_checker.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace testing::ext;

class DriverRecoveryModeGateTest : public testing::Test {
public:
    void SetUp() override
    {
        RecoveryModeChecker::SetParameterReaderForTest([]() { return std::string("0x2"); });
    }

    void TearDown() override
    {
        RecoveryModeChecker::ClearParameterReaderForTest();
    }
};

HWTEST_F(DriverRecoveryModeGateTest, StartDriverExtensionReturnsNotSupport, TestSize.Level1)
{
    int32_t ret = DriverExtensionController::GetInstance().StartDriverExtension("test.bundle", "TestAbility");
    EXPECT_EQ(ret, static_cast<int32_t>(UsbErrCode::EDM_ERR_NOT_SUPPORT));
}

HWTEST_F(DriverRecoveryModeGateTest, ConnectDriverExtensionReturnsNotSupport, TestSize.Level1)
{
    int32_t ret = DriverExtensionController::GetInstance().ConnectDriverExtension(
        "test.bundle", "TestAbility", nullptr, 1);
    EXPECT_EQ(ret, static_cast<int32_t>(UsbErrCode::EDM_ERR_NOT_SUPPORT));
}

HWTEST_F(DriverRecoveryModeGateTest, DeviceConnectReturnsNotSupport, TestSize.Level1)
{
    auto device = std::make_shared<Device>(std::make_shared<UsbDeviceInfo>(1));
    EXPECT_EQ(device->Connect(), static_cast<int32_t>(UsbErrCode::EDM_ERR_NOT_SUPPORT));
}
} // namespace ExternalDeviceManager
} // namespace OHOS
