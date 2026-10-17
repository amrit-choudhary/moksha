#include "tcp_socket.h"

#include "moksha/logging/logging.h"
#include "moksha/shared/net/socket_platform.h"

ME::Net::TcpSocket::~TcpSocket() {
    Close();
}

bool ME::Net::TcpSocket::Listen(uint16_t port) {
    Close();

    const intptr_t s = SocketPlatform::CreateTcp();
    if (s == SocketPlatform::INVALID_HANDLE) {
        LogError("Creating socket failed: ", SocketPlatform::LastError());
        return false;
    }

    SocketPlatform::SetListenOptions(s);

    if (!SocketPlatform::BindAny(s, port)) {
        LogError("Binding port ", port, " failed: ", SocketPlatform::LastError());
        SocketPlatform::Close(s);
        return false;
    }

    if (!SocketPlatform::Listen(s)) {
        LogError("Listening failed: ", SocketPlatform::LastError());
        SocketPlatform::Close(s);
        return false;
    }

    if (!SocketPlatform::SetNonBlocking(s)) {
        LogError("Setting listen socket non-blocking failed: ", SocketPlatform::LastError());
        SocketPlatform::Close(s);
        return false;
    }

    handle = s;
    return true;
}

ME::Net::TcpResult ME::Net::TcpSocket::Accept(TcpSocket& outClient) {
    const intptr_t client = SocketPlatform::Accept(handle);
    if (client == SocketPlatform::INVALID_HANDLE) {
        return SocketPlatform::LastErrorIsWouldBlock() ? TcpResult::WouldBlock : TcpResult::Error;
    }

    // Linux does not pass non-blocking mode from the listen socket to accepted ones, so set it explicitly.
    if (!SocketPlatform::SetNonBlocking(client)) {
        LogError("Setting client socket non-blocking failed: ", SocketPlatform::LastError());
        SocketPlatform::Close(client);
        return TcpResult::Error;
    }

    SocketPlatform::SetConnectionOptions(client);

    outClient.Close();
    outClient.handle = client;
    return TcpResult::Ok;
}

ME::Net::TcpResult ME::Net::TcpSocket::Connect(const char* ip, uint16_t port) {
    Close();

    const intptr_t s = SocketPlatform::CreateTcp();
    if (s == SocketPlatform::INVALID_HANDLE) {
        LogError("Creating socket failed: ", SocketPlatform::LastError());
        return TcpResult::Error;
    }

    // Non-blocking before connecting, so connect() returns at once instead of waiting for the handshake.
    if (!SocketPlatform::SetNonBlocking(s)) {
        LogError("Setting socket non-blocking failed: ", SocketPlatform::LastError());
        SocketPlatform::Close(s);
        return TcpResult::Error;
    }
    SocketPlatform::SetConnectionOptions(s);

    const SocketPlatform::ConnectState state = SocketPlatform::StartConnect(s, ip, port);
    if (state == SocketPlatform::ConnectState::Failed) {
        LogError("Connecting to ", ip, ":", port, " failed: ", SocketPlatform::LastError());
        SocketPlatform::Close(s);
        return TcpResult::Error;
    }

    handle = s;
    return state == SocketPlatform::ConnectState::Connected ? TcpResult::Ok : TcpResult::WouldBlock;
}

ME::Net::TcpResult ME::Net::TcpSocket::PollConnect() {
    int error = 0;
    const SocketPlatform::ConnectState state = SocketPlatform::PollConnect(handle, error);
    if (state == SocketPlatform::ConnectState::InProgress) {
        return TcpResult::WouldBlock;
    }
    if (state == SocketPlatform::ConnectState::Failed) {
        LogError("Connect failed: ", error);
        Close();
        return TcpResult::Error;
    }
    return TcpResult::Ok;
}

ME::Net::TcpResult ME::Net::TcpSocket::Send(const uint8_t* data, int size, int& outSent) {
    outSent = 0;
    const int sent = SocketPlatform::Send(handle, data, size);
    if (sent >= 0) {
        outSent = sent;
        return TcpResult::Ok;
    }
    return SocketPlatform::LastErrorIsWouldBlock() ? TcpResult::WouldBlock : TcpResult::Error;
}

ME::Net::TcpResult ME::Net::TcpSocket::Recv(uint8_t* buffer, int capacity, int& outReceived) {
    outReceived = 0;
    const int received = SocketPlatform::Recv(handle, buffer, capacity);
    if (received > 0) {
        outReceived = received;
        return TcpResult::Ok;
    }
    if (received == 0) {
        return TcpResult::Closed;
    }
    return SocketPlatform::LastErrorIsWouldBlock() ? TcpResult::WouldBlock : TcpResult::Error;
}

void ME::Net::TcpSocket::Close() {
    if (handle != SocketPlatform::INVALID_HANDLE) {
        SocketPlatform::Close(handle);
        handle = SocketPlatform::INVALID_HANDLE;
    }
}

bool ME::Net::TcpSocket::IsOpen() const {
    return handle != SocketPlatform::INVALID_HANDLE;
}
