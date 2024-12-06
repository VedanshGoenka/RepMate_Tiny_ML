#include "inference.h"

// Define the label variables that were declared extern in the header
const int label_count = 6;
const char *labels[label_count] = {"l_i", "n_l", "o_a", "p_f", "p_m", "s_w"};

const bool DEBUG_OUTPUT = true;

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

  setupOutputLights();
}

void doInference()
{
    TfLiteTensor *input = interpreter->input(0);
    TfLiteTensor *output = interpreter->output(0);
    
    if (!input || !output) {
        TF_LITE_REPORT_ERROR(error_reporter, "Failed to get input/output tensors");
        return;
    }

    // Debug quantization parameters
    if (DEBUG_OUTPUT && false) {
        printf("Input quantization - scale: %f, zero_point: %d\n",
               input->params.scale, input->params.zero_point);
        printf("Output quantization - scale: %f, zero_point: %d\n",
               output->params.scale, output->params.zero_point);
    }

    if (dataBuffer.size() < GRAB_LEN) {
        TF_LITE_REPORT_ERROR(error_reporter,
            "Insufficient data in buffer for inference. Need %d samples, have %zu",
            GRAB_LEN, dataBuffer.size());
        return;
    }

    try {
        if (DEBUG_OUTPUT) {
            printf("Preprocessing data\n");
        }

        // Preprocess and quantize input
        preprocess_buffer_to_input(dataBuffer, input->data.int8);

        if (DEBUG_OUTPUT) {
            printf("Invoking inference\n");
        }

        // Run inference
        unsigned long start_time = millis();
        TfLiteStatus invoke_status = interpreter->Invoke();
        unsigned long inference_time = millis() - start_time;

        if (invoke_status != kTfLiteOk) {
            TF_LITE_REPORT_ERROR(error_reporter, 
                "Inference failed with status: %d", invoke_status);
            return;
        }

        if (DEBUG_OUTPUT) {
            printf("Inference completed in %lu ms\n", inference_time);
            
            // Print raw quantized output
            printf("Raw logits: ");
            for (int i = 0; i < 6; i++) {
                printf("%d ", output->data.int8[i]);
            }
            printf("\n");

            // Dequantize and compute softmax
            float dequantized[6];
            float softmax[6];
            float sum = 0.0f;
            
            // Dequantize: (raw - zero_point) * scale
            for (int i = 0; i < 6; i++) {
                dequantized[i] = (output->data.int8[i] - output->params.zero_point) 
                                * output->params.scale;
                float exp_val = exp(dequantized[i]);
                softmax[i] = exp_val;
                sum += exp_val;
            }

            // Find max probability and corresponding class
            int max_idx = 0;
            float max_prob = 0.0f;

            // Normalize softmax and find max
            printf("Class probabilities:\n");
            for (int i = 0; i < 6; i++) {
                softmax[i] /= sum;
                printf("%s: %.4f\n", labels[i], softmax[i]);
                if (softmax[i] > max_prob) {
                    max_prob = softmax[i];
                    max_idx = i;
                }
            }

            // Print final prediction
            printf("\nPredicted class: %s (confidence: %.2f%%)\n", 
                   labels[max_idx], max_prob * 100);

            printf("----------------------------------\n");
        }
    }
    catch (const std::exception& e) {
        TF_LITE_REPORT_ERROR(error_reporter, "Exception during inference: %s", e.what());
    }
}

void getInferenceResult()
{
  TfLiteTensor *output = interpreter->output(0);
  if (DEBUG_OUTPUT)
  {
    printf("Inference output: ");
  }

  int max_index = 0;
  int max_value = output->data.int8[0];

  int output_values[label_count];

  for (int i = 0; i < label_count; ++i)
  {
    output_values[i] = output->data.int8[i];
    if (DEBUG_OUTPUT)
    {
      printf("%d ", output_values[i]);
    }
    if (output_values[i] > max_value)
    {
      max_value = output_values[i];
      max_index = i;
    }
  }

  // Apply softmax to the output values
  float softmax_values[label_count];
  applySoftmax(output_values, max_index, label_count, softmax_values);

  printf("\n");
  printf("Inference result: %s with confidence %.2f\n", labels[max_index], softmax_values[max_index]);
  printf("\n");

  if (DEBUG_OUTPUT)
  {
    printf("\nRaw logits: ");
    for (int i = 0; i < label_count; ++i) {
      printf("%d ", output_values[i]);
    }
    printf("\nSoftmax probabilities: ");
    for (int i = 0; i < label_count; ++i) {
      printf("%.4f ", softmax_values[i]);
    }
    printf("\n");
  }
}

void applySoftmax(const int *output_values, int max_index, size_t label_count, float *softmax_values)
{
    // Convert to float and find maximum for numerical stability
    float max_val = output_values[0];
    for (int i = 0; i < label_count; i++) {
        if (output_values[i] > max_val) {
            max_val = output_values[i];
        }
    }

    // Calculate exp() for each value and sum
    float sum = 0.0f;
    for (int i = 0; i < label_count; i++) {
        // Scale the values to prevent overflow/underflow
        softmax_values[i] = exp((output_values[i] - max_val) / 128.0f);
        sum += softmax_values[i];
    }

    // Normalize
    for (int i = 0; i < label_count; i++) {
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