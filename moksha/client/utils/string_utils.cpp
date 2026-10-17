#include "string_utils.h"

#include <cstring>

void ME::StringUtils::CopyBounded(char* dest, size_t destCapacity, const char* src) {
    strncpy(dest, src, destCapacity - 1);
    dest[destCapacity - 1] = '\0';
}
