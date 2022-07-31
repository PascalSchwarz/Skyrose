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

//------------------------------------
// Globals and instances of stuff
//------------------------------------

// init of stepper
A4988 stepper(MOTOR_STEPS, DIR, STEP);

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
  DMXSerial.init(DMXReceiver);

  pinMode(LED, OUTPUT);
  pinMode(lampOn, OUTPUT);
  pinMode(setDMXBtn, INPUT_PULLUP);

  pinMode(endStopL, INPUT_PULLUP);
  pinMode(endStopR, INPUT_PULLUP);
}

//------------------------------------
// Run loop until heatdeath of universe
//------------------------------------

void loop() {
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
  if(digitalRead(endStopR) == 0 and azimut.getDirection() == L298N::BACKWARD)
  {
    // we hit the endstop, reverse motor
    azimut.forward();
  }
  if(digitalRead(endStopL) == 0 and azimut.getDirection() == L298N::FORWARD)
  {
    // hit the other endstop, reverse again!
    azimut.backward();
  }

  //------------------------------------
  // Handle shutter
  //------------------------------------

  // update everything every 20ms only, should be fine
  delay(20);
}