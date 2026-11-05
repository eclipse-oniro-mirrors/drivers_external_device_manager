/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

import deviceManager from '@ohos.driver.deviceManager'
import {describe, beforeAll, afterAll, it, expect} from 'deccjsunit/index'

describe("DeviceManagerEmulatorTest", function () {
    const EMULATOR_NOT_SUPPORT_CODE = 804
    const EMULATOR_NOT_SUPPORT_MESSAGE = "The capability is not supported on the emulator at this time."

    beforeAll(function() {
        console.info('DeviceManagerEmulatorTest beforeAll called');
    })

    afterAll(function() {
        console.info('DeviceManagerEmulatorTest afterAll called');
    })

    /*
     * @tc.name:DeviceManagerEmulator_queryDevices_001
     * @tc.desc:verify queryDevices throws 804 on emulator
     * @tc.type: FUNC
     */
    it("DeviceManagerEmulator_queryDevices_001", 0, function () {
        console.info('----------------------DeviceManagerEmulator_queryDevices_001---------------------------');
        try {
            deviceManager.queryDevices(deviceManager.BusType.USB);
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(EMULATOR_NOT_SUPPORT_CODE);
            expect(error.message).assertEqual(EMULATOR_NOT_SUPPORT_MESSAGE);
        }
    })

    /*
     * @tc.name:DeviceManagerEmulator_bindDevice_001
     * @tc.desc:verify bindDevice throws 804 on emulator
     * @tc.type: FUNC
     */
    it("DeviceManagerEmulator_bindDevice_001", 0, function () {
        console.info('----------------------DeviceManagerEmulator_bindDevice_001---------------------------');
        try {
            deviceManager.bindDevice(12345, () => {});
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(EMULATOR_NOT_SUPPORT_CODE);
            expect(error.message).assertEqual(EMULATOR_NOT_SUPPORT_MESSAGE);
        }
    })

    /*
     * @tc.name:DeviceManagerEmulator_bindDeviceDriver_001
     * @tc.desc:verify bindDeviceDriver throws 804 on emulator
     * @tc.type: FUNC
     */
    it("DeviceManagerEmulator_bindDeviceDriver_001", 0, function () {
        console.info('----------------------DeviceManagerEmulator_bindDeviceDriver_001---------------------------');
        try {
            deviceManager.bindDeviceDriver(12345, () => {});
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(EMULATOR_NOT_SUPPORT_CODE);
            expect(error.message).assertEqual(EMULATOR_NOT_SUPPORT_MESSAGE);
        }
    })

    /*
     * @tc.name:DeviceManagerEmulator_unbindDevice_001
     * @tc.desc:verify unbindDevice throws 804 on emulator
     * @tc.type: FUNC
     */
    it("DeviceManagerEmulator_unbindDevice_001", 0, function () {
        console.info('----------------------DeviceManagerEmulator_unbindDevice_001---------------------------');
        try {
            deviceManager.unbindDevice(12345);
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(EMULATOR_NOT_SUPPORT_CODE);
            expect(error.message).assertEqual(EMULATOR_NOT_SUPPORT_MESSAGE);
        }
    })

    /*
     * @tc.name:DeviceManagerEmulator_bindDriverWithDeviceId_001
     * @tc.desc:verify bindDriverWithDeviceId throws 804 on emulator
     * @tc.type: FUNC
     */
    it("DeviceManagerEmulator_bindDriverWithDeviceId_001", 0, function () {
        console.info('----------------------DeviceManagerEmulator_bindDriverWithDeviceId_001---------------------------');
        try {
            deviceManager.bindDriverWithDeviceId(12345, () => {});
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(EMULATOR_NOT_SUPPORT_CODE);
            expect(error.message).assertEqual(EMULATOR_NOT_SUPPORT_MESSAGE);
        }
    })

    /*
     * @tc.name:DeviceManagerEmulator_unbindDriverWithDeviceId_001
     * @tc.desc:verify unbindDriverWithDeviceId throws 804 on emulator
     * @tc.type: FUNC
     */
    it("DeviceManagerEmulator_unbindDriverWithDeviceId_001", 0, function () {
        console.info('----------------------DeviceManagerEmulator_unbindDriverWithDeviceId_001---------------------------');
        try {
            deviceManager.unbindDriverWithDeviceId(12345);
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(EMULATOR_NOT_SUPPORT_CODE);
            expect(error.message).assertEqual(EMULATOR_NOT_SUPPORT_MESSAGE);
        }
    })

    /*
     * @tc.name:DeviceManagerEmulator_queryDeviceInfo_001
     * @tc.desc:verify queryDeviceInfo throws 804 on emulator
     * @tc.type: FUNC
     */
    it("DeviceManagerEmulator_queryDeviceInfo_001", 0, function () {
        console.info('----------------------DeviceManagerEmulator_queryDeviceInfo_001---------------------------');
        try {
            deviceManager.queryDeviceInfo();
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(EMULATOR_NOT_SUPPORT_CODE);
            expect(error.message).assertEqual(EMULATOR_NOT_SUPPORT_MESSAGE);
        }
    })

    /*
     * @tc.name:DeviceManagerEmulator_queryDriverInfo_001
     * @tc.desc:verify queryDriverInfo throws 804 on emulator
     * @tc.type: FUNC
     */
    it("DeviceManagerEmulator_queryDriverInfo_001", 0, function () {
        console.info('----------------------DeviceManagerEmulator_queryDriverInfo_001---------------------------');
        try {
            deviceManager.queryDriverInfo();
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(EMULATOR_NOT_SUPPORT_CODE);
            expect(error.message).assertEqual(EMULATOR_NOT_SUPPORT_MESSAGE);
        }
    })
})
