#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// Define the pins to monitor
const int pins[] = {0, 1, 2, 3, 4};
const int numPins = sizeof(pins) / sizeof(pins[0]);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Adafruit MPU6050 test!");

  // Initialize MPU6050
  while (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    delay(500);
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Set pin modes
  for (int i = 0; i < numPins; i++) {
    pinMode(pins[i], INPUT);
  }

  Serial.println("Setup complete.");
}

void loop() {
  // Check if any monitored pin is HIGH
  for (int i = 0; i < numPins; i++) {
    if (digitalRead(pins[i]) == HIGH) {
      recordData(pins[i]);
    }
  }
  delay(10);
}

void recordData(int triggeredPin) {
  unsigned long startTime = millis();
  unsigned long duration = 3000; // Record for 3 seconds

  Serial.println("Recording data...");
  while (millis() - startTime < duration) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Print data as a comma-separated list
    Serial.print(triggeredPin);
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

    delay(10); // Adjust sampling rate as needed
  }
  Serial.println("Recording complete.");
}