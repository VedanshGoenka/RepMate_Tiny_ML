#pragma once

#include <Adafruit_MPU6050.h>
#include "buffer.h"

void imuSetup();
void imuCollect(CircularBuffer<TimeSeriesDataPoint> &buffer);
void addDataToBuffer(CircularBuffer<TimeSeriesDataPoint> &buffer, float aX, float aY, float aZ, float gX, float gY, float gZ);
