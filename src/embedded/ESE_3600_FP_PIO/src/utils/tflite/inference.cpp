#include "inference.h"
#include "debug.h"

// Define the label variables that were declared extern in the header
const int label_count = 6;
const char *labels[label_count] = {"l_i", "n_l", "o_a", "p_f", "p_m", "s_w"};

const bool DEBUG_OUTPUT = false;

// Buffer to store IMU data
CircularBuffer<TimeSeriesDataPoint> dataBuffer;

// TensorFlow Lite globals
namespace
{
  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *interpreter = nullptr;
  tflite::AllOpsResolver resolver;
  constexpr int kTensorArenaSize = 128 * 1024;
  static uint8_t* tensor_arena = nullptr;
  static uint8_t* model_buffer = nullptr;
}

void setupModel(bool verbose = false)
{
  // Initialize the error reporter
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Load model into PSRAM
  if (!model) {
    if (!psramFound()) {
      TF_LITE_REPORT_ERROR(error_reporter, "PSRAM not found!");
      return;
    }
    
    size_t model_size = g_rep_mate_model_data_len;
    TF_LITE_REPORT_ERROR(error_reporter, "Model size: %d bytes", (int)model_size);
    
    // Debug model data
    // inspectModelData(g_rep_mate_model_data, model_size);
    
    model_buffer = (uint8_t*)ps_malloc(model_size);
    if (!model_buffer) {
      TF_LITE_REPORT_ERROR(error_reporter, "Failed to allocate model buffer!");
      return;
    }

    memcpy(model_buffer, g_rep_mate_model_data, model_size);
    model = tflite::GetModel(model_buffer);
    
    if (!model) {
      TF_LITE_REPORT_ERROR(error_reporter, "Failed to get model from buffer");
      return;
    }

    // inspectModel(model, error_reporter);
  }

  // Allocate tensor arena
  if (!tensor_arena) {
    tensor_arena = (uint8_t*)ps_malloc(kTensorArenaSize);
    if (!tensor_arena) {
      TF_LITE_REPORT_ERROR(error_reporter, "Failed to allocate tensor arena!");
      return;
    }
  }

  // Set up the interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate tensors
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Failed to allocate tensors!");
    return;
  }

  // Debug tensor details
  // inspectTensorDetails(interpreter, error_reporter);

  TF_LITE_REPORT_ERROR(error_reporter, "Model setup complete");
  setupOutputLights();
}

void doInference()
{
  TfLiteTensor *input = interpreter->input(0);
  
  if (!input) {
    TF_LITE_REPORT_ERROR(error_reporter, "Failed to get input tensor");
    return;
  }

  // Verify input shape
  // inspectInputShape(input, error_reporter);

  try {
    // Preprocess data
    preprocess_buffer_to_input(dataBuffer, input->data.f);

    // Show input statistics
    if (DEBUG_OUTPUT) {
      inspectInputStats(input);
    }

    // Run inference
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
      return;
    }

    // Show output details
    TfLiteTensor* output = interpreter->output(0);
    if (DEBUG_OUTPUT && output) {
      inspectOutputValues(output, labels, label_count);
    }

    // Get inference result
    getInferenceResult();
  }
  catch (const std::exception& e) {
    TF_LITE_REPORT_ERROR(error_reporter, "Exception: %s", e.what());
  }
}

void getInferenceResult()
{
  TfLiteTensor *output = interpreter->output(0);

  if (DEBUG_OUTPUT)
  {
    printf("Raw logits: ");
    for (int i = 0; i < label_count; ++i)
    {
      printf("%f ", output->data.f[i]);
    }
    printf("\n");
  }

  float softmax_values[label_count];
  applySoftmax(output->data.f, -1, label_count, softmax_values);

  // Find the maximum probability and its index
  int max_index = 0;
  float max_prob = softmax_values[0];
  for (int i = 1; i < label_count; ++i)
  {
    if (softmax_values[i] > max_prob)
    {
      max_prob = softmax_values[i];
      max_index = i;
    }
  }

  printf("\nInference result: %s with confidence %.2f\n", labels[max_index], max_prob * 100);

  if (DEBUG_OUTPUT)
  {
    printf("Softmax probabilities: ");
    for (int i = 0; i < label_count; ++i)
    {
      printf("%.4f ", softmax_values[i]);
    }
    printf("\n");
  }
}

void applySoftmax(const float *output_values, int max_index, size_t label_count, float *softmax_values)
{
  // Convert to float and find maximum for numerical stability
  float max_val = output_values[0];
  for (int i = 0; i < label_count; i++)
  {
    if (output_values[i] > max_val)
    {
      max_val = output_values[i];
    }
  }

  // Calculate exp() for each value and sum
  float sum = 0.0f;
  for (int i = 0; i < label_count; i++)
  {
    softmax_values[i] = exp(output_values[i] - max_val);
    sum += softmax_values[i];
  }

  // Normalize
  for (int i = 0; i < label_count; i++)
  {
    softmax_values[i] /= sum;
  }
}

/////////////////////////
// Debugging functions //
/////////////////////////

// Helper function to convert TfLiteType to string
const char *getTfLiteTypeName(TfLiteType type)
{
  switch (type)
  {
  case kTfLiteFloat32:
    return "kTfLiteFloat32";
  case kTfLiteInt32:
    return "kTfLiteInt32";
  case kTfLiteUInt8:
    return "kTfLiteUInt8";
  case kTfLiteInt8:
    return "kTfLiteInt8";
  case kTfLiteInt64:
    return "kTfLiteInt64";
  default:
    return "Unknown";
  }
}

void printModelDetails(bool shouldPrint = false)
{
  if (!shouldPrint || !interpreter)
    return;

  TfLiteTensor *input = interpreter->input(0);
  TfLiteTensor *output = interpreter->output(0);

  printf("\n=== Model Details ===\n");

  // Input tensor details
  printf("Input Tensor:\n");
  printf("- Dimensions: [");
  for (int i = 0; i < input->dims->size; i++)
  {
    printf("%d%s", input->dims->data[i], (i < input->dims->size - 1) ? ", " : "");
  }
  printf("]\n");
  printf("- Type: %s\n", getTfLiteTypeName(input->type));
  printf("- Bytes: %zu\n", input->bytes);

  // Output tensor details
  printf("\nOutput Tensor:\n");
  printf("- Dimensions: [");
  for (int i = 0; i < output->dims->size; i++)
  {
    printf("%d%s", output->dims->data[i], (i < output->dims->size - 1) ? ", " : "");
  }
  printf("]\n");
  printf("- Type: %s\n", getTfLiteTypeName(output->type));
  printf("- Bytes: %zu\n", output->bytes);

  // Print labels
  printf("\nModel Labels:\n");
  for (int i = 0; i < label_count; i++)
  {
    printf("- %d: %s\n", i, labels[i]);
  }

  printf("\nTensor Arena Size: %d bytes\n", kTensorArenaSize);
  printf("===================\n\n");
}

void setupOutputLights()
{
  uint8_t pins[7] = {D0, D1, D2, D3, D6, D7, D8};
  for (int i = 0; i < 7; i++)
  {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}

void outputLights(int index)
{
  uint8_t pins[7] = {D0, D1, D2, D3, D6, D7, D8};
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(pins[i], LOW);
  }
  digitalWrite(pins[index], HIGH);
}