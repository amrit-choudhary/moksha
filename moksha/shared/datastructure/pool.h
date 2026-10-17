#pragma once

#include <cstddef>
#include <cstdint>

namespace ME {

/**
 * A simple object pool of fixed size.
 * No dynamic resizing is done. Active items are packed into [0, activeCount)
 * so Acquire/Release are O(1) with no scanning.
 *
 * Release() fills the hole by moving the last active item into the freed
 * slot, so the item currently at index (activeCount - 1) can move to a new
 * address on any Release() call, not just its own. Don't hold a T* across a
 * Release() of a different item, and never Release() the same pointer twice.
 */
template <typename T>
class Pool {
   public:
    Pool() = delete;
    Pool(size_t size) : size(size), activeCount(0) {
        items = new T[size];
    }

    ~Pool() {
        delete[] items;
    }

    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;
    Pool(Pool&&) = delete;
    Pool& operator=(Pool&&) = delete;

    // Creates a new item in the pool and returns a pointer to it, or nullptr if the pool is full.
    T* Acquire() {
        if (activeCount == size) {
            return nullptr;  // Pool is full
        }
        return &items[activeCount++];
    }

    // Creates multiple new items in the pool and returns a pointer to the first one, or nullptr if there is not enough
    // space.
    T* Acquire(size_t count) {
        if (count > size - activeCount) {
            return nullptr;  // Not enough space in the pool
        }
        T* result = &items[activeCount];
        activeCount += count;
        return result;
    }

    // Releases an item back to the pool. The item must have been acquired from this pool and not already released.
    void Release(T* item) {
        size_t index = item - items;
        if (index >= activeCount) {
            return;  // out of range, or already free
        }
        --activeCount;
        if (index != activeCount) {
            items[index] = items[activeCount];
        }
    }

    /**
     * Number of currently active (acquired) items, packed into [0, GetActiveCount()).
     */
    size_t GetActiveCount() const {
        return activeCount;
    }

    T& operator[](size_t index) {
        return items[index];
    }

   private:
    size_t size = 0;
    size_t activeCount = 0;
    T* items = nullptr;
};

}  // namespace ME
