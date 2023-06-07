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

#include "driver_extension_controller.h"
#include "iostream"
#include "hilog_wrapper.h"
#include "string_ex.h"
#include "iservice_registry.h"
#include "accesstoken_kit.h"
#include "token_setproc.h"
#include "nativetoken_kit.h"
using namespace OHOS::ExternalDeviceManager;
using namespace OHOS;
using namespace std;
using namespace OHOS::Security::AccessToken;
constexpr uint32_t PARAM_COUNT_NEED = 4;
constexpr uint32_t PARAM_INDEX_OF_ACTION      = 1;
constexpr uint32_t PARAM_INDEX_OF_BUNDLENAME  = 2;
constexpr uint32_t PARAM_INDEX_OF_ABILITYNAME = 3;

TokenInfoParams g_sysInfoInstance = {
    .dcapsNum = 0,
    .permsNum = 0,
    .aclsNum = 0,
    .dcaps = nullptr,
    .perms = nullptr,
    .acls = nullptr,
    .processName = "usb_manager",
    .aplStr = "system_basic",
};


static void SetTestCaseNative (TokenInfoParams *infoInstance)
{
    uint64_t tokenId = GetAccessTokenId(infoInstance);
    int ret = SetSelfTokenID(tokenId);
    if (ret == 0) {
        cout << "SetSelfTokenID success" << endl;
    } else {
        cout << "SetSelfTokenID fail" << endl;
    }
    AccessTokenKit::ReloadNativeTokenInfo();
}


int main(int argc, char **argv)
{
    int ret = 0;
    cout << "driver_extension_controller_mt" << endl;
    cout << "usage: start/stop bundleName ExtensionAbilityName" << endl;
    if (argc != PARAM_COUNT_NEED) {
        cout << "error! wrong param count" << endl;
        return 0;
    }
    cout << "args: "<< argv[PARAM_INDEX_OF_ACTION] <<
        ", " << argv[PARAM_INDEX_OF_BUNDLENAME] <<
        ", " << argv[PARAM_INDEX_OF_ABILITYNAME] << endl;
    if (string(argv[1]) == "start") {
        cout << "begin to connect extension ability" << endl;
        SetTestCaseNative(&g_sysInfoInstance);
        ret = DriverExtensionController::StartDriverExtension(
            argv[PARAM_INDEX_OF_BUNDLENAME], argv[PARAM_INDEX_OF_ABILITYNAME]);
    } else if (string(argv[1]) == "stop") {
        cout << "begin to stop extension ability" << endl;
        SetTestCaseNative(&g_sysInfoInstance);
        ret = DriverExtensionController::StopDriverExtension(
            argv[PARAM_INDEX_OF_BUNDLENAME], argv[PARAM_INDEX_OF_ABILITYNAME]);
    } else {
        cout << "wrong param! please check!" << endl;
        return 0;
    }
    if (ret == 0) {
        cout << "Start / Stop Extension Ability sucess" << endl;
    } else {
        cout << "Start / Stop Extension Ability fail , ret = " << ret << endl;
    }
    return 0;
}