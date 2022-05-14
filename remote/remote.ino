// Copyright 2022 Matt Aaron
#include <SPI.h>
#include "RF24.h"
#include <Wire.h>

const int MPU_ADDR = 0x68;
float xa, ya, za, roll, pitch;

struct Controls {
  int8_t drive;
  int8_t steer;
};

RF24 radio(7, 8);
uint8_t address[6] = {"00001"};
Controls payload;

void setup() {
  Serial.begin(115200);

  if (!radio.begin()) {
    Serial.println("Radio failure");
    while (1) {} // hold in infinite loop
  } else {
    Serial.println("Radio OK");
  }

  // Configure radio
  radio.setPALevel(RF24_PA_MAX);
  radio.setPayloadSize(sizeof(payload));
  radio.openWritingPipe(address);
  radio.stopListening();  // put radio in TX mode

  // Initialize accelerometer
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}

void loop() {
  // Read data from accelerometer and calculate tilt
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);
  int t = Wire.read();
  xa = (t << 8) | Wire.read();
  t = Wire.read();
  ya = (t << 8) | Wire.read();
  t = Wire.read();
  za = (t << 8) | Wire.read();
  // Formula from https://wiki.dfrobot.com/How_to_Use_a_Three-Axis_Accelerometer_for_Tilt_Sensing
  roll = atan2(ya , za) * 180.0 / PI;
  pitch = atan2(-xa , sqrt(ya * ya + za * za)) * 180.0 / PI;  // account for roll already applied

  // Set the payload values to be sent to the car
  if (roll > 45) {
    payload.steer = 127;
  } else if (roll < -45) {
    payload.steer = -127;
  } else {
    payload.steer = 0;
  }

  if (pitch > 45) {
    payload.drive = 100;
  } else if (pitch < -45) {
    payload.drive = -100;
  } else {
    payload.drive = 0;
  }


  radio.write(&payload, sizeof(payload));
  delay(2);
}
