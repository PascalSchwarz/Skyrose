#include <Arduino.h>
#include <DMXSerial.h>
#include <L298N.h>
#include "A4988.h"

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

// init of stepper
A4988 stepper(MOTOR_STEPS, DIR, STEP);

// Create azimut motor instance
L298N azimut(azEN, azIN1, azIN2);

// Create mirror motor instance
L298N mirror(miEN, miIN1, miIN2);

void setup() {
  // put your setup code here, to run once:
  DMXSerial.init(DMXReceiver);
}

void loop() {
  // put your main code here, to run repeatedly:
}