/**
 * Thin per-platform wrappers over the OS socket API (Winsock on Windows, POSIX on Mac/Linux).
 * Internal to moksha/shared/net: socket classes call these so they stay free of #ifdefs.
 * Handles are intptr_t (SOCKET on Windows, file descriptor on POSIX); INVALID_HANDLE means no socket.
 */
#pragma once

#include <cstdint>

namespace ME::Net::SocketPlatform {

constexpr intptr_t INVALID_HANDLE = -1;

/**
 * Once per program, before / after any socket use. WSAStartup/WSACleanup on Windows, no-op on POSIX.
 */
bool Init();
void Shutdown();

intptr_t CreateTcp();
intptr_t CreateUdp();
void Close(intptr_t s);
bool SetNonBlocking(intptr_t s);

/**
 * Listen socket options: exclusive port use on Windows, fast rebind after a restart on POSIX.
 */
void SetListenOptions(intptr_t s);

/**
 * Connected socket options: SIGPIPE protection on Mac; no-op elsewhere.
 */
void SetConnectionOptions(intptr_t s);

/**
 * UDP socket options, set before binding: exclusive port use and no ICMP reset errors on Windows; no-op on POSIX.
 */
void SetDatagramOptions(intptr_t s);

/**
 * Binds to port on all network interfaces.
 */
bool BindAny(intptr_t s, uint16_t port);
bool Listen(intptr_t s);

/**
 * Returns the new connection's handle, or INVALID_HANDLE (then check LastErrorIsWouldBlock).
 */
intptr_t Accept(intptr_t s);

/**
 * Return bytes sent / received, or -1 on failure (then check LastErrorIsWouldBlock).
 * Recv returns 0 when the other side closed the connection.
 */
int Send(intptr_t s, const uint8_t* data, int size);
int Recv(intptr_t s, uint8_t* buffer, int capacity);

/**
 * Converts text such as "192.168.1.50" to an IPv4 address in host byte order. False if it is not one.
 */
bool ParseIPv4(const char* text, uint32_t& outIp);

/**
 * Send / receive one whole datagram; ip and port are in host byte order. Return bytes or -1 (then check
 * LastErrorIsWouldBlock). A datagram bigger than capacity is cut to capacity on every platform.
 */
int SendTo(intptr_t s, const uint8_t* data, int size, uint32_t ip, uint16_t port);
int RecvFrom(intptr_t s, uint8_t* buffer, int capacity, uint32_t& outIp, uint16_t& outPort);

enum class ConnectState : uint8_t {
    Connected,   // Handshake finished; the socket can send and receive.
    InProgress,  // Handshake still running in the background; poll again later.
    Failed,      // Refused, unreachable or invalid address.
};

/**
 * Starts connecting a non-blocking socket to an IPv4 address such as "192.168.1.50".
 * On Failed, LastError says why.
 */
ConnectState StartConnect(intptr_t s, const char* ip, uint16_t port);

/**
 * Checks, without waiting, whether a connect begun by StartConnect has finished.
 * On Failed, outError holds the OS error code.
 */
ConnectState PollConnect(intptr_t s, int& outError);

/**
 * Error of the last failed socket call. Read it right after the failure, before other socket calls.
 */
int LastError();
bool LastErrorIsWouldBlock();

}  // namespace ME::Net::SocketPlatform
