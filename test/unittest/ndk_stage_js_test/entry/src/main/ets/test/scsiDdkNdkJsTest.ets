/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import { beforeAll, describe, it, expect } from "@ohos/hypium"
import scsiDdkNdkTest from 'libscsi_ddk_ndk_test.so'
import deviceManager from "@ohos.driver.deviceManager";


export default function scsiDdkNdkJsTest() {
  let deviceId: number = -1;

  describe('scsiDdkNdkJsTest', () => {

    const TAG: string = "[scsiDdkNdkJsTest]";
    const SCSIPERIPHERAL_DDK_NO_PERM: number = 201;
    const SCSIPERIPHERAL_DDK_INVALID_PARAMETER: number = 401;
    const SCSIPERIPHERAL_DDK_SUCCESS: number = 31700000;
    const SCSIPERIPHERAL_DDK_MEMORY_ERROR: number = 31700001;
    const SCSIPERIPHERAL_DDK_INVALID_OPERATION: number = 31700002;
    const SCSIPERIPHERAL_DDK_IO_ERROR: number = 31700003;
    const SCSIPERIPHERAL_DDK_TIMEOUT: number = 31700004;
    const SCSIPERIPHERAL_DDK_INIT_ERROR: number = 31700005;
    const SCSIPERIPHERAL_DDK_SERVICE_ERROR: number = 31700006;
    const SCSIPERIPHERAL_DDK_DEVICE_NOT_FOUND: number = 31700007;

    beforeAll(async () => {
      try {
        let devices: Array<deviceManager.Device> = deviceManager.queryDevices(deviceManager.BusType.USB);
        for (let item of devices) {
          let device: deviceManager.USBDevice = item as deviceManager.USBDevice;
          console.info(`Device id is ${device.deviceId}`)
          if (!scsiDdkNdkTest.isScsiDevice(device.deviceId)) {
            console.info(`This id ${device.deviceId} is not a SCSI device`)
            continue;
          }
          deviceId = device.deviceId;
          console.info(`Usb serial device id is ${device.deviceId}`)
        }
      } catch (error) {
        console.error(`Failed to query device. Code is ${error.code}, message is ${error.message}`);
      }
    })

    /**
     * @tc.name       : testScsiPeripheralInit001
     * @tc.desc       : OH_ScsiPeripheral_Init Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralInit001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralInit001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralInitOne();
        console.info("Test Result testScsiPeripheralInit001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralInit001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralRelease001
     * @tc.desc       : OH_ScsiPeripheral_Release Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralRelease001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralRelease001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralReleaseOne();
        console.info("Test Result testScsiPeripheralRelease001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralRelease001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralRelease002
     * @tc.desc       : OH_ScsiPeripheral_Release Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralRelease002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralRelease002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralReleaseTwo();
        console.info("Test Result testScsiPeripheralRelease002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralRelease002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralOpen001
     * @tc.desc       : OH_ScsiPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralOpen001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralOpen001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenOne(deviceId);
        console.info("Test Result testScsiPeripheralOpen001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralOpen001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralOpen002
     * @tc.desc       : OH_ScsiPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralOpen002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralOpen002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenTwo(deviceId);
        console.info("Test Result testScsiPeripheralOpen002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralOpen002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralOpen003
     * @tc.desc       : OH_ScsiPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralOpen003', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralOpen003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenThree(deviceId);
        console.info("Test Result testScsiPeripheralOpen003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralOpen003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralOpen004
     * @tc.desc       : OH_ScsiPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralOpen004', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralOpen004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenFour(deviceId);
        console.info("Test Result testScsiPeripheralOpen004 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralOpen004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralClose001
     * @tc.desc       : OH_ScsiPeripheral_Close Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralClose001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralClose001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralCloseOne(deviceId);
        console.info("Test Result testScsiPeripheralClose001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralClose001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralClose002
     * @tc.desc       : OH_ScsiPeripheral_Close Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralClose002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralClose002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralCloseTwo(deviceId);
        console.info("Test Result testScsiPeripheralClose002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralClose002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralClose003
     * @tc.desc       : OH_ScsiPeripheral_Close Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralClose003', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralClose003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralCloseThree(deviceId);
        console.info("Test Result testScsiPeripheralClose003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralClose003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralReadCapacity001
     * @tc.desc       : OH_ScsiPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralReadCapacity001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralReadCapacity001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityOne(deviceId);
        console.info("Test Result testScsiPeripheralReadCapacity001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralReadCapacity001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralReadCapacity002
     * @tc.desc       : OH_ScsiPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralReadCapacity002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralReadCapacity002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityTwo(deviceId);
        console.info("Test Result testScsiPeripheralReadCapacity002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralReadCapacity002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralReadCapacity003
     * @tc.desc       : OH_ScsiPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralReadCapacity003', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralReadCapacity003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityThree(deviceId);
        console.info("Test Result testScsiPeripheralReadCapacity003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralReadCapacity003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralTestUnitReady001
     * @tc.desc       : OH_ScsiPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralTestUnitReady001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralTestUnitReady001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyOne(deviceId);
        console.info("Test Result testScsiPeripheralTestUnitReady001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralTestUnitReady001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralTestUnitReady002
     * @tc.desc       : OH_ScsiPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralTestUnitReady002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralTestUnitReady002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyTwo(deviceId);
        console.info("Test Result testScsiPeripheralTestUnitReady002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralTestUnitReady002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralTestUnitReady003
     * @tc.desc       : OH_ScsiPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralTestUnitReady003', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralTestUnitReady003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyThree(deviceId);
        console.info("Test Result testScsiPeripheralTestUnitReady003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralTestUnitReady003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralInquiry001
     * @tc.desc       : OH_ScsiPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralInquiry001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralInquiry001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryOne(deviceId);
        console.info("Test Result testScsiPeripheralInquiry001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralInquiry001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralInquiry002
     * @tc.desc       : OH_ScsiPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralInquiry002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralInquiry002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryTwo(deviceId);
        console.info("Test Result testScsiPeripheralInquiry002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralInquiry002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralInquiry003
     * @tc.desc       : OH_ScsiPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralInquiry003', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralInquiry003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryThree(deviceId);
        console.info("Test Result testScsiPeripheralInquiry003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralInquiry003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralRequestSense001
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralRequestSense001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralRequestSense001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseOne(deviceId);
        console.info("Test Result testScsiPeripheralRequestSense001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralRequestSense001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testRequestSense002
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testRequestSense002', 0, async (done: Function) => {
      console.info('----------------------testRequestSense002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseTwo(deviceId);
        console.info("Test Result testRequestSense002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testRequestSense002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testRequestSense003
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testRequestSense003', 0, async (done: Function) => {
      console.info('----------------------testRequestSense003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseThree(deviceId);
        console.info("Test Result testRequestSense003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testRequestSense003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralRead001
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralRead001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralRead001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadOne(deviceId);
        console.info("Test Result testScsiPeripheralRead001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralRead001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralRead002
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralRead002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralRead002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadTwo(deviceId);
        console.info("Test Result testScsiPeripheralRead002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralRead002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralRead003
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralRead003', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralRead003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadThree(deviceId);
        console.info("Test Result testScsiPeripheralRead003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralRead003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralWrite001
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralWrite001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralWrite001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralWriteOne(deviceId);
        console.info("Test Result testScsiPeripheralWrite001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralWrite001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralWrite002
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralWrite002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralWrite002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralWriteTwo(deviceId);
        console.info("Test Result testScsiPeripheralWrite002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralWrite002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralWrite003
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralWrite003', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralWrite003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralWriteThree(deviceId);
        console.info("Test Result testScsiPeripheralWrite003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralWrite003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralVerify001
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralVerify001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralVerify001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralVerifyOne(deviceId);
        console.info("Test Result testScsiPeripheralVerify001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralVerify001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralVerify002
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralVerify002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralVerify002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralVerifyTwo(deviceId);
        console.info("Test Result testScsiPeripheralVerify002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralVerify002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralVerify003
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralVerify003', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralVerify003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralVerifyThree(deviceId);
        console.info("Test Result testScsiPeripheralVerify003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralVerify003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralSendRequestByCDB001
     * @tc.desc       : OH_ScsiPeripheral_SendRequestByCDB Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralSendRequestByCDB001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralSendRequestByCDB001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralSendRequestByCDBOne(deviceId);
        console.info("Test result testScsiPeripheralSendRequestByCDB001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralSendRequestByCDB001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralSendRequestByCDB002
     * @tc.desc       : OH_ScsiPeripheral_SendRequestByCDB Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralSendRequestByCDB002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralSendRequestByCDB002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralSendRequestByCDBTwo();
        console.info("Test result testScsiPeripheralSendRequestByCDB002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralSendRequestByCDB002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralSendRequestByCDB003
     * @tc.desc       : OH_ScsiPeripheral_SendRequestByCDB Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralSendRequestByCDB003', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralSendRequestByCDB003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralSendRequestByCDBThree(deviceId);
        console.info("Test result testScsiPeripheralSendRequestByCDB003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralSendRequestByCDB003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralCreateDeviceMemMap001
     * @tc.desc       : OH_ScsiPeripheral_CreateDeviceMemMap Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralCreateDeviceMemMap001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralCreateDeviceMemMap001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralCreateDeviceMemMapOne(deviceId);
        console.info("Test result testScsiPeripheralCreateDeviceMemMap001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralCreateDeviceMemMap001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralCreateDeviceMemMap002
     * @tc.desc       : OH_ScsiPeripheral_CreateDeviceMemMap Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralCreateDeviceMemMap002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralCreateDeviceMemMap002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralCreateDeviceMemMapTwo(deviceId);
        console.info("Test result testScsiPeripheralCreateDeviceMemMap002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralCreateDeviceMemMap002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralDestroyDeviceMemMap001
     * @tc.desc       : OH_ScsiPeripheral_DestroyDeviceMemMap Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralDestroyDeviceMemMap001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralDestroyDeviceMemMap001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralDestroyDeviceMemMapOne(deviceId);
        console.info("Test result testScsiPeripheralDestroyDeviceMemMap001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralDestroyDeviceMemMap001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralDestroyDeviceMemMap002
     * @tc.desc       : OH_ScsiPeripheral_DestroyDeviceMemMap Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralDestroyDeviceMemMap002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralDestroyDeviceMemMap002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralDestroyDeviceMemMapTwo(deviceId);
        console.info("Test result testScsiPeripheralDestroyDeviceMemMap002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralDestroyDeviceMemMap002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralParseBasicSenseInfo001
     * @tc.desc       : OH_ScsiPeripheral_ParseBasicSenseInfo Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralParseBasicSenseInfo001', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralParseBasicSenseInfo001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralParseBasicSenseInfoOne();
        console.info("Test result testScsiPeripheralParseBasicSenseInfo001 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralParseBasicSenseInfo001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralParseBasicSenseInfo002
     * @tc.desc       : OH_ScsiPeripheral_ParseBasicSenseInfo Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralParseBasicSenseInfo002', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralParseBasicSenseInfo002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralParseBasicSenseInfoTwo();
        console.info("Test result testScsiPeripheralParseBasicSenseInfo002 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralParseBasicSenseInfo002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });
  })
}