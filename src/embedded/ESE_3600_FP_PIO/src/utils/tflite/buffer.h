#pragma once

#include "esp_attr.h"
#include <cstdint>
#include <stdexcept>

// Buffer Size
constexpr size_t BUFFER_LENGTH = 1500; // 7.5 seconds at 5 ms intervals
constexpr size_t NUM_FEATURES = 6;

// Define the data point structure
struct __attribute__((packed)) TimeSeriesDataPoint
{
  float aX;
  float aY;
  float aZ;
  float gX;
  float gY;
  float gZ;
};

// Define the circular buffer template class
template <typename T>
class CircularBuffer
{
private:
  T *buffer;
  size_t head = 0;
  size_t tail = 0;
  size_t count = 0;

public:
  CircularBuffer();
  ~CircularBuffer();
  void push(const T &data);
  T get(size_t index) const;
  void getData(float *output, size_t required_length) const;
  size_t size() const;
  bool isFull() const;
  void clear();
  void getRecent(size_t n, T *output) const;
};