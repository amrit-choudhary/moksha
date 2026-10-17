#include "message_framing.h"

#include <cstring>

#include "moksha/shared/serialization/byte_reader.h"
#include "moksha/shared/serialization/byte_writer.h"

namespace {
// Smallest legal length field: version + verb, empty payload.
constexpr uint32_t MIN_LENGTH = 2;
}  // namespace

bool ME::Net::BeginFrame(ByteWriter& writer, uint8_t version, uint8_t verb, size_t& outFrameStart) {
    outFrameStart = writer.GetSize();
    return writer.WriteU32(0) && writer.WriteU8(version) && writer.WriteU8(verb);
}

bool ME::Net::FinishFrame(ByteWriter& writer, size_t frameStart, size_t externalPayloadSize) {
    const size_t frameSize = writer.GetSize() - frameStart + externalPayloadSize;
    return writer.PatchU32(frameStart, static_cast<uint32_t>(frameSize - FRAME_LENGTH_SIZE));
}

ME::Net::FrameReceiver::FrameReceiver(uint8_t* storage, size_t capacity, uint32_t maxFrameSize)
    : storage(storage), capacity(capacity), maxFrameSize(maxFrameSize) {}

uint8_t* ME::Net::FrameReceiver::GetFreeData() {
    return storage + used;
}

size_t ME::Net::FrameReceiver::GetFreeSize() const {
    return capacity - used;
}

void ME::Net::FrameReceiver::CommitReceived(size_t count) {
    used += count;
}

ME::Net::FrameResult ME::Net::FrameReceiver::PeekFrame(FrameView& out) const {
    ByteReader reader(storage, used);

    uint32_t length = 0;
    if (!reader.ReadU32(length)) {
        return FrameResult::Incomplete;
    }

    // 64-bit math so a huge length can't wrap around when the length field size is added.
    const uint64_t frameSize = static_cast<uint64_t>(FRAME_LENGTH_SIZE) + length;
    if (length < MIN_LENGTH || frameSize > maxFrameSize || frameSize > capacity) {
        return FrameResult::Invalid;
    }

    if (used < frameSize) {
        return FrameResult::Incomplete;
    }

    const uint32_t payloadSize = length - MIN_LENGTH;
    reader.ReadU8(out.version);
    reader.ReadU8(out.verb);
    reader.ReadBytes(payloadSize, out.payload);
    out.payloadSize = payloadSize;
    return FrameResult::Ok;
}

void ME::Net::FrameReceiver::PopFrame() {
    FrameView frame;
    if (PeekFrame(frame) != FrameResult::Ok) {
        return;
    }

    const size_t frameSize = FRAME_HEADER_SIZE + frame.payloadSize;
    // memmove, not memcpy: source and destination overlap when shifting within one buffer.
    std::memmove(storage, storage + frameSize, used - frameSize);
    used -= frameSize;
}

void ME::Net::FrameReceiver::Reset() {
    used = 0;
}
