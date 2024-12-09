#pragma once

#include <Adafruit_MPU6050.h>
#include "circular_buffer.h"
#include "flat_buffer.h"

void imuSetup();

template <template <typename> class T>
void imuCollect(T<TimeSeriesDataPoint> &buffer);

void imuCollect(CircularBuffer<TimeSeriesDataPoint> &buffer);
void imuFlatCollect(FlatBuffer<TimeSeriesDataPoint> &buffer);

template <template <typename> class T>
void addDataToBuffer(T<TimeSeriesDataPoint> &buffer, float aX, float aY, float aZ, float gX, float gY, float gZ);

float normalize_value(float value, float min, float max);
