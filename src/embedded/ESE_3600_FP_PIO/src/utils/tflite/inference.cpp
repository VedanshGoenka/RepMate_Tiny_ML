#include "inference.h"

// Define the label variables that were declared extern in the header
const int label_count = 5;
const char *labels[5] = {"l_i", "o_a", "p_f", "p_m", "s_w"};

// Buffer to store IMU data
CircularBuffer<TimeSeriesDataPoint> dataBuffer;

// TensorFlow Lite globals
namespace
{
  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *interpreter = nullptr;

  // All Ops Resolver
  tflite::AllOpsResolver resolver;

  // Define memory for input, output, and intermediate tensors
  // kTensor Area size was too small, was originally 10 x 1024, making it larger
  constexpr int kTensorArenaSize = 128 * 1024; // Adjust this as per the model's memory requirement
  uint8_t tensor_arena[kTensorArenaSize];
}

void setupModel(bool verbose = false)
{
  // Initialize the error reporter
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model
  model = tflite::GetModel(g_rep_mate_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Set up the interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory for the model's tensors
  if (interpreter->AllocateTensors() != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Failed to allocate tensors.");
    return;
  }

  // Check the model's inputs and outputs
  if (interpreter->inputs().size() != 1)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Model expects 1 input tensor, but got %zu.", interpreter->inputs().size());
    return;
  }

  // Check input dimensions
  if (interpreter->input(0)->dims->data[1] != OUTPUT_SEQUENCE_LENGTH)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Input tensor expects %d samples, but got %d.",
                         OUTPUT_SEQUENCE_LENGTH,
                         interpreter->input(0)->dims->data[1]);
    return;
  }

  if (interpreter->input(0)->dims->data[2] != NUM_FEATURES)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Input tensor expects %d features per sample, but got %d.",
                         NUM_FEATURES,
                         interpreter->input(0)->dims->data[2]);
    return;
  }

  if (interpreter->output(0)->dims->data[1] != label_count)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Output tensor expects %d labels, but got %d.", label_count, interpreter->output(0)->dims->data[1]);
    return;
  }

  // Print model details if verbose mode is enabled
  printModelDetails(verbose);

  TF_LITE_REPORT_ERROR(error_reporter, "Model setup complete.");
}

void doInference()
{
  TfLiteTensor *input = interpreter->input(0);

  if (dataBuffer.size() < GRAB_LEN)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Insufficient data in buffer for inference. Need %d samples, but only have %d.",
                         GRAB_LEN, dataBuffer.size());
    return;
  }

  printf("Preprocessing data\n");

  preprocess_buffer_to_input(dataBuffer, input->data.int8);

  printf("Invoking inference\n");

  // Run inference
  if (interpreter->Invoke() != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Inference invocation failed.");
    return;
  }

  printf("Inference complete\n");

  // Get the output tensor and print the results
  TfLiteTensor *output = interpreter->output(0);
  printf("Inference output: ");
  for (int i = 0; i < output->dims->data[1]; ++i)
  { // Assuming output shape [1, 5]
    printf("%f ", output->data.f[i]);
  }
  printf("\n");
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