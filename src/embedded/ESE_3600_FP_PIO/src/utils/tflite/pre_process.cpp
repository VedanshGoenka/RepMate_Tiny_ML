#include "pre_process.h"

// Buffer definitions
static float temp_buffer[NUM_FEATURES][GRAB_LEN];
static float averaged_buffer[NUM_FEATURES][OUTPUT_SEQUENCE_LENGTH];
static float feature_ranges[NUM_FEATURES][2]; // [][0] = min, [][1] = max

// Copies sensor data to the temporary buffer
static void copy_to_temp_buffer(const TimeSeriesDataPoint *recent_data)
{
  for (size_t i = 0; i < GRAB_LEN; i++)
  {
    temp_buffer[0][i] = recent_data[i].aX;
    temp_buffer[1][i] = recent_data[i].aY;
    temp_buffer[2][i] = recent_data[i].aZ;
    temp_buffer[3][i] = recent_data[i].gX;
    temp_buffer[4][i] = recent_data[i].gY;
    temp_buffer[5][i] = recent_data[i].gZ;
  }
}

// Applies a moving average to the sensor data and tracks the range to feature_ranges
static void window_avg_find_range(size_t feature_idx)
{
  const size_t window_size = AVERAGING_WINDOW;

  // Initialize min/max with first averaged value
  float sum = 0;
  for (size_t j = 0; j < window_size && j < GRAB_LEN; j++)
  {
    sum += temp_buffer[feature_idx][j];
  }
  averaged_buffer[feature_idx][0] = sum / window_size;
  feature_ranges[feature_idx][0] = feature_ranges[feature_idx][1] = averaged_buffer[feature_idx][0];

  // Process remaining values while tracking min/max
  for (size_t i = 1; i < OUTPUT_SEQUENCE_LENGTH; i++)
  {
    sum = 0;
    for (size_t j = 0; j < window_size; j++)
    {
      size_t idx = i * window_size + j;
      sum += temp_buffer[feature_idx][idx];
    }
    averaged_buffer[feature_idx][i] = sum / window_size;

    feature_ranges[feature_idx][0] = std::min<float>(feature_ranges[feature_idx][0], averaged_buffer[feature_idx][i]);
    feature_ranges[feature_idx][1] = std::max<float>(feature_ranges[feature_idx][1], averaged_buffer[feature_idx][i]);
  }
}

// Normalizes and quantizes the averaged data to the output buffer
static void normalize_and_quantize(size_t feature_idx, int8_t *output_buffer)
{
  float min_val = feature_ranges[feature_idx][0];
  float max_val = feature_ranges[feature_idx][1];
  float scale = (OUTPUT_MAX - OUTPUT_MIN) / (max_val - min_val);

  for (size_t i = 0; i < OUTPUT_SEQUENCE_LENGTH; i++)
  {
    float val = ((averaged_buffer[feature_idx][i] - min_val) * scale) + OUTPUT_MIN;
    output_buffer[feature_idx * OUTPUT_SEQUENCE_LENGTH + i] =
        static_cast<int8_t>(std::min<float>(std::max<float>(
                                                static_cast<float>(OUTPUT_MIN), val),
                                            static_cast<float>(OUTPUT_MAX)));
  }
}

// Preprocesses the buffer to the input
void preprocess_buffer_to_input(const CircularBuffer<TimeSeriesDataPoint> &buffer,
                                int8_t *output_buffer)
{
  TimeSeriesDataPoint recent_data[GRAB_LEN];
  buffer.getRecent(GRAB_LEN, recent_data);

  copy_to_temp_buffer(recent_data);

  for (size_t feature = 0; feature < NUM_FEATURES; feature++)
  {
    window_avg_find_range(feature);
    normalize_and_quantize(feature, output_buffer);
  }
}
