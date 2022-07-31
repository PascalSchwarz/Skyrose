#include <Arduino.h>
#include <DMXSerial.h>
#include <L298N.h>
#include "A4988.h"
#include <EEPROM.h>

//------------------------------------
// Defines for Pins
//------------------------------------

// Pins for the stepper motor shutter
#define MOTOR_STEPS 200
#define DIR 2
#define STEP 3

// Pins for DC motors in azimut
#define azIN1 10
#define azIN2 9
#define azEN 11
// Pins for DC motors in mirror
#define miIN1 7
#define miIN2 8
#define miEN 6

// Lamp control
#define lampOn 12

// DMX address set button
#define setDMXBtn 4

// Endstops for Azimut
#define endStopL A0
#define endStopR A1

// Onboard LED
#define LED 13

//------------------------------------
// Defines for other stuff
//------------------------------------

#define azOffset 0
#define miOffset 1
#define shutOffset 2
#define lampOffset 3

// speed and microstep for stepper
#define MICROSTEPS 1
#define RPM 120

//------------------------------------
// Globals and instances of stuff
//------------------------------------

// init of stepper
A4988 shutter(MOTOR_STEPS, DIR, STEP);

// Create azimut motor instance
L298N azimut(azEN, azIN1, azIN2);

// Create mirror motor instance
L298N mirror(miEN, miIN1, miIN2);

// dmxAddr variable, will be loaded from eeprom at startup
uint16_t dmxAddr = 0;

//------------------------------------
// Run setup once
//------------------------------------

void setup() {
  // init DMX
  DMXSerial.init(DMXReceiver);

  // set pinmodes for outputs
  pinMode(LED, OUTPUT);
  pinMode(lampOn, OUTPUT);

  // set pinmodes for inputs
  pinMode(setDMXBtn, INPUT_PULLUP);
  pinMode(endStopL, INPUT_PULLUP);
  pinMode(endStopR, INPUT_PULLUP);

  // init stepper for shutter
  shutter.begin(RPM, MICROSTEPS);

  // get DMX address for EEPROM
  EEPROM.get(0, dmxAddr);
}

//------------------------------------
// Run loop until heatdeath of universe
//------------------------------------

void loop() {
  static uint32_t loopcnt = 0;

  //------------------------------------
  // Handle mirror reflector
  //------------------------------------
  int16_t mirrorSpeed = (int16_t)DMXSerial.read(dmxAddr + miOffset) - (int16_t)127;

  mirror.setSpeed(abs(mirrorSpeed));
  if(mirrorSpeed > 0)
  {
    // turn mirror in the one direction
    mirror.forward();
  } 
  else
  {
    // other direction
    mirror.backward();
  }

  //------------------------------------
  // Handle azimut
  //------------------------------------
  uint8_t azimutSpeed = DMXSerial.read(dmxAddr + azOffset);

  azimut.setSpeed(azimutSpeed);
  if(digitalRead(endStopR) == LOW and azimut.getDirection() == L298N::BACKWARD)
  {
    // we hit the endstop, reverse motor
    azimut.forward();
  }
  if(digitalRead(endStopL) == LOW and azimut.getDirection() == L298N::FORWARD)
  {
    // hit the other endstop, reverse again!
    azimut.backward();
  }

  //------------------------------------
  // Handle shutter
  //------------------------------------
  if(DMXSerial.read(dmxAddr + shutOffset) > 127)
  {
    shutter.startRotate(120);
  }
  else
  {
    shutter.startRotate(-120);
  }

  //------------------------------------
  // Handle Lamp
  //------------------------------------

  if(DMXSerial.read(dmxAddr + lampOffset) > 200)
  {
    // turn HID lamp on
    digitalWrite(lampOn, HIGH);
  }
  else
  {
    // turn lamp off with hysteresis
    if(DMXSerial.read(dmxAddr + lampOffset) < 170)
    {
      digitalWrite(lampOn, LOW);
    }
  }

  //------------------------------------
  // Misc stuff, LED blinking
  //------------------------------------

  // evenly divisible by 50 -> every second, i hope
  if(loopcnt % 50 == 0)
  {
    digitalWrite(LED, !digitalRead(LED));
  }

  // change DMX address if button is pushed
  if(digitalRead(setDMXBtn) == LOW)
  {
    uint16_t highestAddr = 0;
    uint8_t highestVal = 0;

    // search for highest channel
    for (uint16_t i = 0; i < 511; i++)
    {
      if(DMXSerial.read(i) > highestVal)
      {
        highestVal = DMXSerial.read(i);
        highestAddr = i;
      }
    }

    // write channel to variable
    dmxAddr = highestAddr;
    
    // write variable to EEPROM for next boot
    EEPROM.put(0, dmxAddr);
  }

  // update everything every 20ms only, should be fine
  delay(20);
  loopcnt++;
}