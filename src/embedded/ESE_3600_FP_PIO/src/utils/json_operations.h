#pragma once

#include <ArduinoJson.h>
#include "file_system.h"

void setupJSON(const int pin);

void createJSONHeading(String lift_name, String lift_classification);

void addDataPoint(float timestamp, float accel_x, float accel_y, float accel_z, float gyro_x, float gyro_y, float gyro_z);

void closeJSONArray();

void closeDataFile();
