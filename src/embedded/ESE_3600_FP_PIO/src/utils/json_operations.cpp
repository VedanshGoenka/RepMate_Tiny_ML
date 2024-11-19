#include "json_operations.h"
#include "file_system.h"

File json_file;

bool isFirstReading = true;
bool recording = false;

void setupJSON(const int pin)
{
  if (recording)
  {
    return;
  }
  // Create the file
  String file_path = add_file_to_folder(pin, "data", ".json");
  if (file_path.isEmpty())
  {
    return;
  }
  json_file = LittleFS.open(file_path, "w");
  if (!json_file)
  {
    return;
  }
}

void createJSONHeading(String lift_name, String lift_classification)
{
  if (!json_file)
  {
    return;
  }
  json_file.println("{");
  json_file.printf("\"lift_name\": \"%s\",\n", lift_name.c_str());
  json_file.printf("\"lift_classification\": \"%s\",\n", lift_classification.c_str());
  json_file.println("\"Time Series Data\": [");
  // save the state by flushing the file
  json_file.flush();
  isFirstReading = true;
}

void addDataPoint(float timestamp, float accel_x, float accel_y, float accel_z, float gyro_x, float gyro_y, float gyro_z)
{
  if (!json_file)
  {
    return;
  }
  if (!isFirstReading)
  {
    json_file.println(",");
  }
  json_file.printf("{\"timestamp\": %f, \"Accelerometer X\": %f, \"Accelerometer Y\": %f, \"Accelerometer Z\": %f, \"Gyroscope X\": %f, \"Gyroscope Y\": %f, \"Gyroscope Z\": %f}\n",
                   timestamp, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);
  isFirstReading = false;
}

void closeJSONArray()
{
  if (!json_file)
  {
    return;
  }
  json_file.println("]");
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
  json_file.close();
  isFirstReading = true;
}