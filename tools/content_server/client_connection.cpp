#include "client_connection.h"

#include "content_store.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/net/content_manifest.h"
#include "moksha/shared/net/content_protocol.h"
#include "moksha/shared/serialization/byte_reader.h"
#include "moksha/shared/serialization/byte_writer.h"

using ME::Net::ContentProtocol::Verb;

ME::ClientConnection::ClientConnection()
    : receiver(recvStorage, sizeof(recvStorage), Net::ContentProtocol::MAX_MESSAGE_SIZE) {}

bool ME::ClientConnection::AcceptFrom(Net::TcpSocket& listener, uint32_t clientId, double now) {
    if (listener.Accept(socket) != Net::TcpResult::Ok) {
        return false;
    }

    id = clientId;
    lastActivity = now;
    receiver.Reset();
    headerSize = headerSent = 0;
    body = nullptr;
    bodySize = bodySent = 0;
    closeAfterSend = false;
    LogInfo("Client ", id, ": connected");
    return true;
}

bool ME::ClientConnection::Update(const ContentStore& store, double now) {
    bool didWork = false;

    // 1. Continue sending the current reply.
    if (!FlushSend(didWork)) {
        LogError("Client ", id, ": send failed, dropping");
        Close();
        return true;
    }

    // 2. Receive more request bytes. Skipped when the buffer is full: Recv with 0 capacity would return 0,
    //    which looks exactly like "connection closed".
    if (!closeAfterSend && receiver.GetFreeSize() > 0) {
        int received = 0;
        const Net::TcpResult result =
            socket.Recv(receiver.GetFreeData(), static_cast<int>(receiver.GetFreeSize()), received);
        if (result == Net::TcpResult::Ok) {
            receiver.CommitReceived(static_cast<size_t>(received));
            didWork = true;
        } else if (result == Net::TcpResult::Closed) {
            // The client is done sending, but may still be reading our last reply.
            closeAfterSend = true;
            didWork = true;
        } else if (result == Net::TcpResult::Error) {
            LogError("Client ", id, ": recv failed, dropping");
            Close();
            return true;
        }
    }

    // 3. Answer the next complete request, one at a time: only when no reply is still being sent.
    Net::FrameView request;
    while (!HasPendingSend()) {
        const Net::FrameResult result = receiver.PeekFrame(request);
        if (result == Net::FrameResult::Incomplete) {
            break;
        }
        if (result == Net::FrameResult::Invalid) {
            LogError("Client ", id, ": invalid frame length, dropping");
            Close();
            return true;
        }

        // request.payload points into the receiver, so handle it before PopFrame.
        const bool ok = HandleRequest(request, store);
        receiver.PopFrame();
        didWork = true;
        if (!ok) {
            Close();
            return true;
        }
    }

    // 4. Start sending the new reply right away.
    if (!FlushSend(didWork)) {
        LogError("Client ", id, ": send failed, dropping");
        Close();
        return true;
    }

    if (closeAfterSend && !HasPendingSend()) {
        LogInfo("Client ", id, ": disconnected");
        Close();
        return true;
    }

    // Any progress (bytes in or out, a request handled) counts as activity. A client that neither sends
    // nor reads its reply is holding a slot for nothing.
    if (didWork) {
        lastActivity = now;
    } else if (now - lastActivity > IDLE_TIMEOUT_SECONDS) {
        LogWarning("Client ", id, ": idle for ", IDLE_TIMEOUT_SECONDS, " s, disconnecting");
        Close();
        return true;
    }
    return didWork;
}

bool ME::ClientConnection::IsOpen() const {
    return socket.IsOpen();
}

void ME::ClientConnection::Close() {
    socket.Close();
}

