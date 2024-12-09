#pragma once

#ifndef INFERENCE_H_
#define INFERENCE_H_

#include "model.h"
#include "pre_process.h"
#include "main.h"

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
extern const char *full_label_classes[];

extern int current_lift_idx;

// Debug control
extern const bool DEBUG_OUTPUT;

// Buffer to store IMU data - update to use template type selection
extern float dataBuffer[];

// Core inference functions
void setupModel(bool verbose);
void doInference();
void getInferenceResult();

// Data processing functions
void addDataToBuffer(unsigned long timestamp, float ax, float ay, float az, float gx, float gy, float gz);
void applySoftmax(const float *output_values, size_t label_count, float *softmax_values);

// Output and visualization functions
void setupOutputLights();
void outputLights(int index);

// Debug and utility functions
const char *getTfLiteTypeName(TfLiteType type);
void printModelDetails(bool shouldPrint);

const char *getCurrentLiftName(int current_lift_idx);

#endif