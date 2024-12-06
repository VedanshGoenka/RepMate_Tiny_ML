#include "pre_process.h"
#include <float.h>

// Buffer pointers instead of static arrays
static float (*temp_buffer)[GRAB_LEN] = nullptr;
static float (*averaged_buffer)[OUTPUT_SEQUENCE_LENGTH] = nullptr;
static float (*feature_ranges)[2] = nullptr; // [0][0] = accel min, [0][1] = accel max, [1][0] = gyro min, [1][1] = gyro max

// Initialize buffers
static bool init_buffers()
{
    try {
        if (!temp_buffer) {
            temp_buffer = new float[NUM_FEATURES][GRAB_LEN];
        }
        if (!averaged_buffer) {
            averaged_buffer = new float[NUM_FEATURES][OUTPUT_SEQUENCE_LENGTH];
        }
        if (!feature_ranges) {
            feature_ranges = new float[2][2];
            // Initialize ranges
            for (int i = 0; i < 2; i++) {
                feature_ranges[i][0] = FLT_MAX;   // min
                feature_ranges[i][1] = -FLT_MAX;  // max
            }
        }
        return true;
    }
    catch (const std::bad_alloc& e) {
        cleanup_buffers();
        printf("Failed to allocate preprocessing buffers: %s\n", e.what());
        return false;
    }
}

// Cleanup buffers
static void cleanup_buffers()
{
  delete[] temp_buffer;
  delete[] averaged_buffer;
  delete[] feature_ranges;
  temp_buffer = nullptr;
  averaged_buffer = nullptr;
  feature_ranges = nullptr;
}

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
  bool is_gyro = feature_idx >= 3;
  size_t range_idx = is_gyro ? 1 : 0;

  float sum = 0;
  for (size_t j = 0; j < window_size && j < GRAB_LEN; j++)
  {
    sum += temp_buffer[feature_idx][j];
  }
  averaged_buffer[feature_idx][0] = sum / window_size;

  feature_ranges[range_idx][0] = std::min<float>(feature_ranges[range_idx][0], averaged_buffer[feature_idx][0]);
  feature_ranges[range_idx][1] = std::max<float>(feature_ranges[range_idx][1], averaged_buffer[feature_idx][0]);

  for (size_t i = 1; i < OUTPUT_SEQUENCE_LENGTH; i++)
  {
    sum = 0;
    for (size_t j = 0; j < window_size; j++)
    {
      size_t idx = i * window_size + j;
      sum += temp_buffer[feature_idx][idx];
    }
    averaged_buffer[feature_idx][i] = sum / window_size;

    feature_ranges[range_idx][0] = std::min<float>(feature_ranges[range_idx][0], averaged_buffer[feature_idx][i]);
    feature_ranges[range_idx][1] = std::max<float>(feature_ranges[range_idx][1], averaged_buffer[feature_idx][i]);
  }

//   // Add debug output at the end
//   printf("Feature %zu range: [%f, %f]\n", 
//          feature_idx, 
//          feature_ranges[range_idx][0], 
//          feature_ranges[range_idx][1]);
}

// Normalizes and quantizes the averaged data to the output buffer
static void normalize_and_quantize(size_t feature_idx, int8_t *output_buffer)
{
    // Find min/max for this specific feature
    float min_val = FLT_MAX;
    float max_val = -FLT_MAX;
    
    for (size_t i = 0; i < OUTPUT_SEQUENCE_LENGTH; i++) {
        float value = averaged_buffer[feature_idx][i];
        min_val = std::min(min_val, value);
        max_val = std::max(max_val, value);
    }

    // Print raw values for debugging
    // printf("Feature %zu raw values: ", feature_idx);
    // for (int i = 0; i < 5; i++) {
    //     printf("%f ", averaged_buffer[feature_idx][i]);
    // }
    // printf("\n");

    // First normalize to [0,1]
    const float eps = 1e-7f;
    float range = std::max(max_val - min_val, eps);

    // Get quantization parameters from model
    const float input_scale = 0.003921568859368563f;
    const int input_zero_point = -128;

    for (size_t i = 0; i < OUTPUT_SEQUENCE_LENGTH; i++)
    {
        // First normalize to [0,1]
        float value = averaged_buffer[feature_idx][i];
        float normalized = (value - min_val) / range;
        
        // Apply model's quantization formula:
        // quantized = data / input_scale + input_zero_point
        float quantized = normalized / input_scale + input_zero_point;
        
        // Clamp to int8 range
        quantized = std::min(std::max(quantized, -128.0f), 127.0f);
        
        output_buffer[feature_idx * OUTPUT_SEQUENCE_LENGTH + i] = 
            static_cast<int8_t>(std::round(quantized));
    }

    // // Debug output
    // printf("Feature %zu quantized samples: ", feature_idx);
    // for (int i = 0; i < 5; i++) {
    //     printf("%d ", output_buffer[feature_idx * OUTPUT_SEQUENCE_LENGTH + i]);
    // }
    // printf("\n");
}

static void inspect_output_buffer(int8_t *output_buffer)
{
  printf("Output buffer: ");
  for (size_t i = 0; i < NUM_FEATURES * OUTPUT_SEQUENCE_LENGTH; i++)
  {
    printf("%d ", output_buffer[i]);
    if ((i + 1) % OUTPUT_SEQUENCE_LENGTH == 0)
      printf("\n");
  }
  printf("\n");
}

// Preprocesses the buffer to the input
void preprocess_buffer_to_input(const CircularBuffer<TimeSeriesDataPoint> &buffer,
                                int8_t *output_buffer)
{
  // Initialize buffers
  if (!init_buffers())
  {
    printf("Failed to allocate preprocessing buffers\n");
    return;
  }

  // Allocate recent_data on heap
  TimeSeriesDataPoint *recent_data = new (std::nothrow) TimeSeriesDataPoint[GRAB_LEN];
  if (!recent_data)
  {
    printf("Failed to allocate recent_data buffer\n");
    cleanup_buffers();
    return;
  }

  printf("Getting recent data\n");
  buffer.getRecent(GRAB_LEN, recent_data);

  printf("Copying to temp buffer\n");
  copy_to_temp_buffer(recent_data);

  printf("Window averaging\n");
  for (size_t feature = 0; feature < NUM_FEATURES; feature++)
  {
    window_avg_find_range(feature);
  }

  printf("Normalizing and quantizing\n");
  for (size_t feature = 0; feature < NUM_FEATURES; feature++)
  {
    normalize_and_quantize(feature, output_buffer);
  }

  // inspect_output_buffer(output_buffer);

  // Cleanup
  delete[] recent_data;
  cleanup_buffers();

  printf("Preprocessing complete\n");
}
