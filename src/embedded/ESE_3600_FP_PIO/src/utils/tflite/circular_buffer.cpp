#include "circular_buffer.h"

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

// Add a data point to the buffer
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

// Get a data point from the buffer
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

// Get the data from the buffer
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

// Get the number of elements in the buffer
template <typename T>
size_t CircularBuffer<T>::size() const
{
  return count;
}

// Check if the buffer is full
template <typename T>
bool CircularBuffer<T>::isFull() const
{
  return count == BUFFER_LENGTH;
}

// Clear the buffer
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
  // Input validation with detailed error messages
  if (n > count)
  {
    char err[100];
    snprintf(err, sizeof(err),
             "Not enough data in buffer. Requested: %zu, Available: %zu",
             n, count);
    throw std::runtime_error(err);
  }
  if (n > BUFFER_LENGTH)
  {
    char err[100];
    snprintf(err, sizeof(err),
             "Requested more data than buffer size. Requested: %zu, Max: %zu",
             n, BUFFER_LENGTH);
    throw std::runtime_error(err);
  }
  if (output == nullptr)
  {
    throw std::runtime_error("Output buffer is null");
  }

  // Calculate start position for copying
  size_t start = (head + BUFFER_LENGTH - n) % BUFFER_LENGTH;

  // printf("Buffer stats - head: %zu, count: %zu, n: %zu, start: %zu\n",
  //        head, count, n, start);

  // Copy the most recent n elements with proper alignment handling
  for (size_t i = 0; i < n; i++)
  {
    size_t buffer_index = (start + i) % BUFFER_LENGTH;

    // Copy individual fields to maintain alignment
    output[i].aX = buffer[buffer_index].aX;
    output[i].aY = buffer[buffer_index].aY;
    output[i].aZ = buffer[buffer_index].aZ;
    output[i].gX = buffer[buffer_index].gX;
    output[i].gY = buffer[buffer_index].gY;
    output[i].gZ = buffer[buffer_index].gZ;
  }
}

// Explicit template instantiation
template class CircularBuffer<TimeSeriesDataPoint>;