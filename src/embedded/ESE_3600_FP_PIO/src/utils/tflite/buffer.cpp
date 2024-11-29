#include "buffer.h"

// Initialize global buffer instance
CircularBuffer<TimeSeriesDataPoint> BUFFER;

// Constructor implementation
template <typename T>
CircularBuffer<T>::CircularBuffer()
{
  buffer = new T[BUFFER_LENGTH];
  head = 0;
  tail = 0;
  count = 0;
}

// Destructor implementation
template <typename T>
CircularBuffer<T>::~CircularBuffer()
{
  delete[] buffer;
}

// Define the push method for the circular buffer template class
template <typename T>
void CircularBuffer<T>::push(const T &data)
{
  buffer[head] = data;
  head = (head + 1) % BUFFER_LENGTH;

  if (count < BUFFER_LENGTH)
  {
    count++;
  }
  else
  {
    // Buffer is full, move tail
    tail = (tail + 1) % BUFFER_LENGTH;
  }
}

// Define the get method for the circular buffer template class
template <typename T>
T CircularBuffer<T>::get(size_t index) const
{
  if (index >= count)
  {
    throw std::out_of_range("Index out of bounds");
  }
  size_t actual_index = (tail + index) % BUFFER_LENGTH;
  return buffer[actual_index];
}

// Define the getData method for the circular buffer template class
template <typename T>
void CircularBuffer<T>::getData(float *output, size_t required_length) const
{
  if (required_length > count)
  {
    throw std::runtime_error("Not enough data in buffer");
  }

  for (size_t i = 0; i < required_length; i++)
  {
    TimeSeriesDataPoint point = get(i);
    // Flatten the struct into array format [aX, aY, aZ, gX, gY, gZ]
    output[i * NUM_FEATURES + 0] = point.aX;
    output[i * NUM_FEATURES + 1] = point.aY;
    output[i * NUM_FEATURES + 2] = point.aZ;
    output[i * NUM_FEATURES + 3] = point.gX;
    output[i * NUM_FEATURES + 4] = point.gY;
    output[i * NUM_FEATURES + 5] = point.gZ;
  }
}

// Define the size method for the circular buffer template class
template <typename T>
size_t CircularBuffer<T>::size() const
{
  return count;
}

// Define the isFull method for the circular buffer template class
template <typename T>
bool CircularBuffer<T>::isFull() const
{
  return count == BUFFER_LENGTH;
}

// Define the clear method for the circular buffer template class
template <typename T>
void CircularBuffer<T>::clear()
{
  head = 0;
  tail = 0;
  count = 0;
}

// Get the most recent n elements into output array
template <typename T>
void CircularBuffer<T>::getRecent(size_t n, T *output) const
{
  if (n > count)
  {
    throw std::runtime_error("Not enough data in buffer");
  }

  size_t start = (head - n + BUFFER_LENGTH) % BUFFER_LENGTH;
  for (size_t i = 0; i < n; i++)
  {
    output[i] = buffer[(start + i) % BUFFER_LENGTH];
  }
}

// Explicit template instantiation
template class CircularBuffer<TimeSeriesDataPoint>;
