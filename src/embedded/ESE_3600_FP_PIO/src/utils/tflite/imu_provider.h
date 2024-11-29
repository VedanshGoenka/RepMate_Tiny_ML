#pragma once

#include <Adafruit_MPU6050.h>
#include "buffer.h"

extern CircularBuffer<TimeSeriesDataPoint> BUFFER;

void imuSetup();
void imuCollect();
void addDataToBuffer(float aX, float aY, float aZ, float gX, float gY, float gZ);
