#pragma once

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>


// Pins to monitor
const int pins[] = {D0, D1, D2, D3, D6};
const int numPins = sizeof(pins) / sizeof(pins[0]);

// Functions
void data_collection_setup();
void data_collection_loop();
void recordData(int triggeredPin);
