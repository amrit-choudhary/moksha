#ifdef ME_WIN

#include "socket_platform.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>

// SIO_UDP_CONNRESET. Needs winsock2.h first; kept in its own block so include sorting can't move it above.
#include <mswsock.h>

#include "moksha/logging/logging.h"

namespace {
SOCKET ToNative(intptr_t s) {
    return static_cast<SOCKET>(s);
}
}  // namespace

bool ME::Net::SocketPlatform::Init() {
    WSADATA wsaData;
    const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        LogError("WSAStartup failed: ", result);
        return false;
    }
    return true;
}

void ME::Net::SocketPlatform::Shutdown() {
    WSACleanup();
}

intptr_t ME::Net::SocketPlatform::CreateTcp() {
    const SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return s == INVALID_SOCKET ? INVALID_HANDLE : static_cast<intptr_t>(s);
}

intptr_t ME::Net::SocketPlatform::CreateUdp() {
    const SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    return s == INVALID_SOCKET ? INVALID_HANDLE : static_cast<intptr_t>(s);
}

void ME::Net::SocketPlatform::Close(intptr_t s) {
    closesocket(ToNative(s));
}

bool ME::Net::SocketPlatform::SetNonBlocking(intptr_t s) {
    u_long nonBlocking = 1;
    return ioctlsocket(ToNative(s), FIONBIO, &nonBlocking) == 0;
}

void ME::Net::SocketPlatform::SetListenOptions(intptr_t s) {
    // Stop another program from binding the same port while we listen. (Windows' SO_REUSEADDR would allow it.)
    BOOL exclusive = TRUE;
    setsockopt(ToNative(s), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char*>(&exclusive),
               sizeof(exclusive));
}

void ME::Net::SocketPlatform::SetConnectionOptions(intptr_t) {
    // Windows never raises SIGPIPE, so nothing to set.
}

void ME::Net::SocketPlatform::SetDatagramOptions(intptr_t s) {
    // Stop another program from binding the same port and receiving our datagrams.
    BOOL exclusive = TRUE;
    setsockopt(ToNative(s), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char*>(&exclusive),
               sizeof(exclusive));

    // Sending to a port nobody listens on makes Windows fail the next recvfrom with WSAECONNRESET, even
    // though the socket is fine. Turn that report off; POSIX never reports it on unconnected UDP sockets.
    BOOL reportReset = FALSE;
    DWORD bytesReturned = 0;
    WSAIoctl(ToNative(s), SIO_UDP_CONNRESET, &reportReset, sizeof(reportReset), nullptr, 0, &bytesReturned, nullptr,
             nullptr);
}

bool ME::Net::SocketPlatform::BindAny(intptr_t s, uint16_t port) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    return bind(ToNative(s), reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0;
}

bool ME::Net::SocketPlatform::Listen(intptr_t s) {
    return listen(ToNative(s), SOMAXCONN) == 0;
}

intptr_t ME::Net::SocketPlatform::Accept(intptr_t s) {
    const SOCKET client = accept(ToNative(s), nullptr, nullptr);
    return client == INVALID_SOCKET ? INVALID_HANDLE : static_cast<intptr_t>(client);
}

int ME::Net::SocketPlatform::Send(intptr_t s, const uint8_t* data, int size) {
    return send(ToNative(s), reinterpret_cast<const char*>(data), size, 0);
}

int ME::Net::SocketPlatform::Recv(intptr_t s, uint8_t* buffer, int capacity) {
    return recv(ToNative(s), reinterpret_cast<char*>(buffer), capacity, 0);
}

bool ME::Net::SocketPlatform::ParseIPv4(const char* text, uint32_t& outIp) {
    in_addr address{};
    // inet_pton turns the text address into 4 bytes in network byte order; returns 1 only for valid IPv4.
    if (inet_pton(AF_INET, text, &address) != 1) {
        return false;
    }
    outIp = ntohl(address.s_addr);
    return true;
}

int ME::Net::SocketPlatform::SendTo(intptr_t s, const uint8_t* data, int size, uint32_t ip, uint16_t port) {
    sockaddr_in to{};
    to.sin_family = AF_INET;
    to.sin_port = htons(port);
    to.sin_addr.s_addr = htonl(ip);
    return sendto(ToNative(s), reinterpret_cast<const char*>(data), size, 0, reinterpret_cast<sockaddr*>(&to),
                  sizeof(to));
}

int ME::Net::SocketPlatform::RecvFrom(intptr_t s, uint8_t* buffer, int capacity, uint32_t& outIp, uint16_t& outPort) {
    sockaddr_in from{};
    int fromLength = sizeof(from);
    int received = recvfrom(ToNative(s), reinterpret_cast<char*>(buffer), capacity, 0,
                            reinterpret_cast<sockaddr*>(&from), &fromLength);

    // A datagram bigger than the buffer fails with WSAEMSGSIZE, but the buffer is still filled with its
    // first bytes. POSIX returns those bytes as a normal receive; do the same here.
    if (received == SOCKET_ERROR && WSAGetLastError() == WSAEMSGSIZE) {
        received = capacity;
    }

    if (received >= 0) {
        outIp = ntohl(from.sin_addr.s_addr);
        outPort = ntohs(from.sin_port);
    }
    return received;
}

ME::Net::SocketPlatform::ConnectState ME::Net::SocketPlatform::StartConnect(intptr_t s, const char* ip, uint16_t port) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    // inet_pton turns the text address into 4 bytes; returns 1 only for a valid IPv4 address.
    if (inet_pton(AF_INET, ip, &address.sin_addr) != 1) {
        WSASetLastError(WSAEINVAL);
        return ConnectState::Failed;
    }

    if (connect(ToNative(s), reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0) {
        return ConnectState::Connected;
    }
    // A non-blocking connect reports "still working" as WSAEWOULDBLOCK on Windows.
    return WSAGetLastError() == WSAEWOULDBLOCK ? ConnectState::InProgress : ConnectState::Failed;
}

ME::Net::SocketPlatform::ConnectState ME::Net::SocketPlatform::PollConnect(intptr_t s, int& outError) {
    const SOCKET native = ToNative(s);

    // select() with a zero timeout asks "is it ready?" without waiting. Windows reports a finished
    // connect as writable, and a failed one in the error set (not as writable, unlike POSIX).
    fd_set writeSet;
    fd_set errorSet;
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);
    FD_SET(native, &writeSet);
    FD_SET(native, &errorSet);
    timeval noWait{0, 0};

    if (select(0, nullptr, &writeSet, &errorSet, &noWait) == SOCKET_ERROR) {
        outError = WSAGetLastError();
        return ConnectState::Failed;
    }

    if (FD_ISSET(native, &errorSet)) {
        // SO_ERROR holds why the connect failed, e.g. WSAECONNREFUSED.
        int error = 0;
        int length = sizeof(error);
        getsockopt(native, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &length);
        outError = error;
        return ConnectState::Failed;
    }
    return FD_ISSET(native, &writeSet) ? ConnectState::Connected : ConnectState::InProgress;
}

int ME::Net::SocketPlatform::LastError() {
    return WSAGetLastError();
}

bool ME::Net::SocketPlatform::LastErrorIsWouldBlock() {
    return WSAGetLastError() == WSAEWOULDBLOCK;
}

#endif  // ME_WIN
