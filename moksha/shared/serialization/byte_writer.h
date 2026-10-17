/**
 * Bounds-checked binary writing into memory owned by the caller, in native byte order.
 * General serialization: network messages, binary files, save data. Read back with ByteReader.
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace ME {

class ByteWriter {
   public:
    ByteWriter(uint8_t* buffer, size_t capacity);

    /**
     * Each write returns false, and writes nothing, if fewer bytes of room remain than it needs.
     */
    bool WriteU8(uint8_t value);
    bool WriteU16(uint16_t value);
    bool WriteU32(uint32_t value);
    bool WriteBytes(const uint8_t* data, size_t count);

    /**
     * Overwrites 4 bytes already written at offset, e.g. a length field filled in after the payload.
     */
    bool PatchU32(size_t offset, uint32_t value);

    /**
     * Number of bytes written so far, from the start of the buffer.
     */
    size_t GetSize() const;
    size_t GetRemaining() const;

   private:
    uint8_t* buffer = nullptr;
    size_t capacity = 0;
    size_t position = 0;
};

}  // namespace ME
