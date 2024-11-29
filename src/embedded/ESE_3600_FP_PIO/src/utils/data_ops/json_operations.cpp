#include "json_operations.h"

File json_file;

bool isFirstReading = true;
bool recording = false;

void setupJSON(const int pin)
{
  if (recording)
  {
    return;
  }

  for (int i = 0; i < numPins; i++)
  {
    if (!setup_folder_structure(pins[i]))
    {
      Serial.printf("Failed to setup folder structure for pin %u\n", pins[i]);
      return;
    }
  }

  // Create the file
  String file_path = add_file_to_folder(pin, "d", ".json");
  if (file_path.isEmpty())
  {
    return;
  }
  json_file = LittleFS.open(file_path, "w");
  if (!json_file)
  {
    Serial.printf("Failed to create file for pin %u\n", pin);
    return;
  }
}

void createJSONHeading(String lift_name, String lift_classification)
{
  if (!json_file)
    return;

  recording = true;
  json_file.println("{");
  json_file.printf("  \"lN\": \"%s\",\n", lift_name.c_str());
  json_file.printf("  \"lC\": \"%s\",\n", lift_classification.c_str());
  json_file.println("  \"tSD\": [");
  json_file.flush();
  isFirstReading = true;
}

void addDataPoint(float timestamp, float accel_x, float accel_y, float accel_z, float gyro_x, float gyro_y, float gyro_z)
{
  if (!json_file)
    return;

  if (!isFirstReading)
  {
    json_file.print(",\n    ");
  }
  else
  {
    json_file.print("    ");
  }
  json_file.printf(
      "{\"t\": %.1f, \"aX\": %.3f, \"aY\": %.3f, \"aZ\": %.3f, \"gX\": %.3f, \"gY\": %.3f, \"gZ\": %.3f}",
      timestamp, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);
  isFirstReading = false;
}

void closeJSONArray()
{
  if (!json_file)
  {
    return;
  }
  json_file.println("\n  ]");
  json_file.println("}");
  json_file.flush();
  isFirstReading = true;
}

void closeDataFile()
{
  if (!json_file)
  {
    return;
  }
  json_file.flush();
  json_file.close();
  isFirstReading = true;
  recording = false;
}
