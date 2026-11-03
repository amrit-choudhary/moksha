#ifdef ME_WEB

/**
 * Web has no raw sockets, so every socket fails to open and the network clients stay offline. Address parsing still
 * works, so a config's address isn't reported as invalid.
 */

#include <arpa/inet.h>

#include "socket_platform.h"

namespace {
constexpr int NO_SOCKETS_ERROR = -1;
}  // namespace

bool ME::Net::SocketPlatform::Init() {
    return true;
}

void ME::Net::SocketPlatform::Shutdown() {}

intptr_t ME::Net::SocketPlatform::CreateTcp() {
    return INVALID_HANDLE;
}

intptr_t ME::Net::SocketPlatform::CreateUdp() {
    return INVALID_HANDLE;
}

void ME::Net::SocketPlatform::Close(intptr_t) {}

bool ME::Net::SocketPlatform::SetNonBlocking(intptr_t) {
    return false;
}

void ME::Net::SocketPlatform::SetListenOptions(intptr_t) {}

void ME::Net::SocketPlatform::SetConnectionOptions(intptr_t) {}

void ME::Net::SocketPlatform::SetDatagramOptions(intptr_t) {}

bool ME::Net::SocketPlatform::BindAny(intptr_t, uint16_t) {
    return false;
}

bool ME::Net::SocketPlatform::Listen(intptr_t) {
    return false;
}

intptr_t ME::Net::SocketPlatform::Accept(intptr_t) {
    return INVALID_HANDLE;
}

int ME::Net::SocketPlatform::Send(intptr_t, const uint8_t*, int) {
    return -1;
}

int ME::Net::SocketPlatform::Recv(intptr_t, uint8_t*, int) {
    return -1;
}

bool ME::Net::SocketPlatform::ParseIPv4(const char* text, uint32_t& outIp) {
    in_addr address{};
    if (inet_pton(AF_INET, text, &address) != 1) {
        return false;
    }
    outIp = ntohl(address.s_addr);
    return true;
}

int ME::Net::SocketPlatform::SendTo(intptr_t, const uint8_t*, int, uint32_t, uint16_t) {
    return -1;
}

int ME::Net::SocketPlatform::RecvFrom(intptr_t, uint8_t*, int, uint32_t&, uint16_t&) {
    return -1;
}

ME::Net::SocketPlatform::ConnectState ME::Net::SocketPlatform::StartConnect(intptr_t, const char*, uint16_t) {
    return ConnectState::Failed;
}

ME::Net::SocketPlatform::ConnectState ME::Net::SocketPlatform::PollConnect(intptr_t, int& outError) {
    outError = NO_SOCKETS_ERROR;
    return ConnectState::Failed;
}

int ME::Net::SocketPlatform::LastError() {
    return NO_SOCKETS_ERROR;
}

bool ME::Net::SocketPlatform::LastErrorIsWouldBlock() {
    return false;
}

#endif  // ME_WEB
