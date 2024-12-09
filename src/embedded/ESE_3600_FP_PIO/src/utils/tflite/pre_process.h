#pragma once

#include <cmath>

#include "circular_buffer.h"
#include "flat_buffer.h"

#include "data.h"

// Constants for preprocessing
constexpr size_t GRAB_LEN = 1000;
constexpr size_t OUTPUT_SEQUENCE_LENGTH = 200;
constexpr size_t AVERAGING_WINDOW = GRAB_LEN / OUTPUT_SEQUENCE_LENGTH;

static void window_avg(TimeSeriesDataPoint *recent_data, float *output_tensor_arr);

static void force_input_tensor_to_data(float *input_tensor_arr,
                                       float data_2d_array[OUTPUT_SEQUENCE_LENGTH][NUM_FEATURES]);

template <template <typename> class T>
void preprocess_buffer_to_input(const T<TimeSeriesDataPoint> &buffer, float *input_tensor_arr);