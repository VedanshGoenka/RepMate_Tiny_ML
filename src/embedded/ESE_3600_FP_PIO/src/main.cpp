#include "main.h"

// Setup  Flags
const bool copy_files = false;           // If true, SD card sets formatting flags to NEVER format
const bool collect_data = false;         // If true, data is collected and saved to the file system
const bool run_inference = true;         // If true, inference is run on the data
const bool force_reformat = !copy_files; // If true, the file system will be reformatted during data collection setup
const bool ble_enabled = true;          // If true, BLE is enabled

// Data Collection Constants
const String lift_names[3] = {"dC", "bP", "dF"}; // dumbbell curl, bench press, dumbbell flys
String current_lift = lift_names[0];

// Buffer Constants

// Define the buffer using the selected type
extern float dataBuffer[];

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
    // Setup IMU
    imuSetup();

    // Setup TFLite
    setupModel(false);
  }
  if (ble_enabled)
  {
    setupBLE();
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
    // Collect data
    imuCollect(dataBuffer);
    // Run inference
    doInference();

    if (ble_enabled)
    {
      printf("BLE loop\n\n");
      BLEloop("Lift was classified as: No Lift");
    }
  }
}