#include "imu_provider.h"
#include "../hardware/mpu.h"

const int target_samples = 50;      // Number of samples to collect (50 * 5 ms = 250 ms)
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

void imuCollect(CircularBuffer<TimeSeriesDataPoint> &buffer)
{

  for (int i = 0; i < target_samples; ++i)
  {
    // Fetch IMU data
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    // Add data to the buffer
    addDataToBuffer(
        buffer,
        accel.acceleration.x,
        accel.acceleration.y,
        accel.acceleration.z,
        gyro.gyro.x,
        gyro.gyro.y,
        gyro.gyro.z);

    // Wait for the next sampling interval
    delay(sampling_interval_ms);
  }
}

void addDataToBuffer(CircularBuffer<TimeSeriesDataPoint> &buffer, float aX, float aY, float aZ, float gX, float gY, float gZ)
{
  TimeSeriesDataPoint dataPoint = {
      .aX = normalize_value(aX, ACCEL_MIN, ACCEL_MAX),
      .aY = normalize_value(aY, ACCEL_MIN, ACCEL_MAX),
      .aZ = normalize_value(aZ, ACCEL_MIN, ACCEL_MAX),
      .gX = normalize_value(gX, GYRO_MIN, GYRO_MAX),
      .gY = normalize_value(gY, GYRO_MIN, GYRO_MAX),
      .gZ = normalize_value(gZ, GYRO_MIN, GYRO_MAX)};

  // Add to circular buffer
  buffer.push(dataPoint);
}

float normalize_value(float value, float min, float max)
{
  float normalized = (value - min) / (max - min);
  // Clamp to 0-1
  normalized = (normalized < 0) ? 0 : (normalized > 1) ? 1
                                                       : normalized;
  return normalized;
}
