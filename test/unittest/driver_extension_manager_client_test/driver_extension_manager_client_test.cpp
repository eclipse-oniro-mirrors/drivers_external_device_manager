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

#include <cinttypes>
#include <iostream>
#include <gtest/gtest.h>
#include "driver_ext_mgr_callback_stub.h"
#include "driver_ext_mgr_client.h"
#include "edm_errors.h"
#include "ext_object.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace testing::ext;
class DrvExtMgrClientTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(DrvExtMgrClientTest, QueryDevice001, TestSize.Level1)
{
    uint32_t busType = static_cast<uint32_t>(BusType::BUS_TYPE_INVALID);
    std::vector<std::shared_ptr<DeviceData>> devices;
    UsbErrCode ret = DriverExtMgrClient::GetInstance().QueryDevice(busType, devices);
    ASSERT_EQ(ret, UsbErrCode::EDM_ERR_INVALID_PARAM);
    ASSERT_TRUE(devices.empty());
}

HWTEST_F(DrvExtMgrClientTest, QueryDevice002, TestSize.Level1)
{
    uint32_t busType = static_cast<uint32_t>(BusType::BUS_TYPE_USB);
    std::vector<std::shared_ptr<DeviceData>> devices;
    UsbErrCode ret = DriverExtMgrClient::GetInstance().QueryDevice(busType, devices);
    ASSERT_EQ(ret, UsbErrCode::EDM_OK);
    std::cout << "size of devices:" << devices.size() << std::endl;
}

HWTEST_F(DrvExtMgrClientTest, BindDevice001, TestSize.Level1)
{
    uint64_t deviceId = 0;
    sptr<IDriverExtMgrCallback> connectCallback = nullptr;
    UsbErrCode ret = DriverExtMgrClient::GetInstance().BindDevice(deviceId, connectCallback);
    ASSERT_EQ(ret, UsbErrCode::EDM_ERR_INVALID_PARAM);
}

class DriverExtMgrCallbackTest : public DriverExtMgrCallbackStub {
public:
    void OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg) override;

    void OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg) override;

    void OnUnBind(uint64_t deviceId, const ErrMsg &errMsg) override;
};

void DriverExtMgrCallbackTest::OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg)
{
    EDM_LOGE(EDM_MODULE_TEST, "ErrMsg:%{public}d:%{public}s, deviceId:%{public}016" PRIX64 "",
        static_cast<UsbErrCode>(errMsg.errCode), errMsg.msg.c_str(), deviceId);
    std::cout << "OnConnect {errCode:" << static_cast<UsbErrCode>(errMsg.errCode) << ", ";
    std::cout << "msg:" << errMsg.msg << ", ";
    std::cout << "deviceId:" << deviceId << "}" << std::endl;
}

void DriverExtMgrCallbackTest::OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGE(EDM_MODULE_TEST, "ErrMsg:%{public}d:%{public}s, deviceId:%{public}016" PRIX64 "",
        static_cast<UsbErrCode>(errMsg.errCode), errMsg.msg.c_str(), deviceId);
    std::cout << "OnDisconnect {errCode:" << static_cast<UsbErrCode>(errMsg.errCode) << ", ";
    std::cout << "msg:" << errMsg.msg << ", ";
    std::cout << "deviceId:" << deviceId << "}" << std::endl;
}

void DriverExtMgrCallbackTest::OnUnBind(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGE(EDM_MODULE_TEST, "ErrMsg:%{public}d:%{public}s, deviceId:%{public}016" PRIX64 "",
        static_cast<UsbErrCode>(errMsg.errCode), errMsg.msg.c_str(), deviceId);
    std::cout << "OnUnBind {errCode:" << static_cast<UsbErrCode>(errMsg.errCode) << ", ";
    std::cout << "msg:" << errMsg.msg << ", ";
    std::cout << "deviceId:" << deviceId << "}" << std::endl;
}

HWTEST_F(DrvExtMgrClientTest, BindDevice002, TestSize.Level1)
{
    uint64_t deviceId = 0;
    sptr<IDriverExtMgrCallback> connectCallback = new DriverExtMgrCallbackTest {};
    UsbErrCode ret = DriverExtMgrClient::GetInstance().BindDevice(deviceId, connectCallback);
    ASSERT_EQ(ret, UsbErrCode::EDM_OK);
}

HWTEST_F(DrvExtMgrClientTest, UnBindDevice001, TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbErrCode ret = DriverExtMgrClient::GetInstance().UnBindDevice(deviceId);
    ASSERT_EQ(ret, UsbErrCode::EDM_OK);
}
} // namespace ExternalDeviceManager
} // namespace OHOS