#include "data_collection.h"

Adafruit_MPU6050 mpu;

// Configuration Parameters
const uint8_t pins[5] = {D0, D1, D2, D3, D6};
const String lift_names[3] = {"Dumbbell Curl", "Bench Press", "Dumbbell Flys"};
String current_lift = lift_names[0];
bool output_to_json = false;
const unsigned long duration = 3000;
const unsigned long sampling_rate = 1;

const std::map<int, String> lift_classification_map = {
    {pins[0], "Proper Form"},
    {pins[1], "Lift Instability"},
    {pins[2], "Partial Motion"},
    {pins[3], "Off-Axis"},
    {pins[4], "Swinging the Weight"}};

void data_collection_setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("Adafruit MPU6050 test!");

  // Initialize MPU6050
  while (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    delay(500);
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Set pin modes
  for (int i = 0; i < numPins; i++)
  {
    pinMode(pins[i], INPUT);
  }

  // Add file system initialization
  if (!file_system_setup())
  {
    Serial.println("Failed to initialize file system");
    while (1)
      delay(1000); // Halt if file system fails
  }

  Serial.println("Setup complete.");
}

void data_collection_loop()
{
  // Check if any monitored pin is HIGH
  for (int i = 0; i < numPins; i++)
  {
    if (digitalRead(pins[i]) == HIGH)
    {
      recordData(pins[i], output_to_json);
    }
  }
  delay(10);
}

void recordData(int triggeredPin, bool to_json)
{
  Serial.println("Recording data...");
  if (to_json)
  {
    setupJSON(triggeredPin);
    createJSONHeading(current_lift, lift_classification_map.find(triggeredPin)->second);
  }
  unsigned long startTime = millis();
  while (millis() - startTime < duration)
  {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Print data as a comma-separated list
    if (to_json)
    {
      addDataPoint(millis() - startTime, a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z);
    }
    else
    {
      printData(millis() - startTime, triggeredPin, a, g);
    }
    delay(sampling_rate); // Adjust sampling rate as needed
  }
  Serial.println("Recording complete.");
  if (to_json)
  {
    closeJSONArray();
    closeDataFile();
  }
}

void printData(unsigned long time, int triggeredPin, sensors_event_t a, sensors_event_t g)
{
  Serial.print(time);
  Serial.print(", ");
  Serial.print(lift_classification_map.find(triggeredPin)->second.c_str());
  Serial.print(", ");
  Serial.print(a.acceleration.x);
  Serial.print(", ");
  Serial.print(a.acceleration.y);
  Serial.print(", ");
  Serial.print(a.acceleration.z);
  Serial.print(", ");
  Serial.print(g.gyro.x);
  Serial.print(", ");
  Serial.print(g.gyro.y);
  Serial.print(", ");
  Serial.println(g.gyro.z);
}