/**
 * Non-blocking UDP socket shared by servers and clients. Each send / receive is one whole datagram, to or from
 * any address; there is no connection. A server opens a fixed port, a client lets the OS pick one.
 */
#pragma once

#include <cstdint>

#include "moksha/shared/net/net_address.h"

namespace ME::Net {

enum class UdpResult : uint8_t {
    Ok,          // One datagram sent or received.
    WouldBlock,  // Nothing to receive right now, or the send buffer is full. Try again later.
    Error,       // Call failed; the datagram is lost. The socket usually stays usable.
};

class UdpSocket {
   public:
    UdpSocket() = default;
    ~UdpSocket();

    // A socket owns an OS handle; copying would close it twice.
    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    /**
     * Opens and binds on all network interfaces. port 0 = the OS picks a free port (clients).
     */
    bool Open(uint16_t port = 0);

    UdpResult SendTo(const uint8_t* data, int size, const Address& to);

    /**
     * Receives one datagram into buffer; a datagram bigger than capacity is cut to capacity.
     * outReceived can be 0: an empty datagram is valid in UDP.
     */
    UdpResult RecvFrom(uint8_t* buffer, int capacity, int& outReceived, Address& outFrom);

    void Close();
    bool IsOpen() const;

   private:
    // OS socket handle: SOCKET on Windows, int file descriptor on POSIX. -1 (SocketPlatform::INVALID_HANDLE) = none.
    intptr_t handle = -1;
};

}  // namespace ME::Net
