#include "debug.h"
#include <cstdio>
#include <cfloat>
#include <algorithm>

void inspectModel(const tflite::Model *model, tflite::ErrorReporter *error_reporter)
{
  if (!model)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Model is null");
    return;
  }

  TF_LITE_REPORT_ERROR(error_reporter,
                       "Model version: %d\n"
                       "Operator codes: %d\n"
                       "Subgraphs: %d\n"
                       "Description: %s",
                       model->version(),
                       model->operator_codes()->size(),
                       model->subgraphs()->size(),
                       model->description() ? model->description()->c_str() : "None");
}

void inspectModelData(const uint8_t *model_data, size_t model_size)
{
  printf("Model data header: ");
  for (int i = 0; i < 16 && i < model_size; i++)
  {
    printf("%02x ", model_data[i]);
  }
  printf("\n");
}

void inspectTensorDetails(tflite::MicroInterpreter *interpreter, tflite::ErrorReporter *error_reporter)
{
  TfLiteTensor *input = interpreter->input(0);
  TfLiteTensor *output = interpreter->output(0);

  if (input)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Input tensor details:\n"
                         "- Type: %d\n"
                         "- Dims: [%d,%d,%d]\n"
                         "- Bytes: %zu",
                         input->type,
                         input->dims->data[0],
                         input->dims->data[1],
                         input->dims->data[2],
                         input->bytes);
  }

  if (output)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Output tensor details:\n"
                         "- Type: %d\n"
                         "- Dims: [%d,%d]\n"
                         "- Bytes: %zu",
                         output->type,
                         output->dims->data[0],
                         output->dims->data[1],
                         output->bytes);
  }
}

void inspectInputShape(TfLiteTensor *input, tflite::ErrorReporter *error_reporter)
{
  if (!input)
    return;

  if (input->dims->size != 3 ||
      input->dims->data[1] != OUTPUT_SEQUENCE_LENGTH ||
      input->dims->data[2] != NUM_FEATURES)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Invalid input shape: [%d,%d,%d], expected: [1,%d,%d]",
                         input->dims->data[0], input->dims->data[1], input->dims->data[2],
                         OUTPUT_SEQUENCE_LENGTH, NUM_FEATURES);
  }
}

void inspectInputStats(TfLiteTensor *input)
{
  if (!input)
    return;

  float min_val = FLT_MAX;
  float max_val = -FLT_MAX;
  float sum = 0;
  float *input_data = input->data.f;
  int input_size = input->dims->data[1] * input->dims->data[2];

  for (int i = 0; i < input_size; i++)
  {
    min_val = std::min(min_val, input_data[i]);
    max_val = std::max(max_val, input_data[i]);
    sum += input_data[i];
  }

  printf("Input stats - min: %f, max: %f, mean: %f\n",
         min_val, max_val, sum / input_size);
}

void inspectOutputValues(TfLiteTensor *output, const char *labels[], int label_count)
{
  if (!output)
    return;

  printf("Raw output values:\n");
  for (int i = 0; i < label_count; i++)
  {
    printf("%s: %f\n", labels[i], output->data.f[i]);
  }
}