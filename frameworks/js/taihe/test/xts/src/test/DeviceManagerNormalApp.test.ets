/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

import { describe, it, expect, TestType, Size, Level, beforeAll, beforeEach, afterEach, afterAll } from "../../index";
import hilog from '@ohos.hilog'
import deviceManager from '@ohos.driver.deviceManager'
import { BusinessError } from '@ohos.base';

const TAG = 0x0000;

export default function DeviceManagerJsNormalTest(){
  describe("DeviceManagerJsNormalTest", ():void => {
    let deviceId: double = 0;
    const SYSTEMAPI_DENIED_CODE = 202;

    beforeAll(() => {
      hilog.info(TAG, 'testTag', 'beforeAll called');
      try {
        let devices = deviceManager.queryDevices(deviceManager.BusType.USB);
        if (devices != null && devices.length > 0 && devices[0] != null) {
          deviceId = devices[0].deviceId;
          hilog.info(TAG, 'testTag', 'deviceId: ' + devices[0].deviceId);
          hilog.info(TAG, 'testTag', 'busType: ' + devices[0].busType);
          hilog.info(TAG, 'testTag', 'description: ' + devices[0].description);
        } else {
          hilog.info(TAG, 'testTag', 'No devices found.');
        }
      } catch (err) {
        hilog.info(TAG, 'testTag', 'err: ' + err);
      }
      if (deviceId == 0) {
        hilog.info(TAG, 'testTag', 'Device ID has not been set.');
      }
      hilog.info(TAG, 'testTag', 'beforeAll called end');
    })

    afterAll(() => {
      hilog.info(TAG, 'testTag', 'afterAll called');
    })

    beforeEach(() => {
      hilog.info(TAG, 'testTag', 'beforeEach called');
    })

    afterEach(() => {
      hilog.info(TAG, 'testTag', 'afterEach called');
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0404
     * @tc.name       : testNormalQueryDriverInfo001
     * @tc.desc       : verify queryDeviceInfo none deviceId for normal app
     */
    it("testNormalQueryDeviceInfo001", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testNormalQueryDeviceInfo001---------------------------');
      if (deviceId == 0) {
        hilog.info(TAG, 'testTag', 'Device ID has not been set.');
        expect(true).assertTrue();
        return Promise.resolve();
      }
      try {
        const deviceInfos = deviceManager.queryDeviceInfo();
        expect(false).assertTrue();
      } catch (error) {
        if (error instanceof BusinessError) {
          hilog.info(TAG, 'testTag', 'Test case testNormalQueryDeviceInfo001 catch err :' + JSON.stringify(error));
          expect(error.code).assertEqual(SYSTEMAPI_DENIED_CODE);
        } else {
          throw error
        }
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0504
     * @tc.name       : testNormalQueryDriverInfo001
     * @tc.desc       : verify queryDriverInfo none driverUid for normal app
     */
    it("testNormalQueryDriverInfo001", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testNormalQueryDriverInfo001---------------------------');
      if (deviceId == 0) {
        hilog.info(TAG, 'testTag', 'Device ID has not been set.');
        expect(true).assertTrue();
        return Promise.resolve();
      }
      try {
        let driverInfos = deviceManager.queryDriverInfo();
        expect(false).assertTrue();
      } catch (error) {
        if (error instanceof BusinessError) {
          hilog.info(TAG, 'testTag', 'Test case testNormalQueryDriverInfo001 catch err :' + JSON.stringify(error));
          expect(error.code).assertEqual(SYSTEMAPI_DENIED_CODE);
        } else {
          throw error
        }
      }
    })
  })
}