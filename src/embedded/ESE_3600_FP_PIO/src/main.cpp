#include "main.h"

// Setup  Flags
const bool copy_files = false;           // If true, SD card sets formatting flags to NEVER format
const bool collect_data = false;         // If true, data is collected and saved to the file system
const bool run_inference = true;         // If true, inference is run on the data
const bool force_reformat = !copy_files; // If true, the file system will be reformatted during data collection setup
const bool ble_enabled = true;           // If true, BLE is enabled
const bool buzzer_enabled = true;        // If true, buzzer is enabled
const uint8_t LEDpins[5] = {D0, D1, D2, D3, D6};

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
    for (int i = 0; i < 5; i++)
    {
      pinMode(LEDpins[i], OUTPUT);
    }
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

    printf("Starting Data Collection\n");

    if (buzzer_enabled)
    {
      for (int i = 0; i < 5; i++)
      {
        buzz(100, 100);
        buzz(1000, 100);
      }
    }
    // Collect data
    imuCollect(dataBuffer);

    if (buzzer_enabled)
    {
      buzz(1000, 100);
    }

    printf("Collected Data\n");

    // Run inference
    doInference(); // This updates the current_lift_idx

    // Get the current lift name
    const char *current_lift_name = getCurrentLiftName(current_lift_idx);

    // Set all the pins to low
    for (int i = 0; i < 5; i++)
    {
      digitalWrite(LEDpins[i], LOW);
    }

    // Set the pin that corresponds to the current lift high and the rest low
    printf("Current lift index: %d\n", current_lift_idx);
    switch (current_lift_idx)
    {
    case 0:
      digitalWrite(LEDpins[1], HIGH);
      break;
    case 2:
      digitalWrite(LEDpins[2], HIGH);
      break;
    case 3:
      digitalWrite(LEDpins[0], HIGH);
      break;
    case 4:
      digitalWrite(LEDpins[3], HIGH);
      break;
    case 5:
      digitalWrite(LEDpins[4], HIGH);
      break;
    default:
      break;
    }

    if (ble_enabled)
    {
      // printf("BLE loop\n\n");
      BLEloop(String("Lift was classified as: ") + current_lift_name);
    }
    delay(2000);
  }
}