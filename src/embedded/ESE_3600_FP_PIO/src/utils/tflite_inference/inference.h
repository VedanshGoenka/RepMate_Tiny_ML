#pragma once

#ifndef INFERENCE_H_
#define INFERENCE_H_

#include "inference_model_data.h"

#include <TensorFlowLite_ESP32.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <vector>

struct DataPoint {
    unsigned long timestamp;
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};

std::vector<DataPoint> dataBuffer;
long duration = 5000;

// Functions to initialize and run inference
void setupModel();
void runInference(const float* input_data, int input_length, float* output_data, int output_length);
void pollSetup();
void pollLoop();
void addDataToBuffer(unsigned long timestamp, float ax, float ay, float az, float gx, float gy, float gz);

#endif