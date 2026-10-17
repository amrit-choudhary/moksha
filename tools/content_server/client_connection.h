/**
 * One connected content client: its socket, receive buffer and the reply being sent.
 * Handles one request at a time: the next request is read only after the current reply is fully sent.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "moksha/shared/net/message_framing.h"
#include "moksha/shared/net/tcp_socket.h"

namespace ME {

class ContentStore;

class ClientConnection {
   public:
    // A client that sends nothing and accepts no reply bytes for this long is disconnected.
    static constexpr double IDLE_TIMEOUT_SECONDS = 30.0;

    ClientConnection();

    // The receiver points into this object's own storage, so it must never be copied or moved.
    ClientConnection(const ClientConnection&) = delete;
    ClientConnection& operator=(const ClientConnection&) = delete;

    /**
     * Takes one pending connection from listener. Returns false if nobody is waiting.
     * clientId is only used to tell clients apart in logs. now is the server's time in seconds.
     */
    bool AcceptFrom(Net::TcpSocket& listener, uint32_t clientId, double now);

    /**
     * Sends pending reply bytes, receives, and answers complete requests; closes the connection when idle
     * for longer than IDLE_TIMEOUT_SECONDS. Returns true if anything happened.
     */
    bool Update(const ContentStore& store, double now);

    bool IsOpen() const;
    void Close();

   private:
    bool HandleRequest(const Net::FrameView& request, const ContentStore& store);
    bool QueueManifest(const ContentStore& store);
    bool QueueFile(const std::string& path, const ContentStore& store);

    /**
     * Sends as much of the queued reply as the OS accepts right now. Returns false on a broken connection.
     */
    bool FlushSend(bool& outDidWork);
    bool HasPendingSend() const;

    // Requests are small; any frame larger than this is rejected as invalid.
    static constexpr size_t RECV_CAPACITY = 4096;
    // Reply header + u16 path length + longest allowed path, with room to spare.
    static constexpr size_t HEADER_CAPACITY = 512;

    Net::TcpSocket socket;
    uint32_t id = 0;
    double lastActivity = 0.0;

    uint8_t recvStorage[RECV_CAPACITY];
    Net::FrameReceiver receiver;

    // A reply is two parts: header bytes built here, then a body sent straight from ContentStore memory.
    uint8_t headerStorage[HEADER_CAPACITY];
    size_t headerSize = 0;
    size_t headerSent = 0;
    const uint8_t* body = nullptr;
    size_t bodySize = 0;
    size_t bodySent = 0;

    // The client finished sending; close once the current reply is fully sent.
    bool closeAfterSend = false;
};

}  // namespace ME
