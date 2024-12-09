#include "main.h"

// Setup  Flags
const bool copy_files = false;           // If true, SD card sets formatting flags to NEVER format
const bool collect_data = false;         // If true, data is collected and saved to the file system
const bool run_inference = true;         // If true, inference is run on the data
const bool force_reformat = !copy_files; // If true, the file system will be reformatted during data collection setup
const bool ble_enabled = true;           // If true, BLE is enabled
const bool buzzer_enabled = true;        // If true, buzzer is enabled

// Data Collection Constants
const String lift_names[3] = {"dC", "bP", "dF"}; // dumbbell curl, bench press, dumbbell flys
String current_lift = lift_names[0];

int current_lift_idx = 1; // {"l_i", "n_l", "o_a", "p_f", "p_m", "s_w"}

// {"Lift Instability", "No Lift", "Off-Axis", "Perfect Form", "Partial Motion", "Swinging Weight"}
const char *current_lift_name = getCurrentLiftName(current_lift_idx);

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
    BLEsetup();
  }
  if (buzzer_enabled)
  {
    buzzerSetup();
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
    if (buzzer_enabled)
    {
      buzz(1000, 100);
    }

    // Collect data
    imuCollect(dataBuffer);

    if (buzzer_enabled)
    {
      for (int i = 0; i < 5; i++)
      {
        buzz(100, 100);
        buzz(1000, 100);
      }
    }

    // Run inference
    doInference(); // This updates the current_lift_idx

    // Get the current lift name
    const char *current_lift_name = getCurrentLiftName(current_lift_idx);

    if (ble_enabled)
    {
      printf("BLE loop\n\n");
      BLEloop(String("Lift was classified as: ") + current_lift_name);
    }
  }
}