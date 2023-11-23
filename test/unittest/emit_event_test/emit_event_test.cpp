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
#include <gtest/gtest.h>
#include <vector>

#include <system_ability_definition.h>
#include <thread>
#include <linux/uinput.h>
#include "driver_ext_mgr_client.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace testing::ext;
class EmitEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override {};
    void TearDown() override {};

private:
    class LoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    };
};

static DriverExtMgrClient &edmClient = DriverExtMgrClient::GetInstance();
static int32_t deviceId = -1;
enum class LoadStatus {
    LOAD_SUCCESS,
    LOAD_FAILED,
    ALREADY_EXISTS,
};

static LoadStatus g_loadStatus_ = LoadStatus::LOAD_FAILED;
static sptr<IRemoteObject> g_saObject = nullptr;
static constexpr uint64_t START_SA_SERVICE_WAIT_TIME = 3;

void EmitEventTest::SetUpTestCase()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s get samgr failed", __func__);
        g_loadStatus_ = LoadStatus::LOAD_FAILED;
        return;
    }

    auto saObj = samgr->CheckSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (saObj != nullptr) {
        g_saObject = saObj;
        g_loadStatus_ = LoadStatus::ALREADY_EXISTS;
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s external device SA exist", __func__);
        return;
    }

    sptr<LoadCallback> loadCallback_ = new LoadCallback();
    int32_t ret = samgr->LoadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID, loadCallback_);
    if (ret != UsbErrCode::EDM_OK) {
        g_loadStatus_ = LoadStatus::LOAD_FAILED;
    }
    EDM_LOGE(EDM_MODULE_TEST, "%{public}s load hdf_ext_devmgr, ret:%{public}d", __func__, ret);
}

void EmitEventTest::TearDownTestCase()
{
    if (g_loadStatus_ == LoadStatus::LOAD_FAILED || g_loadStatus_ == LoadStatus::ALREADY_EXISTS) {
        return;
    }

    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s get samgr failed", __func__);
        return;
    }

    int32_t ret = samgr->UnloadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    EDM_LOGE(EDM_MODULE_TEST, "%{public}s unload hdf_ext_devmgr, ret:%{public}d", __func__, ret);
}

void EmitEventTest::LoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    std::cout << "load success: systemAbilityId:" << systemAbilityId
              << " IRemoteObject result:" << ((remoteObject != nullptr) ? "succeed" : "failed") << std::endl;
    g_loadStatus_ = LoadStatus::LOAD_SUCCESS;
    g_saObject = remoteObject;
}

void EmitEventTest::LoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    std::cout << "load failed: systemAbilityId:" << systemAbilityId << std::endl;
    g_loadStatus_ = LoadStatus::LOAD_FAILED;
    g_saObject = nullptr;
}

HWTEST_F(EmitEventTest, CheckSAServiceLoad001, TestSize.Level1)
{
    if (g_loadStatus_ != LoadStatus::ALREADY_EXISTS) {
        std::this_thread::sleep_for(std::chrono::seconds(START_SA_SERVICE_WAIT_TIME));
    }

    ASSERT_NE(g_saObject, nullptr);
}

HWTEST_F(EmitEventTest, CreateDevice001, TestSize.Level1)
{
    Hid_Device hidDevice = {
        .deviceName = "VSoC keyboard",
        .vendorId = 0x6006,
        .productId = 0x6008,
        .version = 1,
        .bustype = BUS_USB
    };
    std::vector<Hid_EventType> eventType = {HID_EV_KEY};
    Hid_EventTypeArray eventTypeArray = {.hidEventType = eventType.data(), .length = (uint16_t)eventType.size()};
    std::vector<Hid_KeyCode> keyCode = {HID_KEY_1, HID_KEY_SPACE, HID_KEY_BACKSPACE, HID_KEY_ENTER};
    Hid_KeyCodeArray keyCodeArray = {.hidKeyCode = keyCode.data(), .length = (uint16_t)keyCode.size()};
    Hid_EventProperties hidEventProp = {.hidEventTypes = eventTypeArray, .hidKeys = keyCodeArray};
    auto ret = edmClient.CreateDevice(&hidDevice, &hidEventProp);
    deviceId = ret;
    std::cout << "create device: deviceId:" << deviceId << std::endl;
    ASSERT_GE(ret, 0);
}

HWTEST_F(EmitEventTest, EmitEvent001, TestSize.Level1)
{
    std::vector<Hid_EmitItem> items = {
        {1, 0x14a, 108},
        {3, 0,     50 },
        {3, 1,     50 }
    };
    auto ret = edmClient.EmitEvent(0, items);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(EmitEventTest, EmitEvent002, TestSize.Level1)
{
    const uint16_t len = 21;
    std::vector<Hid_EmitItem> items;
    for (uint16_t i = 0; i < len; ++i) {
        Hid_EmitItem item = {1, 0x14a, 108};
        items.push_back(item);
    }
    auto ret = edmClient.EmitEvent(deviceId, items);
    ASSERT_NE(ret, 0);
}

HWTEST_F(EmitEventTest, EmitEvent003, TestSize.Level1)
{
    const uint16_t len = 20;
    std::vector<Hid_EmitItem> items;
    for (uint16_t i = 0; i < len; ++i) {
        Hid_EmitItem item = {1, 0x14a, 108};
        items.push_back(item);
    }
    auto ret = edmClient.EmitEvent(deviceId, items);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(EmitEventTest, DestroyDevice001, TestSize.Level1)
{
    std::cout << "destroy device: deviceId:" << deviceId << std::endl;
    int32_t ret = edmClient.DestroyDevice(deviceId);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(EmitEventTest, DestroyDevice002, TestSize.Level1)
{
    int32_t ret = edmClient.DestroyDevice(-1);
    ASSERT_NE(ret, 0);
}

HWTEST_F(EmitEventTest, DestroyDevice003, TestSize.Level1)
{
    const int16_t devId = 200;
    int32_t ret = edmClient.DestroyDevice(devId);
    ASSERT_NE(ret, 0);
}
} // namespace ExternalDeviceManager
} // namespace OHOS