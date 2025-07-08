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
import hilog from '@ohos.hilog';
import deviceManager from '@ohos.driver.deviceManager';
import { BusinessError } from '@ohos.base';
import Utils from './Util.test';

const TAG = 0x0000;

function isUsbDevice(deviceId : long) {
  return (deviceId & 0x00000000FFFFFFFF) === deviceManager.BusType.USB;
}

function assertInterfaceDesc(interfaceDesc : deviceManager.USBInterfaceDesc) {
  hilog.info(TAG, 'testTag', 'interfaceDesc.bInterfaceNumber:' + interfaceDesc.bInterfaceNumber);
  expect(typeof(interfaceDesc.bInterfaceNumber)).assertEqual('number');
  hilog.info(TAG, 'testTag', 'interfaceDesc.bClass:' + interfaceDesc.bClass);
  expect(typeof(interfaceDesc.bClass)).assertEqual('number');
  hilog.info(TAG, 'testTag', 'interfaceDesc.bSubClass:' + interfaceDesc.bSubClass);
  expect(typeof(interfaceDesc.bSubClass)).assertEqual('number');
  hilog.info(TAG, 'testTag', 'interfaceDesc.bProtocol:' + interfaceDesc.bProtocol);
  expect(typeof(interfaceDesc.bProtocol)).assertEqual('number');
}

function assertUsbDeviceInfoExt(usbDeviceInfo : deviceManager.USBDeviceInfo) {
  hilog.info(TAG, 'testTag', 'usbDeviceInfo.vendorId:' + usbDeviceInfo.vendorId);
  expect(typeof(usbDeviceInfo.vendorId)).assertEqual('number');
  hilog.info(TAG, 'testTag', 'usbDeviceInfo.productId:' + usbDeviceInfo.productId);
  expect(typeof(usbDeviceInfo.productId)).assertEqual('number');
  expect(Array.isArray(usbDeviceInfo.interfaceDescList)).assertTrue();
  for (const desc of usbDeviceInfo.interfaceDescList) {
    assertInterfaceDesc(desc);
  }
}

function assertDeviceInfo(deviceInfo : deviceManager.DeviceInfo) {
  hilog.info(TAG, 'testTag', 'deviceInfo.deviceId:' + deviceInfo.deviceId);
  expect(typeof(deviceInfo.deviceId)).assertEqual('number');
  hilog.info(TAG, 'testTag', 'deviceInfo.isDriverMatched:' + deviceInfo.isDriverMatched);
  expect(typeof(deviceInfo.isDriverMatched)).assertEqual('boolean');
  if (deviceInfo.isDriverMatched) {
    hilog.info(TAG, 'testTag', 'deviceInfo.driverUid:' + deviceInfo.driverUid);
    expect(typeof(deviceInfo.driverUid)).assertEqual('string');
  }
  if (isUsbDevice(deviceInfo.deviceId)) {
    let deviceUsb = deviceInfo as deviceManager.USBDeviceInfo
    assertUsbDeviceInfoExt(deviceUsb)
  }
}

