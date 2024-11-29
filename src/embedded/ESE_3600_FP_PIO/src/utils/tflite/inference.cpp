#include "inference.h"

// TensorFlow Lite globals
namespace {
  tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::AllOpsResolver resolver;
  tflite::MicroInterpreter* interpreter = nullptr;

  // Define memory for input, output, and intermediate tensors
  // kTensor Area size was too small, was originally 10 x 1024, making it larger
  constexpr int kTensorArenaSize = 100 * 1024;  // Adjust this as per the model's memory requirement
  uint8_t tensor_arena[kTensorArenaSize];

  std::vector<DataPoint> dataBuffer;
}

void setupModel() {
  // Initialize the error reporter
  error_reporter = &micro_error_reporter;

  // Map the model
  model = tflite::GetModel(g_rep_mate_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model schema mismatch.");
    return;
  }

  // Set up the interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory for the model's tensors
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    error_reporter->Report("AllocateTensors failed.");
    return;
  }

  error_reporter->Report("Model setup complete.");
}

void runInference(const int8_t* input_data, int input_length, int8_t* output_data, int output_length) {
  // Ensure enough data is present to match the input tensor's shape
  TfLiteTensor* input = interpreter->input(0);
  const int num_samples = input->dims->data[1]; // Expected number of samples (e.g., 200)
  const int features_per_sample = input->dims->data[2]; // Features per sample (e.g., 6)

  for (int i = 0; i < num_samples; ++i) {
    input->data.f[i * features_per_sample + 0] = input_data[i * 6 + 0];
    input->data.f[i * features_per_sample + 1] = input_data[i * 6 + 1];
    input->data.f[i * features_per_sample + 2] = input_data[i * 6 + 2];
    input->data.f[i * features_per_sample + 3] = input_data[i * 6 + 3];
    input->data.f[i * features_per_sample + 4] = input_data[i * 6 + 4];
    input->data.f[i * features_per_sample + 5] = input_data[i * 6 + 5];
  }

  // Run inference
  if (interpreter->Invoke() != kTfLiteOk) {
    error_reporter->Report("Invoke failed.");
    return;
  }

  // Get the output tensor and print the results
  TfLiteTensor* output = interpreter->output(0);
  printf("Inference output: ");
  for (int i = 0; i < output->dims->data[1]; ++i) { // Assuming output shape [1, 5]
    printf("%f ", output->data.f[i]);
  }
  printf("\n");
}

void runInference() {
  TfLiteTensor* input = interpreter->input(0);
  const int num_samples = input->dims->data[1]; // Expected number of samples (e.g., 200)
  const int features_per_sample = input->dims->data[2]; // Features per sample (e.g., 6)

  if (dataBuffer.size() < num_samples) {
    error_reporter->Report("Insufficient data in buffer for inference.");
    return;
  }

  // Copy data into the input tensor
  for (int i = 0; i < num_samples; ++i) {
    const DataPoint& dp = dataBuffer[i];
    input->data.f[i * features_per_sample + 0] = dp.accel_x;
    input->data.f[i * features_per_sample + 1] = dp.accel_y;
    input->data.f[i * features_per_sample + 2] = dp.accel_z;
    input->data.f[i * features_per_sample + 3] = dp.gyro_x;
    input->data.f[i * features_per_sample + 4] = dp.gyro_y;
    input->data.f[i * features_per_sample + 5] = dp.gyro_z;
  }

  // Run inference
  if (interpreter->Invoke() != kTfLiteOk) {
    error_reporter->Report("Invoke failed.");
    return;
  }

  // Get the output tensor and print the results
  TfLiteTensor* output = interpreter->output(0);
  printf("Inference output: ");
  for (int i = 0; i < output->dims->data[1]; ++i) { // Assuming output shape [1, 5]
    printf("%f ", output->data.f[i]);
  }
  printf("\n");
}

