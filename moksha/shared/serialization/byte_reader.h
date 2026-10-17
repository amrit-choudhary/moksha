/**
 * Bounds-checked binary reading from memory owned by the caller, in native byte order.
 * Counterpart of ByteWriter; never reads past the end of its data.
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace ME {

class ByteReader {
   public:
    ByteReader(const uint8_t* data, size_t size);

    /**
     * Each read returns false, and leaves out unchanged, if fewer bytes remain than it needs.
     */
    bool ReadU8(uint8_t& out);
    bool ReadU16(uint16_t& out);
    bool ReadU32(uint32_t& out);

    /**
     * Points outData at the next count bytes inside the source memory (no copy) and moves past them.
     */
    bool ReadBytes(size_t count, const uint8_t*& outData);

    size_t GetRemaining() const;

   private:
    const uint8_t* data = nullptr;
    size_t size = 0;
    size_t position = 0;
};

}  // namespace ME
