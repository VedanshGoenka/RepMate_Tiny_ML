#pragma once

#include "esp_attr.h"

#include <cstdint>
#include <stdexcept>

typedef __fp16 float16_t;

// Define buffer dimensions
constexpr size_t BUFFER_LENGTH = 1500; // 7.5 seconds at 5 ms intervals
constexpr size_t NUM_FEATURES = 6;

// Declare global buffer instance with TimeSeriesDataPoint type
extern CircularBuffer<TimeSeriesDataPoint> BUFFER;

// Define the data point structure
struct __attribute__((packed)) TimeSeriesDataPoint
{
  float16_t aX;
  float16_t aY;
  float16_t aZ;
  float16_t gX;
  float16_t gY;
  float16_t gZ;
};

// Define the circular buffer template class
template <typename T>
class CircularBuffer
{
private:
  T buffer[BUFFER_LENGTH];
  size_t head = 0;  // Points to next write location
  size_t tail = 0;  // Points to oldest data
  size_t count = 0; // Current number of elements

public:
  // Push new data point to the buffer
  void push(const T &data);

  // Get data point at specific index relative to tail
  T get(size_t index) const;

  // Get raw data as float array for ML input
  void getData(float16_t *output, size_t required_length) const;

  // Get current number of elements in buffer
  size_t size() const;

  // Check if buffer is full
  bool isFull() const;

  // Clear the buffer
  void clear();
};

template class CircularBuffer<TimeSeriesDataPoint>;