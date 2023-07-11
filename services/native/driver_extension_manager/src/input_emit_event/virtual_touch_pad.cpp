/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "virtual_touch_pad.h"

#include <linux/input-event-codes.h>
#include <linux/uinput.h>

namespace OHOS {
namespace ExternalDeviceManager {
VirtualTouchPad::VirtualTouchPad(const uint32_t maxX, const uint32_t maxY, const uint32_t maxPressure)
    : VirtualDevice("VSoC touchpad", 0x6006)
{
    uinputDev_.absmin[ABS_X] = 0;
    uinputDev_.absmax[ABS_X] = maxX;
    uinputDev_.absmin[ABS_Y] = 0;
    uinputDev_.absmax[ABS_Y] = maxY;

    uinputDev_.absmin[ABS_PRESSURE] = 0;
    uinputDev_.absmax[ABS_PRESSURE] = maxPressure;

    miscellaneous_ = {MSC_SCAN};
    eventTypes_ = {EV_ABS, EV_KEY, EV_SYN, EV_MSC};
    keys_ = {BTN_TOOL_PEN, BTN_TOOL_RUBBER, BTN_TOUCH, BTN_STYLUS};
    properties_ = {INPUT_PROP_DIRECT};
    abs_ = {ABS_X, ABS_Y, ABS_PRESSURE};
}
} // namespace ExternalDeviceManager
} // namespace OHOS
