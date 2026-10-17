/**
 * Non-owning view over a contiguous array (pointer + count), modeled on eastl::span.
 * Never allocates or frees; the caller must outlive the span's use.
 */

#pragma once

#include <cstddef>

namespace ME {

// Copying a Span copies the view (pointer + count), not the underlying data or its ownership.
// The array's owner is whoever allocated it; a copied Span is just another borrower of it.
template <typename T>
class Span {
   public:
    T* data = nullptr;
    size_t count = 0;

    T& operator[](size_t index) const {
        return data[index];
    }
};

}  // namespace ME
