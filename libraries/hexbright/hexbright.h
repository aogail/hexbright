/*
Copyright (c) 2012, "David Hilton" <dhiltonp@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
*/

#include <Wire.h>
#include <Arduino.h>


// debugging related definitions
//#define DEBUG 7
// Some debug modes set the light.  Your control code may reset it, causing weird flashes at startup.
#define DEBUG_LOOP 1 // main loop
#define DEBUG_LIGHT 2 // Light control
#define DEBUG_TEMP 3  // temperature safety
#define DEBUG_BUTTON 4 // button presses/rear led
#define DEBUG_ACCEL 5 // accelerometer
#define DEBUG_NUMBER 6 // number printing utility
#define DEBUG_CHARGE 7 // charge state


#ifdef DEBUG
#define OVERHEAT_TEMPERATURE 265 // something lower, to more easily verify algorithms
#else
#define OVERHEAT_TEMPERATURE 320 // 340 in original code, 320 = 130* fahrenheit/55* celsius (with calibration)
#endif


///////////////////////////////////
// key points on the light scale //
///////////////////////////////////
#define MAX_LEVEL 1000
#define MAX_LOW_LEVEL 500
#define CURRENT_LEVEL -1

#define NOW 1


// led constants
#define RLED 0
#define GLED 1

#define LED_OFF 0
#define LED_WAIT 1
#define LED_ON 2

// charging constants
#define CHARGING 0
#define BATTERY 1  
#define CHARGED 2

class hexbright {
  public: 
    // ms_delay is the time update will try to wait between runs.
    // the point of ms_delay is to provide regular update speeds,
    // so if code takes longer to execute from one run to the next,
    // the actual interface doesn't change (button click duration,
    // brightness changes). Set this from 5-30. very low is generally
    // fine (or great), BUT if you do any printing, the actual delay
    // may be greater than the value you set.
    hexbright(int update_delay_ms);
  
    // init hardware.
    // put this in your setup().
    static void init_hardware();
    
    // Put update in your loop().  It will block until update_delay has passed.
    static void update();

    // turn off the hexbright.
    // only turns off light when under USB power.  As such, you may want to re-initialize 
    // your variables before a shutdown.
    static void shutdown();


    // go from start_level to end_level over updates (updates*ms_delay) = ms
    // level is from 0-1000. 
    // 0 = no light, 500 = MAX_LOW_LEVEL, MAX_LEVEL=1000.
    // start_level can be CURRENT_LEVEL
    static void set_light(int start_level, int end_level, int updates);
    // get light level (before overheat protection adjustment)
    static int get_light_level();
    // get light level (after overheat protection adjustment)
    static int get_safe_light_level();

    // Returns the duration the button has been in updates.  Keeps its value 
    //  immediately after being released, allowing for use as follows:
    // if(button_released() && button_held()>500)
    // While the button is down, RLED cannot be set.
    static int button_held();
    // button has been released
    static boolean button_released();
    
    // led = GLED or RLED,
    // state = LED_ON or LED_OFF.  LED_WAIT cannot be directly set.
    // updates = updates before led goes to LED_WAIT state (where it remains for 100 ms)
    // While the RLED is on, the rear button cannot be read.
    static void set_led_state(byte led, byte state, int updates);
    // led = GLED or RLED 
    // returns LED_OFF, LED_WAIT, or LED_ON
    static byte get_led_state(byte led);
    // returns the opposite color than the one passed in
    static byte flip_color(byte color);


    // get raw thermal sensor reading
    static int get_thermal_sensor();
    static int get_celsius();
    static int get_fahrenheit();

    // returns CHARGING, CHARGED, or BATTERY
    static byte get_charge_state();

    
    // prints a number through the rear leds
    // 120 = 1 red flashes, 2 green flashes, one long red flash (0), 2 second delay.
    // the largest printable value is +/-999,999,999, as the left-most digit is reserved.
    // negative numbers begin with a leading long flash.
    static void print_number(long number);
    // currently printing a number
    static boolean printing_number();

 private: 
    static void adjust_light();
    static void set_light_level(unsigned long level);
    static void overheat_protection();

    static void update_number();

    // controls actual led hardware set.  
    //  As such, state = HIGH or LOW
    static void _set_led(byte led, byte state);
    static void adjust_leds();

    static void read_thermal_sensor();

    static void read_button();

    static void read_accelerometer();
    static void enable_accelerometer();
    static void disable_accelerometer();
};


