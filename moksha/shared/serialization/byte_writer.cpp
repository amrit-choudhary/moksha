#include "byte_writer.h"

#include <cstring>

// memcpy copies a value's bytes exactly as they sit in memory (native byte order) and works for any
// alignment, unlike casting a uint8_t* to uint32_t*.

ME::ByteWriter::ByteWriter(uint8_t* buffer, size_t capacity) : buffer(buffer), capacity(capacity) {}

bool ME::ByteWriter::WriteU8(uint8_t value) {
    return WriteBytes(&value, sizeof(value));
}

bool ME::ByteWriter::WriteU16(uint16_t value) {
    return WriteBytes(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
}

bool ME::ByteWriter::WriteU32(uint32_t value) {
    return WriteBytes(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
}

bool ME::ByteWriter::WriteBytes(const uint8_t* data, size_t count) {
    if (GetRemaining() < count) {
        return false;
    }
    std::memcpy(buffer + position, data, count);
    position += count;
    return true;
}

bool ME::ByteWriter::PatchU32(size_t offset, uint32_t value) {
    // Only bytes already written can be patched.
    if (offset > position || position - offset < sizeof(value)) {
        return false;
    }
    std::memcpy(buffer + offset, &value, sizeof(value));
    return true;
}

size_t ME::ByteWriter::GetSize() const {
    return position;
}

size_t ME::ByteWriter::GetRemaining() const {
    return capacity - position;
}
