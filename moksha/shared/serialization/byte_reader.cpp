#include "byte_reader.h"

#include <cstring>

// memcpy copies a value's bytes exactly as they sit in memory (native byte order) and works for any
// alignment, unlike casting a uint8_t* to uint32_t*.

ME::ByteReader::ByteReader(const uint8_t* data, size_t size) : data(data), size(size) {}

bool ME::ByteReader::ReadU8(uint8_t& out) {
    if (GetRemaining() < sizeof(out)) {
        return false;
    }
    out = data[position];
    position += sizeof(out);
    return true;
}

bool ME::ByteReader::ReadU16(uint16_t& out) {
    if (GetRemaining() < sizeof(out)) {
        return false;
    }
    std::memcpy(&out, data + position, sizeof(out));
    position += sizeof(out);
    return true;
}

bool ME::ByteReader::ReadU32(uint32_t& out) {
    if (GetRemaining() < sizeof(out)) {
        return false;
    }
    std::memcpy(&out, data + position, sizeof(out));
    position += sizeof(out);
    return true;
}

bool ME::ByteReader::ReadBytes(size_t count, const uint8_t*& outData) {
    if (GetRemaining() < count) {
        return false;
    }
    outData = data + position;
    position += count;
    return true;
}

size_t ME::ByteReader::GetRemaining() const {
    return size - position;
}
