/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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
/**
 * @file
 * @kit DriverDevelopmentKit
 */
import type rpc from '@ohos.rpc';
import type Want from '@ohos.app.ability.Want';
import DriverExtensionContext from 'application.DriverExtensionContext';
import hilog from '@ohos.hilog';

class MyService extends rpc.RemoteObject {
    constructor(descriptor: string) {
        super(descriptor);
    }

    public onRemoteMessageRequest(
        code: number,
        data: rpc.MessageSequence,
        reply: rpc.MessageSequence,
        options: rpc.MessageOption
    ): boolean | Promise<boolean> {
        return false;
    }
}

export default class DriverExtensionAbility {
    private callOnConnect(want: Want): rpc.RemoteObject {
        let p = this.onConnect(want);
        if (p instanceof Promise) {
            this.isOnConnectAsync = true;
            return await(p as Promise<rpc.RemoteObject>);
        } else {
            this.isOnConnectAsync = false;
            return p;
        }
    }
    private callOnDisConnect(want: Want) : void {
        let p = this.onDisconnect(want);
        if (p instanceof Promise) {
            this.isOnDisconnectAsync = true;
            return await(p as Promise<void>);
        } else {
            this.isOnDisconnectAsync = false;
            return;
        }
    }
    onInit(want: Want): void {
        hilog.info(0x0000, 'testTag', `onInit`);
    }
    onRelease(): void {
        hilog.info(0x0000, 'testTag', `onRelease`);
    }
    onConnect(want: Want): rpc.RemoteObject | Promise<rpc.RemoteObject> {
        hilog.info(0x0000, 'testTag', `onConnect`);
        let myService: rpc.RemoteObject = new MyService("onConnect");
        return myService;
    }
    onDisconnect(want: Want): undefined | Promise<void> {
        hilog.info(0x0000, 'testTag', `onDisconnect`);
        return undefined;
    }
    onDump(params: Array<string>): Array<string> {
        hilog.info(0x0000, 'testTag', `onDump`);
        return ['params'];
    }

    private connectCbInfo: long = 0;
    private disConnectCbInfo: long = 0;
    private isOnDisconnectAsync: boolean = true;
    private isOnConnectAsync: boolean = true;
    remoteObject: rpc.RemoteObject | null = null;
    context: DriverExtensionContext = new DriverExtensionContext();
}
