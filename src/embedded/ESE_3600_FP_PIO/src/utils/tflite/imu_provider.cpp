#include "imu_provider.h"
#include "../hardware/mpu.h"

const int sampling_interval_ms = 1; // Interval between samples

const int ACCEL_MIN = -25.09375;
const int ACCEL_MAX = 30.8825;
const int GYRO_MIN = -8.54875;
const int GYRO_MAX = 7.995;

void imuSetup()
{
  // Initialize MPU6050
  while (!mpu.begin())
  {
    delay(500);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

// Collect into a flattened buffer (simple float array)
void imuCollect(float *buffer)
{
  for (int i = 0; i < BUFFER_LEN; ++i)
  {
    // Fetch IMU data
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    buffer[i * NUM_FEATURES] = normalize_value(accel.acceleration.x, ACCEL_MIN, ACCEL_MAX);
    buffer[i * NUM_FEATURES + 1] = normalize_value(accel.acceleration.y, ACCEL_MIN, ACCEL_MAX);
    buffer[i * NUM_FEATURES + 2] = normalize_value(accel.acceleration.z, ACCEL_MIN, ACCEL_MAX);
    buffer[i * NUM_FEATURES + 3] = normalize_value(gyro.gyro.x, GYRO_MIN, GYRO_MAX);
    buffer[i * NUM_FEATURES + 4] = normalize_value(gyro.gyro.y, GYRO_MIN, GYRO_MAX);
    buffer[i * NUM_FEATURES + 5] = normalize_value(gyro.gyro.z, GYRO_MIN, GYRO_MAX);

    delay(sampling_interval_ms);
  }
}

float normalize_value(float value, float min, float max)
{
  float normalized = (value - min) / (max - min);
  // Clamp to 0-1
  normalized = (normalized < 0) ? 0 : (normalized > 1) ? 1
                                                       : normalized;
  return normalized;
}
