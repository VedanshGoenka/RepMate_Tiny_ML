#pragma once

#ifndef INFERENCE_H_
#define INFERENCE_H_

#include "model.h"
#include "buffer.h"

#include <TensorFlowLite_ESP32.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include <Adafruit_Sensor.h>
#include <Wire.h>

// Define label count and labels
extern const int label_count;
extern const char *labels[];

// Functions to initialize and run inference
const char *getTfLiteTypeName(TfLiteType type);
void printModelDetails(bool shouldPrint);
void setupModel(bool verbose);
void runInference(const int8_t *input_data, int input_length, int8_t *output_data, int output_length);
void runInference();
void addDataToBuffer(unsigned long timestamp, float ax, float ay, float az, float gx, float gy, float gz);

#endif