/**
 * Message framing for TCP: TCP is a byte stream, so each message is prefixed with its length.
 * Frame layout: u32 length | u8 version | u8 verb | payload. length counts the bytes after itself.
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace ME {

class ByteWriter;

namespace Net {

constexpr uint32_t FRAME_LENGTH_SIZE = 4;
constexpr uint32_t FRAME_HEADER_SIZE = FRAME_LENGTH_SIZE + 1 + 1;

/**
 * Writes a frame header with a placeholder length; outFrameStart remembers where it began.
 * Write the payload next, then call FinishFrame to fill in the real length.
 */
bool BeginFrame(ByteWriter& writer, uint8_t version, uint8_t verb, size_t& outFrameStart);

/**
 * externalPayloadSize: payload bytes that are not in the writer but sent right after it (e.g. file data
 * sent straight from memory, avoiding a copy). They are counted in the length field.
 */
bool FinishFrame(ByteWriter& writer, size_t frameStart, size_t externalPayloadSize = 0);

/**
 * One received frame. payload points into the receiver's storage and is valid until PopFrame.
 */
class FrameView {
   public:
    uint8_t version = 0;
    uint8_t verb = 0;
    const uint8_t* payload = nullptr;
    uint32_t payloadSize = 0;
};

enum class FrameResult : uint8_t {
    Ok,          // A complete frame is ready.
    Incomplete,  // Not enough bytes yet; receive more.
    Invalid,     // Length field is impossible or too large; close the connection.
};

/**
 * Collects received stream bytes in caller-owned storage and splits them into whole frames.
 * Use: Recv into GetFreeData(), CommitReceived(n), then PeekFrame / PopFrame until Incomplete.
 */
class FrameReceiver {
   public:
    FrameReceiver(uint8_t* storage, size_t capacity, uint32_t maxFrameSize);

    uint8_t* GetFreeData();
    size_t GetFreeSize() const;
    void CommitReceived(size_t count);

    FrameResult PeekFrame(FrameView& out) const;

    /**
     * Drops the frame PeekFrame returned and moves any following bytes to the front.
     */
    void PopFrame();

    void Reset();

   private:
    uint8_t* storage = nullptr;
    size_t capacity = 0;
    uint32_t maxFrameSize = 0;
    size_t used = 0;
};

}  // namespace Net
}  // namespace ME
