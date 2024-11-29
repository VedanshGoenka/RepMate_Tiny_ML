#include "imu_provider.h"
#include "../hardware/mpu.h"

const int target_samples = 50;      // Number of samples to collect (50 * 5 ms = 250 ms)
const int sampling_interval_ms = 2; // Interval between samples
