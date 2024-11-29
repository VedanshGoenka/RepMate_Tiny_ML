#include "imu_provider.h"

Adafruit_MPU6050 mpu;

const int target_samples = 200;     // Number of samples to collect (200 * 5 ms = 1000 ms)
const int sampling_interval_ms = 5; // Interval between samples (1000 ms / 200 samples = 5 ms - 3 ms delay in loop = 2 ms)

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

void imuCollect()
{

  for (int i = 0; i < target_samples; ++i)
  {
    // Fetch IMU data
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    // Add data to the buffer
    addDataToBuffer(
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

void addDataToBuffer(float aX, float aY, float aZ, float gX, float gY, float gZ)
{
  // Create a new data point with float16 precision (half precision, reduced memory usage by 50%)
  TimeSeriesDataPoint dataPoint = {
      .aX = static_cast<float16_t>(aX),
      .aY = static_cast<float16_t>(aY),
      .aZ = static_cast<float16_t>(aZ),
      .gX = static_cast<float16_t>(gX),
      .gY = static_cast<float16_t>(gY),
      .gZ = static_cast<float16_t>(gZ)};

  // Add to circular buffer
  BUFFER.push(dataPoint);
}
