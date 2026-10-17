#include "udp_socket.h"

#include "moksha/logging/logging.h"
#include "moksha/shared/net/socket_platform.h"

ME::Net::UdpSocket::~UdpSocket() {
    Close();
}

bool ME::Net::UdpSocket::Open(uint16_t port) {
    Close();

    const intptr_t s = SocketPlatform::CreateUdp();
    if (s == SocketPlatform::INVALID_HANDLE) {
        LogError("Creating UDP socket failed: ", SocketPlatform::LastError());
        return false;
    }

    // Options must be set before bind: exclusive port use only takes effect at bind time.
    SocketPlatform::SetDatagramOptions(s);

    // Bind even for port 0, so the socket has a port and can receive before its first send.
    if (!SocketPlatform::BindAny(s, port)) {
        LogError("Binding UDP port ", port, " failed: ", SocketPlatform::LastError());
        SocketPlatform::Close(s);
        return false;
    }

    if (!SocketPlatform::SetNonBlocking(s)) {
        LogError("Setting UDP socket non-blocking failed: ", SocketPlatform::LastError());
        SocketPlatform::Close(s);
        return false;
    }

    handle = s;
    return true;
}

ME::Net::UdpResult ME::Net::UdpSocket::SendTo(const uint8_t* data, int size, const Address& to) {
    // UDP sends a datagram whole or not at all, so any non-negative result means it was sent.
    if (SocketPlatform::SendTo(handle, data, size, to.ip, to.port) >= 0) {
        return UdpResult::Ok;
    }
    return SocketPlatform::LastErrorIsWouldBlock() ? UdpResult::WouldBlock : UdpResult::Error;
}

ME::Net::UdpResult ME::Net::UdpSocket::RecvFrom(uint8_t* buffer, int capacity, int& outReceived, Address& outFrom) {
    outReceived = 0;
    uint32_t fromIp = 0;
    uint16_t fromPort = 0;
    const int received = SocketPlatform::RecvFrom(handle, buffer, capacity, fromIp, fromPort);
    if (received >= 0) {
        outReceived = received;
        outFrom.ip = fromIp;
        outFrom.port = fromPort;
        return UdpResult::Ok;
    }
    return SocketPlatform::LastErrorIsWouldBlock() ? UdpResult::WouldBlock : UdpResult::Error;
}

void ME::Net::UdpSocket::Close() {
    if (handle != SocketPlatform::INVALID_HANDLE) {
        SocketPlatform::Close(handle);
        handle = SocketPlatform::INVALID_HANDLE;
    }
}

bool ME::Net::UdpSocket::IsOpen() const {
    return handle != SocketPlatform::INVALID_HANDLE;
}
