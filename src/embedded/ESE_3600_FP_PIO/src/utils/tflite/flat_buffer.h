#pragma once

#include "circular_buffer.h"

// Buffer Size
extern const size_t BUFFER_LENGTH;
extern const size_t NUM_FEATURES;

// Define the data point structure
struct TimeSeriesDataPoint;

template <typename T>
class FlatBuffer
{
private:
  T *buffer;
  size_t head;
  size_t count;

public:
  FlatBuffer();
  ~FlatBuffer();
  void push(const T &data);
  T get(size_t index) const;
  void getData(float *output, size_t required_length) const;
  void getRecent(size_t n, T *output) const;
  size_t size() const;
  bool isFull() const;
  void clear();
};