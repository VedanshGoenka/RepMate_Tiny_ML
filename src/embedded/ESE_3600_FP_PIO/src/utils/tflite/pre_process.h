#pragma once

#include <cmath>

#include "buffer.h"
#include "data.h"

// Constants for preprocessing
constexpr size_t GRAB_LEN = 1000;
constexpr size_t OUTPUT_SEQUENCE_LENGTH = 200;
constexpr size_t AVERAGING_WINDOW = GRAB_LEN / OUTPUT_SEQUENCE_LENGTH;

static void window_avg(TimeSeriesDataPoint *recent_data, float *output_tensor_arr);

static void force_input_tensor_to_data(float *input_tensor_arr, 
                                     float data_2d_array[OUTPUT_SEQUENCE_LENGTH][NUM_FEATURES]);

void preprocess_buffer_to_input(const CircularBuffer<TimeSeriesDataPoint> &buffer, float *output_tensor_arr);
