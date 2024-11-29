#pragma once

#ifndef INFERENCE_H_
#define INFERENCE_H_

#include "model.h"
#include "buffer.h"
#include "pre_process.h"

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

// Buffer to store IMU data
extern CircularBuffer<TimeSeriesDataPoint> dataBuffer;

// Functions to initialize and run inference
const char *getTfLiteTypeName(TfLiteType type);
void printModelDetails(bool shouldPrint);
void setupModel(bool verbose);
void doInference();
void addDataToBuffer(unsigned long timestamp, float ax, float ay, float az, float gx, float gy, float gz);
void setupOutputLights();
void outputLights(int index);

#endif