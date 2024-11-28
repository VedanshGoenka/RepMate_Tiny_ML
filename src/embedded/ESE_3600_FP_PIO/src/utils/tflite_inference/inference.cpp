#include "inference.h"

Adafruit_MPU6050 mpu;

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

void runInference(const float* input_data, int input_length, float* output_data, int output_length) {
  // Get input tensor
  TfLiteTensor* input = interpreter->input(0);
  // Ensure input size matches
  if (input->dims->data[0] != input_length) {
    error_reporter->Report("Input size mismatch.");
    return;
  }

  // Copy input data
  for (int i = 0; i < input_length; ++i) {
    input->data.f[i] = input_data[i];
  }

  // Run inference
  if (interpreter->Invoke() != kTfLiteOk) {
    error_reporter->Report("Invoke failed.");
    return;
  }

  // Get output tensor and copy data to output array
  TfLiteTensor* output = interpreter->output(0);
  if (output->dims->data[0] != output_length) {
    error_reporter->Report("Output size mismatch.");
    return;
  }

  for (int i = 0; i < output_length; ++i) {
    output_data[i] = output->data.f[i];
  }
  printf("Inference complete.\n");
  printf("Output: %f\n", output_data[0]);
  dataBuffer.clear();
}

void pollSetup()
{
  // Initialize MPU6050
  while (!mpu.begin())
  {
    delay(500);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void pollLoop()
{
  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        // Add data to buffer
        addDataToBuffer(
            millis() - startTime,
            a.acceleration.x, a.acceleration.y, a.acceleration.z,
            g.gyro.x, g.gyro.y, g.gyro.z
        );
  }
}

void addDataToBuffer(unsigned long timestamp, float ax, float ay, float az, float gx, float gy, float gz) {
    DataPoint dp = {timestamp, ax, ay, az, gx, gy, gz};
    dataBuffer.push_back(dp);
}