function assertDriverInfo(driverInfo : deviceManager.DriverInfo) {
  hilog.info(TAG, 'testTag', 'driverInfo.busType:' + driverInfo.busType);
  expect(typeof(driverInfo.busType)).assertEqual('number');
  hilog.info(TAG, 'testTag', 'driverInfo.driverUid:' + driverInfo.driverUid);
  expect(typeof(driverInfo.driverUid)).assertEqual('string');
  hilog.info(TAG, 'testTag', 'driverInfo.driverName:' + driverInfo.driverName);
  expect(typeof(driverInfo.driverName)).assertEqual('string');
  hilog.info(TAG, 'testTag', 'driverInfo.driverVersion:' + driverInfo.driverVersion);
  expect(typeof(driverInfo.driverVersion)).assertEqual('string');
  hilog.info(TAG, 'testTag', 'driverInfo.driverSize:' + driverInfo.driverSize);
  expect(typeof(driverInfo.driverSize)).assertEqual('string');
  hilog.info(TAG, 'testTag', 'driverInfo.description:' + driverInfo.description);
  expect(typeof(driverInfo.description)).assertEqual('string');
  if (driverInfo.busType === deviceManager.BusType.USB) {
    let driverInfoUsb = driverInfo as deviceManager.USBDriverInfo
    hilog.info(TAG, 'testTag', 'driverInfo.productIdList:' + JSON.stringify(driverInfoUsb.productIdList));
    expect(Array.isArray(driverInfoUsb.productIdList)).assertTrue();
    hilog.info(TAG, 'testTag', 'driverInfo.vendorIdList:' + JSON.stringify(driverInfoUsb.vendorIdList));
    expect(Array.isArray(driverInfoUsb.vendorIdList)).assertTrue();
    for (const productId of driverInfoUsb.productIdList) {
      expect(typeof(productId)).assertEqual('number');
    }
    for (const vendorId of driverInfoUsb.vendorIdList) {
      expect(typeof(vendorId)).assertEqual('number');
    }
  }
}

