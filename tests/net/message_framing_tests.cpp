/**
 * Tests for TCP message framing: BeginFrame/FinishFrame and FrameReceiver.
 */

#include <cstdint>
#include <cstring>

#include "moksha/shared/net/message_framing.h"
#include "moksha/shared/serialization/byte_writer.h"
#include "moksha/test_framework/test_framework.h"

using ME::Net::FrameReceiver;
using ME::Net::FrameResult;
using ME::Net::FrameView;

// Writes one frame with the given payload into buffer; returns its total size.
static size_t WriteFrame(uint8_t* buffer, size_t capacity, uint8_t verb, const char* payload) {
    ME::ByteWriter writer(buffer, capacity);
    size_t frameStart = 0;
    ME::Net::BeginFrame(writer, 1, verb, frameStart);
    writer.WriteBytes(reinterpret_cast<const uint8_t*>(payload), std::strlen(payload));
    ME::Net::FinishFrame(writer, frameStart);
    return writer.GetSize();
}

static void Receive(FrameReceiver& receiver, const uint8_t* data, size_t count) {
    std::memcpy(receiver.GetFreeData(), data, count);
    receiver.CommitReceived(count);
}

TEST(MessageFraming, LengthFieldCountsBytesAfterItself) {
    uint8_t buffer[32] = {};
    size_t size = WriteFrame(buffer, sizeof(buffer), 2, "abc");

    EXPECT(size == ME::Net::FRAME_HEADER_SIZE + 3);
    uint32_t length = 0;
    std::memcpy(&length, buffer, sizeof(length));
    EXPECT(length == size - ME::Net::FRAME_LENGTH_SIZE);
}

TEST(MessageFraming, ExternalPayloadCountsInLength) {
    uint8_t buffer[32] = {};
    ME::ByteWriter writer(buffer, sizeof(buffer));
    size_t frameStart = 0;
    ME::Net::BeginFrame(writer, 1, 2, frameStart);
    ME::Net::FinishFrame(writer, frameStart, 100);

    uint32_t length = 0;
    std::memcpy(&length, buffer, sizeof(length));
    EXPECT(length == 2 + 100);
}

TEST(FrameReceiver, WholeFrame) {
    uint8_t frame[32] = {};
    size_t size = WriteFrame(frame, sizeof(frame), 7, "hello");

    uint8_t storage[64] = {};
    FrameReceiver receiver(storage, sizeof(storage), sizeof(storage));
    Receive(receiver, frame, size);

    FrameView view;
    ASSERT(receiver.PeekFrame(view) == FrameResult::Ok);
    EXPECT(view.version == 1);
    EXPECT(view.verb == 7);
    EXPECT(view.payloadSize == 5);
    EXPECT(std::memcmp(view.payload, "hello", 5) == 0);

    receiver.PopFrame();
    EXPECT(receiver.PeekFrame(view) == FrameResult::Incomplete);
    EXPECT(receiver.GetFreeSize() == sizeof(storage));
}

TEST(FrameReceiver, FrameSplitAcrossReceives) {
    uint8_t frame[32] = {};
    size_t size = WriteFrame(frame, sizeof(frame), 7, "hello");

    uint8_t storage[64] = {};
    FrameReceiver receiver(storage, sizeof(storage), sizeof(storage));
    FrameView view;

    // Partial length field, then partial payload, then the rest.
    Receive(receiver, frame, 2);
    EXPECT(receiver.PeekFrame(view) == FrameResult::Incomplete);
    Receive(receiver, frame + 2, 6);
    EXPECT(receiver.PeekFrame(view) == FrameResult::Incomplete);
    Receive(receiver, frame + 8, size - 8);
    EXPECT(receiver.PeekFrame(view) == FrameResult::Ok);
}

TEST(FrameReceiver, TwoFramesInOneReceive) {
    uint8_t frames[64] = {};
    size_t first = WriteFrame(frames, sizeof(frames), 1, "one");
    size_t second = WriteFrame(frames + first, sizeof(frames) - first, 2, "two!");

    uint8_t storage[64] = {};
    FrameReceiver receiver(storage, sizeof(storage), sizeof(storage));
    Receive(receiver, frames, first + second);

    FrameView view;
    ASSERT(receiver.PeekFrame(view) == FrameResult::Ok);
    EXPECT(view.verb == 1);
    receiver.PopFrame();

    ASSERT(receiver.PeekFrame(view) == FrameResult::Ok);
    EXPECT(view.verb == 2);
    EXPECT(view.payloadSize == 4);
    EXPECT(std::memcmp(view.payload, "two!", 4) == 0);
    receiver.PopFrame();

    EXPECT(receiver.PeekFrame(view) == FrameResult::Incomplete);
}

TEST(FrameReceiver, ImpossibleLengthIsInvalid) {
    uint8_t storage[64] = {};
    FrameReceiver receiver(storage, sizeof(storage), 32);
    FrameView view;

    // Length below version + verb.
    const uint8_t tooSmall[4] = {1, 0, 0, 0};
    Receive(receiver, tooSmall, sizeof(tooSmall));
    EXPECT(receiver.PeekFrame(view) == FrameResult::Invalid);

    // Length above maxFrameSize.
    receiver.Reset();
    const uint32_t tooBig = 1000;
    Receive(receiver, reinterpret_cast<const uint8_t*>(&tooBig), sizeof(tooBig));
    EXPECT(receiver.PeekFrame(view) == FrameResult::Invalid);
}
