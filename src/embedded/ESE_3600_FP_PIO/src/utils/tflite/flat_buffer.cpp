#include "flat_buffer.h"

// Constructor implementation
template <typename T>
FlatBuffer<T>::FlatBuffer()
{
  buffer = new T[BUFFER_LENGTH];
  head = 0;
  count = 0;
}

// Destructor implementation
template <typename T>
FlatBuffer<T>::~FlatBuffer()
{
  delete[] buffer;
}

// Add a data point to the buffer
template <typename T>
void FlatBuffer<T>::push(const T &data)
{
  if (head >= BUFFER_LENGTH)
  {
    throw std::runtime_error("Buffer is full");
  }
  buffer[head] = data;
  head++;
  count++;
}

// Get a data point from the buffer
template <typename T>
T FlatBuffer<T>::get(size_t index) const
{
  if (index >= count)
  {
    throw std::out_of_range("Index out of bounds");
  }
  return buffer[index];
}

// Get the number of elements in the buffer
template <typename T>
size_t FlatBuffer<T>::size() const
{
  return count;
}

// Check if the buffer is full
template <typename T>
bool FlatBuffer<T>::isFull() const
{
  return count == BUFFER_LENGTH;
}

// Clear the buffer
template <typename T>
void FlatBuffer<T>::clear()
{
  head = 0;
  count = 0;
}

// Output full buffer
template <typename T>
void FlatBuffer<T>::getData(float *output, size_t required_length) const
{
  if (required_length > count)
  {
    throw std::runtime_error("Not enough data in buffer");
  }

  for (size_t i = 0; i < required_length; i++)
  {
    TimeSeriesDataPoint point = get(i);
    output[i * NUM_FEATURES + 0] = point.aX;
    output[i * NUM_FEATURES + 1] = point.aY;
    output[i * NUM_FEATURES + 2] = point.aZ;
    output[i * NUM_FEATURES + 3] = point.gX;
    output[i * NUM_FEATURES + 4] = point.gY;
    output[i * NUM_FEATURES + 5] = point.gZ;
  }
}

template <typename T>
void FlatBuffer<T>::getRecent(size_t n, T *output) const
{
  // Copy the most recent n elements
  for (size_t i = 0; i < n && i < count; i++)
  {
    output[i] = buffer[i];
  }
}

// Explicit template instantiation
template class FlatBuffer<TimeSeriesDataPoint>;
