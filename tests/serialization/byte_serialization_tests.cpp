/**
 * Tests for ByteWriter and ByteReader.
 */

#include <cstdint>

#include "moksha/shared/serialization/byte_reader.h"
#include "moksha/shared/serialization/byte_writer.h"
#include "moksha/test_framework/test_framework.h"

TEST(ByteWriter, RoundTripThroughReader) {
    uint8_t buffer[16] = {};
    ME::ByteWriter writer(buffer, sizeof(buffer));
    const uint8_t bytes[3] = {7, 8, 9};

    EXPECT(writer.WriteU8(0xAB));
    EXPECT(writer.WriteU16(0x1234));
    EXPECT(writer.WriteU32(0xDEADBEEF));
    EXPECT(writer.WriteBytes(bytes, 3));
    EXPECT(writer.GetSize() == 10);
    EXPECT(writer.GetRemaining() == 6);

    ME::ByteReader reader(buffer, writer.GetSize());
    uint8_t u8 = 0;
    uint16_t u16 = 0;
    uint32_t u32 = 0;
    const uint8_t* outBytes = nullptr;

    EXPECT(reader.ReadU8(u8) && u8 == 0xAB);
    EXPECT(reader.ReadU16(u16) && u16 == 0x1234);
    EXPECT(reader.ReadU32(u32) && u32 == 0xDEADBEEF);
    ASSERT(reader.ReadBytes(3, outBytes));
    EXPECT(outBytes[0] == 7 && outBytes[1] == 8 && outBytes[2] == 9);
    EXPECT(reader.GetRemaining() == 0);
}

TEST(ByteWriter, WriteFailsWhenFullAndWritesNothing) {
    uint8_t buffer[3] = {};
    ME::ByteWriter writer(buffer, sizeof(buffer));

    EXPECT(writer.WriteU16(1));
    EXPECT(!writer.WriteU32(2));
    EXPECT(writer.GetSize() == 2);
    EXPECT(writer.WriteU8(3));
    EXPECT(!writer.WriteU8(4));
    EXPECT(writer.GetRemaining() == 0);
}

TEST(ByteWriter, PatchU32OverwritesInPlace) {
    uint8_t buffer[8] = {};
    ME::ByteWriter writer(buffer, sizeof(buffer));
    writer.WriteU32(0);
    writer.WriteU32(5);

    EXPECT(writer.PatchU32(0, 99));
    EXPECT(writer.GetSize() == 8);
    EXPECT(!writer.PatchU32(6, 1));

    ME::ByteReader reader(buffer, writer.GetSize());
    uint32_t first = 0;
    uint32_t second = 0;
    reader.ReadU32(first);
    reader.ReadU32(second);
    EXPECT(first == 99);
    EXPECT(second == 5);
}

TEST(ByteReader, ReadFailsWhenShortAndLeavesOutUnchanged) {
    const uint8_t data[3] = {1, 2, 3};
    ME::ByteReader reader(data, sizeof(data));

    uint32_t u32 = 42;
    EXPECT(!reader.ReadU32(u32));
    EXPECT(u32 == 42);
    EXPECT(reader.GetRemaining() == 3);

    const uint8_t* bytes = nullptr;
    EXPECT(!reader.ReadBytes(4, bytes));
    EXPECT(reader.ReadBytes(3, bytes));
}
