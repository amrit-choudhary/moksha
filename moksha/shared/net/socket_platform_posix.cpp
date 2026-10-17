#if defined(ME_LINUX) || defined(ME_ANDROID) || defined(ME_MAC) || defined(ME_IOS)

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "socket_platform.h"

namespace {
int ToNative(intptr_t s) {
    return static_cast<int>(s);
}
}  // namespace

bool ME::Net::SocketPlatform::Init() {
    // POSIX sockets need no global setup.
    return true;
}

void ME::Net::SocketPlatform::Shutdown() {}

intptr_t ME::Net::SocketPlatform::CreateTcp() {
    const int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return s == -1 ? INVALID_HANDLE : static_cast<intptr_t>(s);
}

intptr_t ME::Net::SocketPlatform::CreateUdp() {
    const int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    return s == -1 ? INVALID_HANDLE : static_cast<intptr_t>(s);
}

void ME::Net::SocketPlatform::Close(intptr_t s) {
    close(ToNative(s));
}

bool ME::Net::SocketPlatform::SetNonBlocking(intptr_t s) {
    // Read the current flags and add O_NONBLOCK, so other flags are kept.
    const int flags = fcntl(ToNative(s), F_GETFL, 0);
    return flags != -1 && fcntl(ToNative(s), F_SETFL, flags | O_NONBLOCK) != -1;
}

void ME::Net::SocketPlatform::SetListenOptions(intptr_t s) {
    // Let a restarted server bind the port again right away, instead of waiting for old connections to expire.
    int reuse = 1;
    setsockopt(ToNative(s), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
}

void ME::Net::SocketPlatform::SetConnectionOptions(intptr_t s) {
#if defined(ME_MAC) || defined(ME_IOS)
    // Sending to a closed connection raises SIGPIPE, which kills the process; this makes it a normal error.
    int noSigPipe = 1;
    setsockopt(ToNative(s), SOL_SOCKET, SO_NOSIGPIPE, &noSigPipe, sizeof(noSigPipe));
#else
    // Linux and Android have no SO_NOSIGPIPE; Send uses MSG_NOSIGNAL instead.
    (void)s;
#endif
}

void ME::Net::SocketPlatform::SetDatagramOptions(intptr_t) {
    // A UDP port is exclusive by default on POSIX (no SO_REUSEADDR), and no ICMP reset errors are reported.
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
    const int client = accept(ToNative(s), nullptr, nullptr);
    return client == -1 ? INVALID_HANDLE : static_cast<intptr_t>(client);
}

int ME::Net::SocketPlatform::Send(intptr_t s, const uint8_t* data, int size) {
#if defined(ME_LINUX) || defined(ME_ANDROID)
    // Sending to a closed connection raises SIGPIPE, which kills the process; MSG_NOSIGNAL makes it a normal error.
    const int flags = MSG_NOSIGNAL;
#else
    const int flags = 0;  // Apple: handled by SO_NOSIGPIPE in SetConnectionOptions.
#endif
    return static_cast<int>(send(ToNative(s), data, static_cast<size_t>(size), flags));
}

int ME::Net::SocketPlatform::Recv(intptr_t s, uint8_t* buffer, int capacity) {
    return static_cast<int>(recv(ToNative(s), buffer, static_cast<size_t>(capacity), 0));
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
    return static_cast<int>(
        sendto(ToNative(s), data, static_cast<size_t>(size), 0, reinterpret_cast<sockaddr*>(&to), sizeof(to)));
}

int ME::Net::SocketPlatform::RecvFrom(intptr_t s, uint8_t* buffer, int capacity, uint32_t& outIp, uint16_t& outPort) {
    sockaddr_in from{};
    socklen_t fromLength = sizeof(from);
    // A datagram bigger than capacity is cut: recvfrom returns its first capacity bytes and drops the rest.
    const int received = static_cast<int>(recvfrom(ToNative(s), buffer, static_cast<size_t>(capacity), 0,
                                                   reinterpret_cast<sockaddr*>(&from), &fromLength));
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
        errno = EINVAL;
        return ConnectState::Failed;
    }

    if (connect(ToNative(s), reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0) {
        return ConnectState::Connected;
    }
    // A non-blocking connect reports "still working" as EINPROGRESS on POSIX.
    return errno == EINPROGRESS ? ConnectState::InProgress : ConnectState::Failed;
}

ME::Net::SocketPlatform::ConnectState ME::Net::SocketPlatform::PollConnect(intptr_t s, int& outError) {
    const int native = ToNative(s);
    // select's fd_set has a fixed size; a descriptor past it would overflow the set.
    if (native >= FD_SETSIZE) {
        outError = EINVAL;
        return ConnectState::Failed;
    }

    // select() with a zero timeout asks "is it ready?" without waiting. POSIX reports a finished
    // connect as writable whether it succeeded or failed; SO_ERROR tells which.
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(native, &writeSet);
    timeval noWait{0, 0};

    const int ready = select(native + 1, nullptr, &writeSet, nullptr, &noWait);
    if (ready < 0) {
        outError = errno;
        return ConnectState::Failed;
    }
    if (ready == 0) {
        return ConnectState::InProgress;
    }

    int error = 0;
    socklen_t length = sizeof(error);
    if (getsockopt(native, SOL_SOCKET, SO_ERROR, &error, &length) != 0) {
        outError = errno;
        return ConnectState::Failed;
    }
    if (error != 0) {
        outError = error;
        return ConnectState::Failed;
    }
    return ConnectState::Connected;
}

int ME::Net::SocketPlatform::LastError() {
    return errno;
}

bool ME::Net::SocketPlatform::LastErrorIsWouldBlock() {
    return errno == EAGAIN || errno == EWOULDBLOCK;
}

#endif  // ME_LINUX || ME_ANDROID || ME_MAC || ME_IOS
