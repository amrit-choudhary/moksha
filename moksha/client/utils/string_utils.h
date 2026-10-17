#pragma once

/**
 * String Utilities Header.
 */

#include <cstddef>

namespace ME {

class StringUtils {
   public:
    StringUtils() = default;
    ~StringUtils() = default;

    /**
     * Copies src into dest, truncating to fit destCapacity and always null-terminating.
     */
    static void CopyBounded(char* dest, size_t destCapacity, const char* src);
};

}  // namespace ME