bool ME::ClientConnection::HandleRequest(const Net::FrameView& request, const ContentStore& store) {
    if (request.version != Net::ContentProtocol::VERSION) {
        LogError("Client ", id, ": unsupported protocol version ", static_cast<int>(request.version));
        return false;
    }

    switch (static_cast<Verb>(request.verb)) {
        case Verb::GET_MANIFEST:
            return QueueManifest(store);

        case Verb::GET_FILE: {
            // Payload: u16 pathLen, path, u32 rangeOffset, u32 rangeLength (range reserved, ignored).
            ByteReader reader(request.payload, request.payloadSize);
            uint16_t pathLength = 0;
            const uint8_t* path = nullptr;
            uint32_t rangeOffset = 0;
            uint32_t rangeLength = 0;
            if (!reader.ReadU16(pathLength) || pathLength > Net::ContentManifest::MAX_PATH_LENGTH ||
                !reader.ReadBytes(pathLength, path) || !reader.ReadU32(rangeOffset) || !reader.ReadU32(rangeLength)) {
                LogError("Client ", id, ": malformed GET_FILE request");
                return false;
            }
            return QueueFile(std::string(reinterpret_cast<const char*>(path), pathLength), store);
        }

        default:
            LogError("Client ", id, ": unexpected request ", Net::ContentProtocol::GetVerbName(request.verb));
            return false;
    }
}

bool ME::ClientConnection::QueueManifest(const ContentStore& store) {
    const std::string& text = store.GetManifestText();

    ByteWriter writer(headerStorage, sizeof(headerStorage));
    size_t frameStart = 0;
    if (!Net::BeginFrame(writer, Net::ContentProtocol::VERSION, static_cast<uint8_t>(Verb::MANIFEST), frameStart) ||
        !Net::FinishFrame(writer, frameStart, text.size())) {
        return false;
    }

    headerSize = writer.GetSize();
    headerSent = 0;
    body = reinterpret_cast<const uint8_t*>(text.data());
    bodySize = text.size();
    bodySent = 0;

    LogInfo("Client ", id, ": GET_MANIFEST -> MANIFEST (", bodySize, " bytes)");
    return true;
}

bool ME::ClientConnection::QueueFile(const std::string& path, const ContentStore& store) {
    // Only files listed in the manifest and loaded at startup can be found, so a path can never reach the disk.
    const StoredFile* file = store.FindFile(path.c_str());
    const Verb replyVerb = file != nullptr ? Verb::FILE : Verb::FILE_NOT_FOUND;
    const size_t bodyBytes = file != nullptr ? file->size : 0;

    ByteWriter writer(headerStorage, sizeof(headerStorage));
    size_t frameStart = 0;
    if (!Net::BeginFrame(writer, Net::ContentProtocol::VERSION, static_cast<uint8_t>(replyVerb), frameStart) ||
        !writer.WriteU16(static_cast<uint16_t>(path.size())) ||
        !writer.WriteBytes(reinterpret_cast<const uint8_t*>(path.data()), path.size()) ||
        !Net::FinishFrame(writer, frameStart, bodyBytes)) {
        return false;
    }

    headerSize = writer.GetSize();
    headerSent = 0;
    body = file != nullptr ? file->data : nullptr;
    bodySize = bodyBytes;
    bodySent = 0;

    if (file != nullptr) {
        LogInfo("Client ", id, ": GET_FILE ", path, " -> FILE (", bodySize, " bytes)");
    } else {
        LogWarning("Client ", id, ": GET_FILE ", path, " -> FILE_NOT_FOUND");
    }
    return true;
}

bool ME::ClientConnection::FlushSend(bool& outDidWork) {
    // Header first, then body. Stop as soon as the OS takes fewer bytes than offered: its send buffer is full.
    while (headerSent < headerSize) {
        int sent = 0;
        const Net::TcpResult result =
            socket.Send(headerStorage + headerSent, static_cast<int>(headerSize - headerSent), sent);
        if (result == Net::TcpResult::Error) {
            return false;
        }
        if (result == Net::TcpResult::WouldBlock || sent == 0) {
            return true;
        }
        headerSent += static_cast<size_t>(sent);
        outDidWork = true;
    }

    while (bodySent < bodySize) {
        int sent = 0;
        const Net::TcpResult result = socket.Send(body + bodySent, static_cast<int>(bodySize - bodySent), sent);
        if (result == Net::TcpResult::Error) {
            return false;
        }
        if (result == Net::TcpResult::WouldBlock || sent == 0) {
            return true;
        }
        bodySent += static_cast<size_t>(sent);
        outDidWork = true;
    }
    return true;
}

bool ME::ClientConnection::HasPendingSend() const {
    return headerSent < headerSize || bodySent < bodySize;
}
