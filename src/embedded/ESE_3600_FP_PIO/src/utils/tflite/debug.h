#pragma once

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include <cstdint>
#include "pre_process.h" // For OUTPUT_SEQUENCE_LENGTH and NUM_FEATURES

// Debug functions for model inspection
void inspectModel(const tflite::Model *model, tflite::ErrorReporter *error_reporter);
void inspectModelData(const uint8_t *model_data, size_t model_size);
void inspectTensorDetails(tflite::MicroInterpreter *interpreter, tflite::ErrorReporter *error_reporter);
void inspectInputShape(TfLiteTensor *input, tflite::ErrorReporter *error_reporter);
void inspectInputStats(TfLiteTensor *input);
void inspectOutputValues(TfLiteTensor *output, const char *labels[], int label_count);