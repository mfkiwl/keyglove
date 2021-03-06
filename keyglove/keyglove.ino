// Keyglove Controller source code - Main setup/loop controller
// 10/1/2010 by Jeff Rowberg <jeff@rowberg.net>

/* ============================================
Controller code is placed under the MIT license
Copyright (c) 2013 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

// =============================================================================
// specific build hardware/debug options (EDIT THIS FILE FOR YOUR HARDWARE)
// =============================================================================

#include "config.h"

// =============================================================================
// !!! NO EDITING SHOULD BE NECESSARY BEYOND THIS POINT !!!
// =============================================================================



/* ===============================================
 * LIBRARY INCLUDES FOR PROPER BUILD PROCESS
=============================================== */

#include <Wire.h>
#include <I2Cdev.h>
#include <ADXL345.h>
#include <ITG3200.h>
#include <MPU6050.h>
#include <HMC5883L.h>
#include <TCA6424A.h>
#include <iWRAP.h>



/* ===============================================
 * UNIVERSAL CONTROLLER DECLARATIONS
=============================================== */

uint8_t keygloveTick = 0;   // increments every ~10ms (100hz), loops at 100
uint32_t keygloveTock = 0;  // increments every 100 ticks, loops at 2^32 (~4 billion)
uint32_t keygloveTickTime = 0, keygloveTickTime0 = 0;

// pre-processor controlled global initializations and setup() calls
#include "setup.h"



/* ===============================================
 * MAIN SETUP ROUTINE
=============================================== */

void setup() {
    // call main setup function (see setup.h for details)
    keyglove_setup();
}



/* ===============================================
 * MAIN LOOP ROUTINE
=============================================== */

uint16_t testA = 0;
void loop() {
    // read all defined and active motion sensors
    #ifdef ENABLE_ACCELEROMETER
        if (activeAccelerometer && readyAccelerometerData) update_motion_accelerometer();
    #endif
    #ifdef ENABLE_GYROSCOPE
        if (activeGyroscope && readyGyroscopeData) update_motion_gyroscope();
    #endif
    #ifdef ENABLE_ACCELGYRO
        if (activeAccelGyro && readyAccelGyroData) update_motion_accelgyro();
    #endif
    #ifdef ENABLE_MAGNETOMETER
        if (activeMagnetometer && readyMagnetometerData) update_motion_magnetometer();
    #endif
    #ifdef ENABLE_FUSION
        if (activeFusion && readyFusionData) update_motion_fusion();
    #endif
    
    #ifdef ENABLE_USB
        update_hostif_usb();
    #endif
    #if (KG_HOSTIF & KG_HOSTIF_USB_SERIAL)
        update_hostif_usb_serial();
    #endif // KG_HOSTIF_USB_SERIAL

    #ifdef ENABLE_BT2
        update_hostif_bt2();
    #endif
    #if (KG_HOSTIF & KG_HOSTIF_BT2_SERIAL)
        update_hostif_bt2_serial();
    #endif // KG_HOSTIF_BT2_SERIAL

    // check for TIMER1 overflow limit and increment tick (should be every 10 ms)
    // 156 results in 9.937 ms, 157 results in 10.001 ms
    testA++;
    if (testA > 10000) {
        testA = 0;
        //txPacketLength = create_packet(txPacket, KG_PACKET_CLASS_SYSTEM, KG_PACKET_SYSTEM_TICK);
        //send_keyglove_packet(txPacket, txPacketLength, true);
    //}
    //if (timer1Overflow >= 157) {
        timer1Overflow = 0;
        keygloveTick++;
        //keygloveTickTime += micros() - keygloveTickTime0;
        //keygloveTickTime0 = micros();

        // check for 100 ticks and reset counter (should be every 1 second)
        if (keygloveTick == 100) {
            //keygloveTickTime = 0;
            keygloveTick = 0;
            keygloveTock++;
            #if (KG_HOSTIF > 0)
                txPacketLength = create_packet(txPacket, KG_PACKET_CLASS_SYSTEM, KG_PACKET_SYSTEM_TICK);
                send_keyglove_packet(txPacket, txPacketLength, true);
            #endif
        }

        #if (defined(ENABLE_ACCELEROMETER) && defined(ENABLE_GYROSCOPE))
            if (activeAccelerometer && activeGyroscope) update_motion();
        #endif

        //if (activeGestures) update_gestures();      // process motion into gestures
        if (activeTouch) update_touch();            // process touch sensor status
        if (activeKeyboard) update_keyboard();      // send any queued keyboard control data
        if (activeMouse) update_mouse();            // send any queued mouse control data
        if (activeJoystick) update_joystick();      // send any queued joystick control data

        // update feedback (should be every 10ms, sliced according to particular mode)
        #if (KG_FEEDBACK & KG_FEEDBACK_BLINK)
            update_feedback_blink();
        #endif // KG_FEEDBACK_BLINK
        #if (KG_FEEDBACK & KG_FEEDBACK_RGB)
            update_feedback_rgb();
        #endif // KG_FEEDBACK_RGB
        #if (KG_FEEDBACK & KG_FEEDBACK_PIEZO)
            update_feedback_piezo();
        #endif // KG_FEEDBACK_PIEZO
        #if (KG_FEEDBACK & KG_FEEDBACK_VIBRATE)
            update_feedback_vibrate();
        #endif // KG_FEEDBACK_VIBRATE
    }
}