#include "pre_process.h"
#include <float.h>

// Applies a moving average to the sensor data and tracks the range to feature_ranges
static void window_avg(float *buffer, float *input_tensor_arr)
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
    // For each feature
    for (size_t feature = 0; feature < NUM_FEATURES; feature++)
    {
      float sum = 0;
      // Calculate start position for this window
      size_t window_start = i * window_size * NUM_FEATURES;
      
      // Sum up window_size samples for this feature
      for (size_t j = 0; j < window_size; j++)
      {
        // Access pattern: window_start + (sample_offset * NUM_FEATURES) + feature_idx
        sum += buffer[window_start + (j * NUM_FEATURES) + feature];
      }
      
      // Store averaged result
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
void preprocess_buffer_to_input(float buffer[], float *input_tensor_arr)
{
  // Allocate recent_data on heap
  float *recent_data = new float[GRAB_LEN];
  if (!recent_data)
  {
    printf("Failed to allocate recent_data buffer\n");
    return;
  }

  printf("Window averaging\n");
  window_avg(buffer, input_tensor_arr);

  // DEBUG //

  // data_2d_lift_instability
  // data_2d_no_lift
  // data_2d_off_axis
  // data_2d_partial_motion
  // data_2d_perfect_form
  // data_2d_swinging_weight

  // printf("Force input tensor to data class: off_axis\n");
  // force_input_tensor_to_data(input_tensor_arr, data_2d_off_axis);

  // inspect_output_buffer(input_tensor_arr);

  // DEBUG //

  // Cleanup
  delete[] recent_data;

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