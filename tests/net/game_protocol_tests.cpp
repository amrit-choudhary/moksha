/**
 * Tests for the UDP game protocol encoding helpers.
 */

#include <cstdint>
#include <cstring>

#include "moksha/shared/math/fp_24_8.h"
#include "moksha/shared/net/game_protocol.h"
#include "moksha/shared/serialization/byte_reader.h"
#include "moksha/shared/serialization/byte_writer.h"
#include "moksha/test_framework/test_framework.h"

namespace GP = ME::Net::GameProtocol;

TEST(GameProtocol, HeaderRoundTrip) {
    uint8_t buffer[GP::MAX_DATAGRAM_SIZE] = {};
    ME::ByteWriter writer(buffer, sizeof(buffer));
    ASSERT(GP::WriteHeader(writer, GP::Verb::PING, 5));
    EXPECT(writer.GetSize() == 3);

    ME::ByteReader reader(buffer, writer.GetSize());
    GP::Header header;
    ASSERT(GP::ReadHeader(reader, header));
    EXPECT(header.version == GP::VERSION);
    EXPECT(header.verb == static_cast<uint8_t>(GP::Verb::PING));
    EXPECT(header.clientID == 5);
}

TEST(GameProtocol, ShortDatagramHasNoHeader) {
    const uint8_t data[2] = {0, 0x03};
    ME::ByteReader reader(data, sizeof(data));
    GP::Header header;
    header.clientID = 77;

    EXPECT(!GP::ReadHeader(reader, header));
    EXPECT(header.clientID == 77);
}

TEST(GameProtocol, FixedPointRoundTripIsExact) {
    uint8_t buffer[8] = {};
    ME::ByteWriter writer(buffer, sizeof(buffer));
    ME::FP_24_8 value(-12.75f);
    ASSERT(GP::WriteFP(writer, value));

    ME::ByteReader reader(buffer, writer.GetSize());
    ME::FP_24_8 out;
    ASSERT(GP::ReadFP(reader, out));
    EXPECT(out.GetRaw() == value.GetRaw());
}

TEST(GameProtocol, StringRoundTrip) {
    uint8_t buffer[64] = {};
    ME::ByteWriter writer(buffer, sizeof(buffer));
    ASSERT(GP::WriteString(writer, "hello"));
    EXPECT(writer.GetSize() == 6);

    ME::ByteReader reader(buffer, writer.GetSize());
    char out[GP::CHAT_CAPACITY] = {};
    ASSERT(GP::ReadString(reader, out, sizeof(out)));
    EXPECT(std::strcmp(out, "hello") == 0);
}

TEST(GameProtocol, StringTooBigForCapacityIsRejected) {
    uint8_t buffer[64] = {};
    ME::ByteWriter writer(buffer, sizeof(buffer));
    GP::WriteString(writer, "hello");

    ME::ByteReader reader(buffer, writer.GetSize());
    char out[5] = {'x', 0, 0, 0, 0};
    // 5 characters + terminating 0 needs 6 bytes.
    EXPECT(!GP::ReadString(reader, out, sizeof(out)));
    EXPECT(out[0] == 'x');
}

TEST(GameProtocol, TruncatedStringIsRejected) {
    const uint8_t data[3] = {10, 'a', 'b'};
    ME::ByteReader reader(data, sizeof(data));
    char out[32] = {};

    EXPECT(!GP::ReadString(reader, out, sizeof(out)));
}

TEST(GameProtocol, VerbNames) {
    EXPECT(std::strcmp(GP::GetVerbName(static_cast<uint8_t>(GP::Verb::PING)), "PING") == 0);
    EXPECT(std::strcmp(GP::GetVerbName(0x7F), "UNKNOWN") == 0);
    EXPECT(std::strcmp(GP::GetVerbName(GP::FIRST_GAME_VERB), "GAME") == 0);
    EXPECT(std::strcmp(GP::GetVerbName(0xFF), "GAME") == 0);
}

TEST(GameProtocol, GameVerbHeaderRoundTrip) {
    uint8_t buffer[GP::MAX_DATAGRAM_SIZE] = {};
    ME::ByteWriter writer(buffer, sizeof(buffer));
    const uint8_t gameVerb = GP::FIRST_GAME_VERB + 3;
    ASSERT(GP::WriteHeader(writer, gameVerb, 9));
    EXPECT(writer.GetSize() == 3);

    ME::ByteReader reader(buffer, writer.GetSize());
    GP::Header header;
    ASSERT(GP::ReadHeader(reader, header));
    EXPECT(header.verb == gameVerb);
    EXPECT(header.clientID == 9);
}
