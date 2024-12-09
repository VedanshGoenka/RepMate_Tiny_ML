#include "pre_process.h"
#include <float.h>

// Applies a moving average to the sensor data and tracks the range to feature_ranges
static void window_avg(TimeSeriesDataPoint *recent_data, float *input_tensor_arr)
{
  const size_t window_size = AVERAGING_WINDOW;

  // Validate buffer size
  if (OUTPUT_SEQUENCE_LENGTH * window_size > GRAB_LEN)
  {
    printf("Error: Required buffer size (%zu) exceeds GRAB_LEN (%zu)\n",
           OUTPUT_SEQUENCE_LENGTH * window_size, GRAB_LEN);
    return;
  }

  // Process each time step
  for (size_t i = 0; i < OUTPUT_SEQUENCE_LENGTH; i++)
  {
    // For each time step, compute averages for all features
    for (size_t feature = 0; feature < NUM_FEATURES; feature++)
    {
      float sum = 0;
      for (size_t j = 0; j < window_size; j++)
      {
        float value;
        switch (feature)
        {
        case 0:
          value = recent_data[i * window_size + j].aX;
          break;
        case 1:
          value = recent_data[i * window_size + j].aY;
          break;
        case 2:
          value = recent_data[i * window_size + j].aZ;
          break;
        case 3:
          value = recent_data[i * window_size + j].gX;
          break;
        case 4:
          value = recent_data[i * window_size + j].gY;
          break;
        case 5:
          value = recent_data[i * window_size + j].gZ;
          break;
        default:
          throw std::runtime_error("Invalid feature index");
        }
        sum += value;
      }
      // Store in interleaved format: (sample_idx * NUM_FEATURES + feature_idx)
      input_tensor_arr[i * NUM_FEATURES + feature] = sum / window_size;
    }
  }
}

static void inspect_output_buffer(float *input_tensor_arr)
{
  printf("Output buffer: ");
  for (size_t i = 0; i < NUM_FEATURES * OUTPUT_SEQUENCE_LENGTH; i++)
  {
    if (i % NUM_FEATURES == 0)
      printf("[");
    printf("%f ", input_tensor_arr[i]);
    if ((i + 1) % NUM_FEATURES == 0)
      printf("],\n");
  }
  printf("\n");
}

// Preprocesses the buffer to the input
void preprocess_buffer_to_input(const CircularBuffer<TimeSeriesDataPoint> &buffer,
                                float *input_tensor_arr)
{
  // // Allocate recent_data on heap
  // TimeSeriesDataPoint *recent_data = new TimeSeriesDataPoint[GRAB_LEN];
  // if (!recent_data)
  // {
  //   printf("Failed to allocate recent_data buffer\n");
  //   return;
  // }

  // printf("Getting recent data\n");
  // buffer.getRecent(GRAB_LEN, recent_data);

  // printf("Window averaging\n");
  // window_avg(recent_data, input_tensor_arr);

  // data_2d_lift_instability
  // data_2d_no_lift
  // data_2d_off_axis
  // data_2d_partial_motion
  // data_2d_perfect_form
  // data_2d_swinging_weight

  printf("Force input tensor to data class: off_axis\n");
  force_input_tensor_to_data(input_tensor_arr, data_2d_off_axis);

  // inspect_output_buffer(input_tensor_arr);

  // Cleanup
  // delete[] recent_data;

  printf("Preprocessing complete\n");
}

static void force_input_tensor_to_data(float *input_tensor_arr, float data_2d_array[OUTPUT_SEQUENCE_LENGTH][NUM_FEATURES])
{
  for (size_t i = 0; i < OUTPUT_SEQUENCE_LENGTH; i++)
  {
    for (size_t j = 0; j < NUM_FEATURES; j++)
    {
      input_tensor_arr[i * NUM_FEATURES + j] = data_2d_array[i][j];
    }
  }
}
