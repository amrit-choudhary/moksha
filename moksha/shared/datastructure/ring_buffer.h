/**
 * A fixed size array of elements where items are inserted sequentially.
 * If it's size is exceeded, it starts filling from the start, replacing
 * old elements.
 * Use cases: if we have a long running time series of data, this can
 * effeciently hold last few significant values.
 */

#pragma once

#include <cstdint>
#include <iostream>

namespace ME {

template <typename T>
class RingBuffer {
   public:
    RingBuffer(uint16_t maxCapacity);
    ~RingBuffer();
    /**
     * Returns current count of elements in the sequence.
     */
    uint16_t GetCount() const;

    /**
     * Returns the capacity of ring buffer.
     */
    uint16_t GetCapacity() const;

    /**
     * Inserts an element in the buffer in a cyclic fashion.
     */
    void Insert(T item);

    /**
     * Gets items from the buffer in FIFO type sequence.
     * Trying to Get(index 0) will be oldest element.
     * Trying to Get(count -1) will be the newest element.
     */
    T Get(uint16_t index) const;
    /**
     * Same as Get(index).
     */
    T operator[](uint16_t index) const;

   private:
    // Delete default constructor, copy, move and assignment.
    RingBuffer() = delete;                               // default constructor
    RingBuffer(const RingBuffer&) = delete;              // copy
    RingBuffer& operator=(const RingBuffer&) = delete;   // copy assignment
    RingBuffer(RingBuffer&&) = delete;                   // move
    RingBuffer& operator=(const RingBuffer&&) = delete;  // move assignment

    const uint16_t capacity;
    uint16_t startIndex;
    uint16_t count;
    T* buffer;
};

template <typename T>
RingBuffer<T>::RingBuffer(uint16_t maxCapacity) : capacity{maxCapacity} {
    startIndex = 0;
    count = 0;
    buffer = new T[capacity];
}

template <typename T>
ME::RingBuffer<T>::~RingBuffer() {
    delete[] buffer;
}

template <typename T>
uint16_t ME::RingBuffer<T>::GetCount() const {
    return count;
}

template <typename T>
uint16_t ME::RingBuffer<T>::GetCapacity() const {
    return capacity;
}

template <typename T>
void ME::RingBuffer<T>::Insert(T item) {
    // Buffer not full.
    if (count < capacity) {
        startIndex = 0;
        buffer[count] = item;
        ++count;
    }
    // Buffer full.
    else {
        ++startIndex;
        if (startIndex >= capacity) {
            startIndex = 0;
            buffer[capacity - 1] = item;
        } else {
            buffer[startIndex - 1] = item;
        }
    }
}

template <typename T>
T ME::RingBuffer<T>::Get(uint16_t index) const {
    uint16_t destIndex = startIndex + index;
    if (destIndex >= capacity) {
        destIndex -= capacity;
    }
    return buffer[destIndex];
}

template <typename T>
inline T RingBuffer<T>::operator[](uint16_t index) const {
    uint16_t destIndex = startIndex + index;
    if (destIndex >= capacity) {
        destIndex -= capacity;
    }
    return buffer[destIndex];
}

}  // namespace ME