#ifndef INFERENCE_H_
#define INFERENCE_H_

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Functions to initialize and run inference
void setupModel();
void runInference(const float* input_data, int input_length, float* output_data, int output_length);

#endif