export default function DeviceManagerJsTest() {
  describe("DeviceManagerJsTest", ():void => {
    let deviceId: long = 0;
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
    const PARAMETER_ERROR_CODE = 401
    const SERVICE_EXCEPTION_CODE = 22900001
    const SERVICE_EXCEPTION_CODE_NEW = 26300001
    const SERVICE_NOT_ALLOW_ACCESS = 26300002
    const SERVICE_NOT_BOUND = 26300003

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0101
     * @tc.name       : testQueryDevices001
     * @tc.desc       : verify queryDevice result
     */
    it("testQueryDevices001", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, ():void => {
      hilog.info(TAG, 'testTag', '----------------------testQueryDevices001---------------------------');
      if (deviceId == 0) {
        hilog.info(TAG, 'testTag', 'Device ID has not been set.');
        expect(true).assertTrue();
        return;
      }
      try {
        let devices = deviceManager.queryDevices(deviceManager.BusType.USB);
        hilog.info(TAG, 'testTag', 'Test case testQueryDevices001 ret : ' + JSON.stringify(devices));
        expect(devices != null).assertEqual(true);
        expect(devices.length > 0).assertEqual(true);
        let deviceUsb = devices[0] as deviceManager.USBDevice;
        expect(deviceUsb != null).assertEqual(true);
        expect(deviceUsb.deviceId != null).assertEqual(true);
        expect(deviceUsb.description != null).assertEqual(true);
        expect(deviceUsb.vendorId != null).assertEqual(true);
        expect(deviceUsb.productId != null).assertEqual(true);
      } catch (err: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testQueryDevices001 catch err: ' + JSON.stringify(err));
        expect().assertFail();
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0102
     * @tc.name       : testQueryDevices002
     * @tc.desc       : verify queryDevice no param result
     */
    it('testQueryDevices002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, ():void => {
      hilog.info(TAG, 'testTag', '----------------------testQueryDevices002---------------------------');
      if (deviceId == 0) {
        hilog.info(TAG, 'testTag', 'Device ID has not been set.');
        expect(true).assertTrue();
        return;
      }
      try {
        let devices = deviceManager.queryDevices();
        hilog.info(TAG, 'testTag', 'Test case testQueryDevices002 ret : ' + JSON.stringify(devices));
        expect(devices != null).assertEqual(true);
        expect(devices[0].deviceId != null).assertTrue();
      } catch (err: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testQueryDevices002 catch err: ' + JSON.stringify(err));
        expect().assertFail();
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0103
     * @tc.name       : testQueryDevices003
     * @tc.desc       : verify queryDevice param is 12345
     */
    it('testQueryDevices003', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, ():void => {
      hilog.info(TAG, 'testTag', '----------------------testQueryDevices003---------------------------');
      try {
        let devices = deviceManager.queryDevices(12345);
        hilog.info(TAG, 'testTag', 'Test case testQueryDevices003 ret : ', JSON.stringify(devices));
        expect(devices.length == 0).assertTrue();
      } catch (err: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testQueryDevices003 catch err : ' + JSON.stringify(err));
        expect(err.code).assertEqual(SERVICE_EXCEPTION_CODE);
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0104
     * @tc.name       : testQueryDevices004
     * @tc.desc       : verify queryDevice errcode 22900001
     */
    it('testQueryDevices004', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL3, ():void => {
      hilog.info(TAG, 'testTag', '----------------------testQueryDevices004---------------------------');
      if (deviceId != 0) {
        hilog.info(TAG, 'testTag', 'Device ID has been set.');
        expect(true).assertTrue();
        return;
      }
      try {
        let devices = deviceManager.queryDevices();
        hilog.info(TAG, 'testTag', 'Test case testQueryDevices004 ret : ' + JSON.stringify(devices));
      } catch (err: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testQueryDevices004 catch err : ' + JSON.stringify(err));
        expect(err.code).assertEqual(SERVICE_EXCEPTION_CODE);
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0201
     * @tc.name       : testBindDriverWithDeviceId001
     * @tc.desc       : verify bindDriverWithDeviceId promise
     */
    it("testBindDriverWithDeviceId001", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done:()=>void) : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testBindDriverWithDeviceId001---------------------------');
      try {
        deviceManager.bindDriverWithDeviceId(12345, (error, data) => {
          expect(false).assertTrue();
          done();
        }).then(data => {
          expect(false).assertTrue();
          done();
        }, error => {
          expect(false).assertTrue();
          done();
        });
        expect(false).assertTrue();
        done();
      } catch (error: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testBindDriverWithDeviceId001 catch err : ' + JSON.stringify(error));
        expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
        expect(error.code != SERVICE_NOT_ALLOW_ACCESS).assertTrue();
        done();
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0202
     * @tc.name       : testBindDriverWithDeviceId002
     * @tc.desc       : verify bindDriverWithDeviceId promise success
     */
    it("testBindDriverWithDeviceId002", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done:()=>void) : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testBindDriverWithDeviceId002---------------------------');
      if (deviceId == 0) {
        hilog.info(TAG, 'testTag', 'Device ID has not been set.');
        expect(true).assertTrue();
        return done();
      }
      try {
        let data: deviceManager.RemoteDeviceDriver = await deviceManager.bindDriverWithDeviceId(deviceId,
          (err: BusinessError<void>|null, data: long|undefined) => {
            hilog.info(TAG, 'testTag', 'unbindDriverWithDeviceId callback success');
          });
        expect(true).assertTrue();
        hilog.info(0, 'testTag ui', 'bindDevice success:' + data.deviceId);
        hilog.info(0, 'testTag ui', 'bindDevice success:' + JSON.stringify(data.remote));
        done();
      } catch (error: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testBindDriverWithDeviceId002 catch err : ' + JSON.stringify(error));
        expect(false).assertTrue();
        done();
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0301
     * @tc.name       : testUnbindDriverWithDeviceId001
     * @tc.desc       : verify unbindDriverWithDeviceId promise success
     */
    it("testUnbindDriverWithDeviceId001", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done:()=>void) : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testUnbindDriverWithDeviceId001---------------------------');
      if (deviceId == 0) {
        hilog.info(TAG, 'testTag', 'Device ID has not been set.');
        expect(true).assertTrue();
        return done();
      }
      try {
        await deviceManager.unbindDriverWithDeviceId(deviceId);
        expect(true).assertTrue();
        await Utils.msSleep(2000);
        done();
      } catch (error: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testUnbindDriverWithDeviceId001 catch err :' + JSON.stringify(error));
        expect(false).assertTrue();
        done();
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0302
     * @tc.name       : testUnbindDriverWithDeviceId002
     * @tc.desc       : verify unbindDriverWithDeviceId promise and no binding relationship
     */
    it("testUnbindDriverWithDeviceId002", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done:()=>void) : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testUnbindDriverWithDeviceId002---------------------------');
      if (deviceId == 0) {
        hilog.info(TAG, 'testTag', 'Device ID has not been set.');
        expect(true).assertTrue();
        return done();
      }
      try {
        await deviceManager.unbindDriverWithDeviceId(deviceId).then(data => {
          expect(false).assertTrue();
          done();
        });
        done();
      } catch (error: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testUnbindDriverWithDeviceId002 catch err :' + JSON.stringify(error));
        expect(error.code).assertEqual(SERVICE_NOT_BOUND);
        done();
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0303
     * @tc.name       : testUnbindDriverWithDeviceId003
     * @tc.desc       : verify unbindDriverWithDeviceId promise
     */
    it("testUnbindDriverWithDeviceId003", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0,
      async (done:()=>void) : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testUnbindDriverWithDeviceId003---------------------------');
      try {
        await deviceManager.unbindDriverWithDeviceId(12345).then(data => {
          expect(false).assertTrue();
          done();
        });
        done();
      } catch (error: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testUnbindDriverWithDeviceId003 catch err :' + JSON.stringify(error));
        expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
        done();
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0401
     * @tc.name       : testQueryDeviceInfo001
     * @tc.desc       : verify queryDeviceInfo none deviceId
     */
    it("testQueryDeviceInfo001", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testQueryDeviceInfo001---------------------------');
      try {
        const deviceInfos = deviceManager.queryDeviceInfo();
        expect(Array.isArray(deviceInfos)).assertTrue();
        for (const deviceInfo of deviceInfos) {
          assertDeviceInfo(deviceInfo);
        }
      } catch (error: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testQueryDeviceInfo001 catch err :' + JSON.stringify(error));
        expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0402
     * @tc.name       : testQueryDeviceInfo002
     * @tc.desc       : verify queryDeviceInfo has deviceId
     */
    it("testQueryDeviceInfo002", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testQueryDeviceInfo002---------------------------');
      try {
        const deviceInfos = deviceManager.queryDeviceInfo(12345);
        expect(Array.isArray(deviceInfos)).assertTrue();
        for (const deviceInfo of deviceInfos) {
          assertDeviceInfo(deviceInfo);
        }
      } catch (error: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testQueryDeviceInfo002 catch err :' + JSON.stringify(error));
        expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0501
     * @tc.name       : testQueryDriverInfo001
     * @tc.desc       : verify queryDriverInfo none driverUid
     */
    it("testQueryDriverInfo001", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testQueryDriverInfo001---------------------------');
      try {
        let driverInfos = deviceManager.queryDriverInfo();
        expect(Array.isArray(driverInfos)).assertTrue();
        for (const driverInfo of driverInfos) {
          assertDriverInfo(driverInfo);
        }
      } catch (error: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testQueryDriverInfo001 catch err :' + JSON.stringify(error));
        expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
      }
    })

    /*
     * @tc.number     : SUB_Driver_Ext_DeviceManagerAPIFunc_0502
     * @tc.name       : testQueryDriverInfo002
     * @tc.desc       : verify queryDriverInfo none driverUid
     */
    it("testQueryDriverInfo002", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL0, async () : Promise<void> => {
      hilog.info(TAG, 'testTag', '----------------------testQueryDriverInfo002---------------------------');
      try {
        const driverInfos = deviceManager.queryDriverInfo('driver-12345');
        expect(Array.isArray(driverInfos)).assertTrue();
        for (const driverInfo of driverInfos) {
          assertDriverInfo(driverInfo);
        }
      } catch (error: BusinessError) {
        hilog.info(TAG, 'testTag', 'Test case testQueryDriverInfo002 catch err :' + JSON.stringify(error));
        expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
      }
    })
  })
}
