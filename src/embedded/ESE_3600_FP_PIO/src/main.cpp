#include "main.h"

// Setup  Flags
const bool copy_files = false;           // If true, SD card sets formatting flags to NEVER format
const bool collect_data = false;         // If true, data is collected and saved to the file system
const bool run_inference = true;         // If true, inference is run on the data
const bool force_reformat = !copy_files; // If true, the file system will be reformatted during setup

// Data Collection Constants
const String lift_names[3] = {"dC", "bP", "dF"}; // dumbbell curl, bench press, dumbbell flys
String current_lift = lift_names[1];

// TFLite Inference Constants
const int input_length = 200*6;
const int output_length = 5;

void setup()
{
  if (copy_files)
  {
    copy_files_setup();
  }
  else if (collect_data)
  {
    data_collection_setup();
  }
  else if (run_inference)
  {
    setupModel();
  }
}

void loop()
{
  if (copy_files)
  {
    handle_serial_commands();
  }
  else if (collect_data)
  {
    data_collection_loop();
  }
  else if (run_inference)
  {
    // define input_data array to be of length input_length
    float input_data[input_length];
    // define output_data array to be of length output_length
    float output_data[output_length];
    for (int i = 0; i < 200; ++i) {
      for (int j = 0; j < 6; ++j) {
        input_data[i * 6 + j] = static_cast<float>((i + j) / 10.0);
      }
    }
    
    runInference(input_data, input_length, output_data, output_length);
    delay(5000);
  }
}
