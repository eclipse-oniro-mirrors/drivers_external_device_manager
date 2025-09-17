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
import scsiDdkNdkTest2 from 'libscsi_ddk_ndk_test2.so'
import serialDdkNdkTest from 'libserial_ddk_ndk_test.so'
import deviceManager from "@ohos.driver.deviceManager";

export default function usbSerialInterfaceJsTest() {
  let usbDeviceId: number = -1;
  let serialDeviceId: number = -1;
  let scsiDeviceId: number = -1;

  describe('usbSerialInterfaceJsTest', () => {

    const TAG: string = "[usbSerialInterfaceJsTest]";
    const SCSI_DDK_NO_PERM: number = 201;
    const SCSI_DDK_INVALID_PARAMETER: number = 401;
    const SCSI_DDK_SUCCESS: number = 31700000;
    const SCSI_DDK_MEMORY_ERROR: number = 31700001;
    const SCSI_DDK_INVALID_OPERATION: number = 31700002;
    const SCSI_DDK_IO_ERROR: number = 31700003;
    const SCSI_DDK_TIMEOUT: number = 31700004;
    const SCSI_DDK_INIT_ERROR: number = 31700005;
    const SCSI_DDK_SERVICE_ERROR: number = 31700006;
    const SCSI_DDK_DEVICE_NOT_FOUND: number = 31700007;

    beforeAll(async () => {
      try {
        let devices: Array<deviceManager.Device> = deviceManager.queryDevices(deviceManager.BusType.USB);
        for (let item of devices) {
          let device: deviceManager.USBDevice = item as deviceManager.USBDevice;
          console.info(`Device id is ${device.scsiDeviceId}`)
          if (scsiDdkNdkTest.isScsiDevice(device.scsiDeviceId)) {
            console.info(`The device ${device.scsiDeviceId} is a SCSI device`);
            scsiDeviceId = device.scsiDeviceId;
            continue;
          }
          if (serialDdkNdkTest.isUsbSerialDevice(device.scsiDeviceId)) {
            console.info(`The device ${device.scsiDeviceId} is a USB Serial device`);
            serialDeviceId = device.scsiDeviceId;
            continue;
          }
          usbDeviceId = device.scsiDeviceId;
          console.info(`Scsi device id is ${device.scsiDeviceId}`)
        }
      } catch (error) {
        console.error(`Query device error. Code is ${error.code}, message is ${error.message}`);
      }
    })

    /**
     * @tc.name       : testIntefaceScsiInit001
     * @tc.desc       : OH_ScsiPeripheral_Init Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInit001', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInit001---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralInitOne();
        console.info("Test Result testIntefaceScsiInit001 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInit001 failed, message is ${err.message}`);
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
        const ret = scsiDdkNdkTest.scsiPeripheralOpenOne(deviceId);
        console.info("Test Result testScsiPeripheralOpen003 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralOpen003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testScsiPeripheralClose004
     * @tc.desc       : OH_ScsiPeripheral_Close Interface testing
     * @tc.type       : Function
     */
    it('testScsiPeripheralClose004', 0, async (done: Function) => {
      console.info('----------------------testScsiPeripheralClose004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviceId = " + deviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralCloseOne(deviceId);
        console.info("Test Result testScsiPeripheralClose004 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testScsiPeripheralClose004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbRelease005
     * @tc.desc       : OH_Usb_Release Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbRelease005', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbRelease005---------------------------');
      try {
        const ret = usbDdkNdkTest.usbRelease();
        console.info("Test Result testInterfaceUsbRelease005 : " + ret);
        expect(ret).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbRelease005 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbReleaseResource006
     * @tc.desc       : OH_Usb_ReleaseResource Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbReleaseResource006', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbReleaseResource006---------------------------');
      try {
        const ret = usbDdkNdkTest.usbReleaseResource();
        console.info("Test Result testInterfaceUsbReleaseResource006 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbReleaseResource006 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetDeviceDescriptor007
     * @tc.desc       : OH_Usb_GetDeviceDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetDeviceDescriptor007', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetDeviceDescriptor007---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetDeviceDescriptorOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetDeviceDescriptor007 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetDeviceDescriptor007 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetDeviceDescriptor008
     * @tc.desc       : OH_Usb_GetDeviceDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetDeviceDescriptor008', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetDeviceDescriptor008---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetDeviceDescriptorTwo(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetDeviceDescriptor008 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetDeviceDescriptor008 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetConfigDescriptor009
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetConfigDescriptor009', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetConfigDescriptor009---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetConfigDescriptorOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetConfigDescriptor009 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetConfigDescriptor009 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetConfigDescriptor010
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetConfigDescriptor010', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetConfigDescriptor010---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetConfigDescriptorTwo();
        console.info("Test Result testInterfaceUsbGetConfigDescriptor010 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetConfigDescriptor010 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetConfigDescriptor011
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetConfigDescriptor011', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetConfigDescriptor011---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetConfigDescriptorThree(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetConfigDescriptor011 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetConfigDescriptor011 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbFreeConfigDescriptor012
     * @tc.desc       : OH_Usb_FreeConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbFreeConfigDescriptor012', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbFreeConfigDescriptor012---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbFreeConfigDescriptor(usbDeviceId);
        console.info("Test Result testInterfaceUsbFreeConfigDescriptor012 : " + ret);
        expect(ret).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbFreeConfigDescriptor012 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiOpen013
     * @tc.desc       : OH_ScsiPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen013', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen013---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen013 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen013 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiClose014
     * @tc.desc       : OH_ScsiPeripheral_Close Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiClose014', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiClose014---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralCloseThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiClose014 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiClose014 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiOpen015
     * @tc.desc       : OH_SCSIPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen015', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen015---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenFive(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen015 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen015 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity016
     * @tc.desc       : OH_ScsiPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity016', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity016---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity016 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity016 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiOpen017
     * @tc.desc       : OH_SCSIPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen017', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen017---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenEight(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen017 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen017 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity018
     * @tc.desc       : OH_SCSIPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity018', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity018---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity018 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity018 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSelectInterfaceSetting019
     * @tc.desc       : OH_Usb_SelectInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSelectInterfaceSetting019', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSelectInterfaceSetting019---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSelectInterfaceSettingOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbSelectInterfaceSetting019 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSelectInterfaceSetting019 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSelectInterfaceSetting020
     * @tc.desc       : OH_Usb_SelectInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSelectInterfaceSetting020', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSelectInterfaceSetting020---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSelectInterfaceSettingTwo(usbDeviceId);
        console.info("Test Result testInterfaceUsbSelectInterfaceSetting020 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSelectInterfaceSetting020 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetCurrentInterfaceSetting021
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetCurrentInterfaceSetting021', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetCurrentInterfaceSetting021---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetCurrentInterfaceSettingOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetCurrentInterfaceSetting021 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetCurrentInterfaceSetting021 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetCurrentInterfaceSetting022
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetCurrentInterfaceSetting022', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetCurrentInterfaceSetting022---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetCurrentInterfaceSettingTwo(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetCurrentInterfaceSetting022 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetCurrentInterfaceSetting022 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiTestUnitReady023
     * @tc.desc       : OH_ScsiPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiTestUnitReady023', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiTestUnitReady023---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiTestUnitReady023 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiTestUnitReady023 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity024
     * @tc.desc       : OH_SCSIPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity024', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity024---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacitySix(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity024 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity024 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiTestUnitReady025
     * @tc.desc       : OH_ScsiPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiTestUnitReady025', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiTestUnitReady025---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiTestUnitReady025 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiTestUnitReady025 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity026
     * @tc.desc       : OH_SCSIPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity026', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity026---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacitySeven(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity026 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity026 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiTestUnitReady027
     * @tc.desc       : OH_ScsiPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiTestUnitReady027', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiTestUnitReady027---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiTestUnitReady027 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiTestUnitReady027 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry028
     * @tc.desc       : OH_ScsiPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry028', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry028---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry028 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry028 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiTestUnitReady029
     * @tc.desc       : OH_SCSIPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiTestUnitReady029', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiTestUnitReady029---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyFive(scsiDeviceId);
        console.info("Test Result testIntefaceScsiTestUnitReady029 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiTestUnitReady029 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry030
     * @tc.desc       : OH_ScsiPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry030', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry030---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry030 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry030 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense031
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense031', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense031---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense031 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense031 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry032
     * @tc.desc       : OH_SCSIPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry032', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry032---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry032 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry032 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense033
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense033', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense033---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense033 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense033 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense034
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense034', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense034---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense034 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense034 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry035
     * @tc.desc       : OH_SCSIPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry035', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry035---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquirySeven(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry035 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry035 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense036
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense036', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense036---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseSix(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense036 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense036 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRead037
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRead037', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRead037---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRead037 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRead037 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiWrite038
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiWrite038', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiWrite038---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest2.scsiPeripheralWriteTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiWrite038 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiWrite038 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRead039
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRead039', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRead039---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRead039 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRead039 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetCurrentInterfaceSetting040
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetCurrentInterfaceSetting040', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetCurrentInterfaceSetting040---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetCurrentInterfaceSettingThree(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetCurrentInterfaceSetting040 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetCurrentInterfaceSetting040 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSendControlReadRequest041
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSendControlReadRequest041', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSendControlReadRequest041---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbSendControlReadRequest041 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSendControlReadRequest041 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSendControlReadRequest042
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSendControlReadRequest042', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSendControlReadRequest042---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestFour(usbDeviceId);
        console.info("Test Result testInterfaceUsbSendControlReadRequest042 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSendControlReadRequest042 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSendControlReadRequest043
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSendControlReadRequest043', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSendControlReadRequest043---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestFive(usbDeviceId);
        console.info("Test Result testInterfaceUsbSendControlReadRequest043 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSendControlReadRequest043 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiWrite044
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiWrite044', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiWrite044---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest2.scsiPeripheralWriteFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiWrite044 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiWrite044 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiWrite0045
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiWrite0045', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiWrite0045---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest2.scsiPeripheralWriteFour1(scsiDeviceId);
        console.info("Test Result testIntefaceScsiWrite0045 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiWrite0045 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRead046
     * @tc.desc       : OH_SCSIPeripheral_Read10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRead046', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRead046---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadSix(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRead046 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRead046 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });
  })
}


