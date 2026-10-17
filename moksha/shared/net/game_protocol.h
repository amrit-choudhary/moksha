/**
 * Wire protocol between the game server and its clients (over UDP). One datagram is one message:
 * u8 version | u8 verb | u8 clientID | payload. Integers are in native byte order.
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace ME {

class ByteReader;
class ByteWriter;
class FP_24_8;

namespace Net::GameProtocol {

constexpr uint8_t VERSION = 0;

// Largest datagram sent or accepted. Small enough to cross any network without being split into fragments.
constexpr int MAX_DATAGRAM_SIZE = 1200;

// Longest string WriteString can send: its length goes in one byte.
constexpr size_t MAX_STRING_LENGTH = 255;

// Buffer size for a chat message, including the terminating 0 (so 63 characters). Longer ones are not sent
// and are dropped on receive.
constexpr size_t CHAT_CAPACITY = 64;

// First verb a game may define for its own messages; 0x80-0xFF belong to the game, the rest to the engine.
constexpr uint8_t FIRST_GAME_VERB = 0x80;

/**
 * Engine verbs. Ranges: 0x00-0x1F system, 0x20-0x3F http, 0x40-0x5F matchmaking, 0x60-0x7F gameplay,
 * 0x80-0xFF game-defined (see FIRST_GAME_VERB).
 */
enum class Verb : uint8_t {
    // System
    NONE = 0x00,
    ACK = 0x01,
    AUTH = 0x02,
    PING = 0x03,
    PONG = 0x04,
    DATA = 0x05,
    CONNECT = 0x06,
    CONNECTED = 0x07,
    DISCONNECT = 0x08,

    // Http
    GET = 0x20,

    // Gameplay
    CHAT_SEND = 0x60,
    CHAT_RECV = 0x61,
    DATA_SEND = 0x62,
    DATA_RECV = 0x63,
};

/**
 * The 3 bytes at the start of every datagram. verb is the raw byte: a buggy or hostile sender can put any value.
 */
class Header {
   public:
    uint8_t version = 0;
    uint8_t verb = 0;
    uint8_t clientID = 0;
};

/**
 * Writes VERSION, verb and clientID. False if the writer has no room.
 */
bool WriteHeader(ByteWriter& writer, Verb verb, uint8_t clientID);

// Same, for a game-defined verb (FIRST_GAME_VERB and up) given as its raw byte.
bool WriteHeader(ByteWriter& writer, uint8_t verb, uint8_t clientID);

/**
 * False, leaving out unchanged, if the datagram is shorter than a header.
 */
bool ReadHeader(ByteReader& reader, Header& out);

/**
 * FP_24_8 travels as its raw 32-bit value, so both ends get exactly the same number.
 */
bool WriteFP(ByteWriter& writer, const FP_24_8& value);
bool ReadFP(ByteReader& reader, FP_24_8& out);

/**
 * Text travels as u8 length + bytes (no terminating 0), so at most MAX_STRING_LENGTH bytes. ReadString adds
 * the 0 and fails, leaving out unchanged, if the text and its 0 don't fit in capacity or the datagram is short.
 */
bool WriteString(ByteWriter& writer, const char* text);
bool ReadString(ByteReader& reader, char* out, size_t capacity);

/**
 * Readable name for logs; "GAME" for game-defined verbs, "UNKNOWN" for other values outside the enum.
 */
const char* GetVerbName(uint8_t verb);

}  // namespace Net::GameProtocol
}  // namespace ME
