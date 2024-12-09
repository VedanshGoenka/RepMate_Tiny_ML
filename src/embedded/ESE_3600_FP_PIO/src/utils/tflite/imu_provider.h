#pragma once

#include <Adafruit_MPU6050.h>

const int NUM_FEATURES = 6;
const int BUFFER_LEN = 1000;

void imuSetup();

void imuCollect(float *buffer);
float normalize_value(float value, float min, float max);