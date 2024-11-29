#pragma once

#include "buffer.h"

// Constants for preprocessing
constexpr size_t GRAB_LEN = 1000;
constexpr size_t OUTPUT_SEQUENCE_LENGTH = 200;
constexpr size_t AVERAGING_WINDOW = GRAB_LEN / OUTPUT_SEQUENCE_LENGTH;
constexpr int8_t OUTPUT_MIN = -128;
constexpr int8_t OUTPUT_MAX = 127;

static void copy_to_temp_buffer(const TimeSeriesDataPoint *recent_data);

static void window_avg_find_range(size_t feature_idx);

static void normalize_and_quantize(size_t feature_idx, int8_t *output_buffer);

void preprocess_buffer_to_input(const CircularBuffer<TimeSeriesDataPoint> &buffer, int8_t *output_buffer);
