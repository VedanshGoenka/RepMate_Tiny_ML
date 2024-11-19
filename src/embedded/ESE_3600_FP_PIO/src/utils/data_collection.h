#pragma once

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <map>

#include "file_system.h"
#include "json_operations.h"

// Pins to monitor
const uint8_t numPins = 5;
extern const uint8_t pins[5];

// Output to JSON
extern bool output_to_json;

// Lift Names
extern const String lift_names[3];
extern String current_lift;

// Duration of data collection
extern const unsigned long duration;
extern const unsigned long sampling_rate;

// Map pins to lift classifications
extern const std::map<int, String> lift_classification_map;

// Functions
void data_collection_setup();
void data_collection_loop();
void recordData(int triggeredPin, bool to_json);
void printData(unsigned long time, int triggeredPin, sensors_event_t a, sensors_event_t g);
