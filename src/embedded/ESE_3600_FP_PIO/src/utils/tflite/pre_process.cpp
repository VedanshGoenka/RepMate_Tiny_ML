#include "pre_process.h"
#include <float.h>
#include <esp32-hal-psram.h>
#include "inference.h"

// Buffer pointers instead of static arrays
static float (*temp_buffer)[GRAB_LEN] = nullptr;
static float (*averaged_buffer)[OUTPUT_SEQUENCE_LENGTH] = nullptr;
static float (*feature_ranges)[2] = nullptr; // [0][0] = accel min, [0][1] = accel max, [1][0] = gyro min, [1][1] = gyro max

static void hardcode_feature_ranges()
{
  feature_ranges[0][0] = -25.09375;
  feature_ranges[0][1] = 30.8825;
  feature_ranges[1][0] = -8.54875;
  feature_ranges[1][1] = 7.995;
}

// Initialize buffers
static bool init_buffers()
{
  try
  {
    if (!temp_buffer)
    {
      if (psramFound())
      {
        temp_buffer = (float(*)[GRAB_LEN])ps_malloc(NUM_FEATURES * GRAB_LEN * sizeof(float));
      }
      else
      {
        temp_buffer = new float[NUM_FEATURES][GRAB_LEN];
      }
    }
    if (!averaged_buffer)
    {
      if (psramFound())
      {
        averaged_buffer = (float(*)[OUTPUT_SEQUENCE_LENGTH])ps_malloc(NUM_FEATURES * OUTPUT_SEQUENCE_LENGTH * sizeof(float));
      }
      else
      {
        averaged_buffer = new float[NUM_FEATURES][OUTPUT_SEQUENCE_LENGTH];
      }
    }
    if (!feature_ranges)
    {
      feature_ranges = new float[2][2];
      hardcode_feature_ranges();
    }
    return true;
  }
  catch (const std::bad_alloc &e)
  {
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
static void window_avg(size_t feature_idx)
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

  for (size_t i = 1; i < OUTPUT_SEQUENCE_LENGTH; i++)
  {
    sum = 0;
    for (size_t j = 0; j < window_size; j++)
    {
      size_t idx = i * window_size + j;
      sum += temp_buffer[feature_idx][idx];
    }
    averaged_buffer[feature_idx][i] = sum / window_size;
  }
}

// Normalizes the averaged data to the output buffer
static void normalize(size_t feature_idx, float *output_buffer)
{
  bool is_gyro = feature_idx >= 3;
  size_t range_idx = is_gyro ? 1 : 0;

  // Load min/max for this feature
  float min_val = feature_ranges[range_idx][0];
  float max_val = feature_ranges[range_idx][1];

  // Prevent division by zero and ensure proper scaling
  const float eps = 1e-6f;
  float range = std::max(max_val - min_val, eps);

  // Normalize to [0,1] range with clipping
  for (size_t i = 0; i < OUTPUT_SEQUENCE_LENGTH; i++)
  {
    float val = averaged_buffer[feature_idx][i];
    // Clip to min/max range
    val = std::max(min_val, std::min(max_val, val));
    // Normalize
    output_buffer[feature_idx * OUTPUT_SEQUENCE_LENGTH + i] =
        (val - min_val) / range;
  }

  if (DEBUG_OUTPUT)
  {
    printf("Feature %zu range: [%f, %f], normalized sample: %f\n",
           feature_idx, min_val, max_val,
           output_buffer[feature_idx * OUTPUT_SEQUENCE_LENGTH]);
  }
}

static void inspect_output_buffer(float *output_buffer)
{
  printf("Output buffer: ");
  for (size_t i = 0; i < NUM_FEATURES * OUTPUT_SEQUENCE_LENGTH; i++)
  {
    printf("%f ", output_buffer[i]); // Changed %d to %f for float output
    if ((i + 1) % OUTPUT_SEQUENCE_LENGTH == 0)
      printf("\n");
  }
  printf("\n");
}

// Preprocesses the buffer to the input
void preprocess_buffer_to_input(const CircularBuffer<TimeSeriesDataPoint> &buffer,
                                float *output_buffer)
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
    window_avg(feature);
  }

  printf("Normalizing and quantizing\n");
  for (size_t feature = 0; feature < NUM_FEATURES; feature++)
  {
    normalize(feature, output_buffer);
  }

  // inspect_output_buffer(output_buffer);

  // Cleanup
  delete[] recent_data;
  cleanup_buffers();

  printf("Preprocessing complete\n");
}
