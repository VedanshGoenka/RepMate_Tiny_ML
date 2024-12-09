#pragma once

#include <cmath>

#include "data.h"
#include "imu_provider.h"

// Constants for preprocessing
constexpr size_t GRAB_LEN = 1000;
constexpr size_t OUTPUT_SEQUENCE_LENGTH = 200;
constexpr size_t AVERAGING_WINDOW = GRAB_LEN / OUTPUT_SEQUENCE_LENGTH;

extern const int NUM_FEATURES;
extern const int BUFFER_LEN;

static void window_avg(float *recent_data, float *output_tensor_arr);

static void force_input_tensor_to_data(float *input_tensor_arr,
                                       float data_2d_array[OUTPUT_SEQUENCE_LENGTH][NUM_FEATURES]);

void preprocess_buffer_to_input(float buffer[], float *input_tensor_arr);
