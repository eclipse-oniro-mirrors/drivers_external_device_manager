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

export default function ddkInterfaceJsTest() {
  let usbDeviceId: number = -1;
  let serialDeviceId: number = -1;
  let scsiDeviceId: number = -1;

  describe('ddkInterfaceJsTest', () => {

    const TAG: string = "[ddkInterfaceJsTest]";
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
     * @tc.name       : testIntefaceScsiRelease002
     * @tc.desc       : OH_ScsiPeripheral_Release Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRelease002', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRelease002---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = scsiDdkNdkTest.scsiPeripheralReleaseOne();
        console.info("Test Result testIntefaceScsiRelease002 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRelease002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiOpen003
     * @tc.desc       : OH_ScsiPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen003', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen003---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen003 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

        /**
     * @tc.name       : testIntefaceScsiClose004
     * @tc.desc       : OH_ScsiPeripheral_Close Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiClose004', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiClose004---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralCloseOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiClose004 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiClose004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

        /**
     * @tc.name       : testInterfaceUsbInit005
     * @tc.desc       : OH_Usb_Init Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbInit005', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbInit005---------------------------');
      try {
        const ret = usbDdkNdkTest.usbInit();
        console.info("Test Result testInterfaceUsbInit005 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbInit005 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbRelease006
     * @tc.desc       : OH_Usb_Release Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbRelease006', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbRelease006---------------------------');
      try {
        const ret = usbDdkNdkTest.usbRelease();
        console.info("Test Result testInterfaceUsbRelease006 : " + ret);
        expect(ret).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbRelease006 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbReleaseResource007
     * @tc.desc       : OH_Usb_ReleaseResource Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbReleaseResource007', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbReleaseResource007---------------------------');
      try {
        const ret = usbDdkNdkTest.usbReleaseResource();
        console.info("Test Result testInterfaceUsbReleaseResource007 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbReleaseResource007 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbGetDeviceDescriptorOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetDeviceDescriptor008 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetDeviceDescriptor008 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetDeviceDescriptor009
     * @tc.desc       : OH_Usb_GetDeviceDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetDeviceDescriptor009', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetDeviceDescriptor009---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetDeviceDescriptorTwo(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetDeviceDescriptor009 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetDeviceDescriptor009 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbGetConfigDescriptorOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetConfigDescriptor010 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
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
        const ret = usbDdkNdkTest.usbGetConfigDescriptorTwo();
        console.info("Test Result testInterfaceUsbGetConfigDescriptor011 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetConfigDescriptor011 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetConfigDescriptor012
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetConfigDescriptor012', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetConfigDescriptor012---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetConfigDescriptorThree(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetConfigDescriptor012 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetConfigDescriptor012 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbFreeConfigDescriptor013
     * @tc.desc       : OH_Usb_FreeConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbFreeConfigDescriptor013', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbFreeConfigDescriptor013---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbFreeConfigDescriptor(usbDeviceId);
        console.info("Test Result testInterfaceUsbFreeConfigDescriptor013 : " + ret);
        expect(ret).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbFreeConfigDescriptor013 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiOpen014
     * @tc.desc       : OH_ScsiPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen014', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen014---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen014 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen014 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

        /**
     * @tc.name       : testIntefaceScsiClose015
     * @tc.desc       : OH_ScsiPeripheral_Close Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiClose015', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiClose015---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI deviscsiDeviceIdceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralCloseTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiClose015 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiClose015 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiOpen016
     * @tc.desc       : OH_ScsiPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen016', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen016---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen016 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen016 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiClose017
     * @tc.desc       : OH_ScsiPeripheral_Close Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiClose017', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiClose017---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralCloseThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiClose017 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiClose017 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiOpen018
     * @tc.desc       : OH_SCSIPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen018', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen018---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenFive(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen018 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen018 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity019
     * @tc.desc       : OH_ScsiPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity019', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity019---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity019 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity019 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

        /**
     * @tc.name       : testIntefaceScsiOpen020
     * @tc.desc       : OH_SCSIPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen020', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen020---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenSix(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen020 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen020 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity021
     * @tc.desc       : OH_ScsiPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity021', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity021---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity021 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity021 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

        /**
     * @tc.name       : testIntefaceScsiOpen022
     * @tc.desc       : OH_SCSIPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen022', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen022---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenSeven(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen022 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen022 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity023
     * @tc.desc       : OH_ScsiPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity023', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity023---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity023 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity023 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiOpen024
     * @tc.desc       : OH_SCSIPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen024', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen024---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenEight(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen024 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen024 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity025
     * @tc.desc       : OH_SCSIPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity025', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity025---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity025 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity025 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

        /**
     * @tc.name       : testIntefaceScsiOpen026
     * @tc.desc       : OH_SCSIPeripheral_Open Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiOpen026', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiOpen026---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralOpenNine(scsiDeviceId);
        console.info("Test Result testIntefaceScsiOpen026 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiOpen026 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity027
     * @tc.desc       : OH_SCSIPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity027', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity027---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacityFive(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity027 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity027 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

        /**
     * @tc.name       : testInterfaceUsbClaimInterface028
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbClaimInterface028', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbClaimInterface028---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbClaimInterfaceOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbClaimInterface028 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbClaimInterface028 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbClaimInterface029
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbClaimInterface029', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbClaimInterface029---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbClaimInterfaceTwo();
        console.info("Test Result testInterfaceUsbClaimInterface029 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbClaimInterface029 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbClaimInterface030
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbClaimInterface030', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbClaimInterface030---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbClaimInterfaceThree(usbDeviceId);
        console.info("Test Result testInterfaceUsbClaimInterface030 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbClaimInterface030 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbReleaseInterface031
     * @tc.desc       : OH_Usb_ReleaseInterface Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbReleaseInterface031', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbReleaseInterface031---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbReleaseInterface(usbDeviceId);
        console.info("Test Result testInterfaceUsbReleaseInterface031 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbReleaseInterface031 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSelectInterfaceSetting032
     * @tc.desc       : OH_Usb_SelectInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSelectInterfaceSetting032', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSelectInterfaceSetting032---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSelectInterfaceSettingOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbSelectInterfaceSetting032 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSelectInterfaceSetting032 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSelectInterfaceSetting033
     * @tc.desc       : OH_Usb_SelectInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSelectInterfaceSetting033', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSelectInterfaceSetting033---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSelectInterfaceSettingTwo(usbDeviceId);
        console.info("Test Result testInterfaceUsbSelectInterfaceSetting033 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSelectInterfaceSetting033 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetCurrentInterfaceSetting034
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetCurrentInterfaceSetting034', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetCurrentInterfaceSetting034---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetCurrentInterfaceSettingOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetCurrentInterfaceSetting034 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetCurrentInterfaceSetting034 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbGetCurrentInterfaceSetting035
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetCurrentInterfaceSetting035', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetCurrentInterfaceSetting035---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetCurrentInterfaceSettingTwo(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetCurrentInterfaceSetting035 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetCurrentInterfaceSetting035 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiTestUnitReady036
     * @tc.desc       : OH_ScsiPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiTestUnitReady036', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiTestUnitReady036---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiTestUnitReady036 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiTestUnitReady036 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity037
     * @tc.desc       : OH_SCSIPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity037', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity037---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacitySix(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity037 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity037 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiTestUnitReady038
     * @tc.desc       : OH_ScsiPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiTestUnitReady038', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiTestUnitReady038---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiTestUnitReady038 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiTestUnitReady038 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiReadCapacity039
     * @tc.desc       : OH_SCSIPeripheral_ReadCapacity10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiReadCapacity039', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiReadCapacity039---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadCapacitySeven(scsiDeviceId);
        console.info("Test Result testIntefaceScsiReadCapacity039 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiReadCapacity039 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiTestUnitReady040
     * @tc.desc       : OH_ScsiPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiTestUnitReady040', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiTestUnitReady040---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiTestUnitReady040 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiTestUnitReady040 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry041
     * @tc.desc       : OH_ScsiPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry041', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry041---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry041 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry041 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiTestUnitReady042
     * @tc.desc       : OH_SCSIPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiTestUnitReady042', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiTestUnitReady042---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiTestUnitReady042 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiTestUnitReady042 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry043
     * @tc.desc       : OH_ScsiPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry043', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry043---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry043 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry043 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiTestUnitReady044
     * @tc.desc       : OH_SCSIPeripheral_TestUnitReady Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiTestUnitReady044', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiTestUnitReady044---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralTestUnitReadyFive(scsiDeviceId);
        console.info("Test Result testIntefaceScsiTestUnitReady044 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiTestUnitReady044 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry045
     * @tc.desc       : OH_ScsiPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry045', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry045---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry045 : " + ret);
        expect(ret).assertEqual(SCSI_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry045 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense046
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense046', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense046---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense046 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense046 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry047
     * @tc.desc       : OH_SCSIPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry047', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry047---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry047 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry047 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense048
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense048', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense048---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense048 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense048 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry050
     * @tc.desc       : OH_SCSIPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry050', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry050---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryFive(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry050 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry050 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense051
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense051', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense051---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense051 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense051 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry052
     * @tc.desc       : OH_SCSIPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry052', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry052---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquirySix(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry052 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry052 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense053
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense053', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense053---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense053 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense053 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiInquiry054
     * @tc.desc       : OH_SCSIPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry054', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry054---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquirySeven(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry054 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry054 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense055
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense055', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense055---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseFive(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense055 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense055 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });


    /**
     * @tc.name       : testIntefaceScsiInquiry056
     * @tc.desc       : OH_SCSIPeripheral_Inquiry Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiInquiry056', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiInquiry056---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralInquiryEight(scsiDeviceId);
        console.info("Test Result testIntefaceScsiInquiry056 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiInquiry056 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRequestSense057
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRequestSense057', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRequestSense057---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralRequestSenseSix(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRequestSense057 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRequestSense057 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRead058
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRead058', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRead058---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRead058 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRead058 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiWrite059
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiWrite059', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiWrite059---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest2.scsiPeripheralWriteOne(scsiDeviceId);
        console.info("Test Result testIntefaceScsiWrite059 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiWrite059 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRead060
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRead060', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRead060---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRead060 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRead060 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiWrite061
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiWrite061', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiWrite061---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest2.scsiPeripheralWriteTwo(scsiDeviceId);
        console.info("Test Result testIntefaceScsiWrite061 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiWrite061 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRead062
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRead062', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRead062---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRead062 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRead062 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });


    /**
     * @tc.name       : testIntefaceScsiWrite063
     * @tc.desc       : OH_ScsiPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiWrite063', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiWrite063---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest2.scsiPeripheralWriteThree(scsiDeviceId);
        console.info("Test Result testIntefaceScsiWrite063 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiWrite063 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRead064
     * @tc.desc       : OH_SCSIPeripheral_Read10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRead064', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRead064---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRead064 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRead064 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

        /**
     * @tc.name       : testInterfaceUsbGetCurrentInterfaceSetting065
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbGetCurrentInterfaceSetting065', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbGetCurrentInterfaceSetting065---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbGetCurrentInterfaceSettingThree(usbDeviceId);
        console.info("Test Result testInterfaceUsbGetCurrentInterfaceSetting065 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbGetCurrentInterfaceSetting065 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSendControlReadRequest066
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSendControlReadRequest066', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSendControlReadRequest066---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbSendControlReadRequest066 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSendControlReadRequest066 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSendControlReadRequest067
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSendControlReadRequest067', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSendControlReadRequest067---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestTwo(usbDeviceId);
        console.info("Test Result testInterfaceUsbSendControlReadRequest067 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSendControlReadRequest067 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSendControlReadRequest068
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSendControlReadRequest068', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSendControlReadRequest068---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestThree(usbDeviceId);
        console.info("Test Result testInterfaceUsbSendControlReadRequest068 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSendControlReadRequest068 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSendControlReadRequest069
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSendControlReadRequest069', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSendControlReadRequest069---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestFour(usbDeviceId);
        console.info("Test Result testInterfaceUsbSendControlReadRequest069 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSendControlReadRequest069 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSendControlReadRequest070
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSendControlReadRequest070', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSendControlReadRequest070---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestFive(usbDeviceId);
        console.info("Test Result testInterfaceUsbSendControlReadRequest070 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSendControlReadRequest070 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testInterfaceUsbSendControlWriteRequest071
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.type       : Function
     */
    it('testInterfaceUsbSendControlWriteRequest071', 0, async (done: Function) => {
      console.info('----------------------testInterfaceUsbSendControlWriteRequest071---------------------------');
      try {
        if (usbDeviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB usbDeviceId = " + usbDeviceId);
        const ret = usbDdkNdkTest.usbSendControlWriteRequestOne(usbDeviceId);
        console.info("Test Result testInterfaceUsbSendControlWriteRequest071 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testInterfaceUsbSendControlWriteRequest071 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiWrite072
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiWrite072', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiWrite072---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest2.scsiPeripheralWriteFour(scsiDeviceId);
        console.info("Test Result testIntefaceScsiWrite072 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiWrite072 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRead073
     * @tc.desc       : OH_SCSIPeripheral_Read10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRead073', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRead073---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadFive(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRead073 : " + ret);
        let exp = (ret == SCSIPERIPHERAL_DDK_IO_ERROR || ret == SCSIPERIPHERAL_DDK_SUCCESS)
        expect(exp).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRead073 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiWrite0074
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiWrite0074', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiWrite0074---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest2.scsiPeripheralWriteFour1(scsiDeviceId);
        console.info("Test Result testIntefaceScsiWrite0074 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiWrite0074 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testIntefaceScsiRead075
     * @tc.desc       : OH_SCSIPeripheral_Read10 Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiRead075', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiRead075---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest.scsiPeripheralReadSix(scsiDeviceId);
        console.info("Test Result testIntefaceScsiRead075 : " + ret);
        expect(ret).assertEqual(SCSIPERIPHERAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiRead075 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

        /**
     * @tc.name       : testIntefaceScsiWrite076
     * @tc.desc       : OH_SCSIPeripheral_RequestSense Interface testing
     * @tc.type       : Function
     */
    it('testIntefaceScsiWrite076', 0, async (done: Function) => {
      console.info('----------------------testIntefaceScsiWrite076---------------------------');
      try {
        if (scsiDeviceId < 0) {
          console.info("Test SCSI device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test SCSI scsiDeviceId = " + scsiDeviceId);
        const ret = scsiDdkNdkTest2.scsiPeripheralWriteFive(scsiDeviceId);
        console.info("Test Result testIntefaceScsiWrite076 : " + ret);
        let exp = (ret == SCSIPERIPHERAL_DDK_IO_ERROR || ret == SCSIPERIPHERAL_DDK_SUCCESS)
        expect(exp).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testIntefaceScsiWrite076 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });
  })
}


