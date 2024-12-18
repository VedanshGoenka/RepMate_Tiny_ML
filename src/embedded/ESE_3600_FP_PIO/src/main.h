#pragma once

#include <Arduino.h>
#include "utils/data_ops/data_collection.h"
#include "utils/data_ops/copy_files.h"
#include "utils/tflite/inference.h"
#include "utils/tflite/imu_provider.h"
#include "utils/hardware/ble.h"
#include "utils/hardware/buzzer.h"

#include "utils/tflite/pre_process.h"

// Setup Flags
extern const bool copy_files;
extern const bool force_reformat;

// Data Collection Constants
extern const String lift_names[3];
extern String current_lift;

// Buffer Constants
extern const char buffer_type;

// Define the buffer using the selected type
extern float dataBuffer[];

// Setup and Loop functions
void setup();
void loop();
