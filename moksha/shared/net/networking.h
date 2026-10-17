/**
 * Process-wide networking setup, shared by every socket type (TCP and UDP).
 * Call InitNetworking once at program start before creating sockets, ShutdownNetworking once at exit after
 * closing them. Winsock needs this; no-op on POSIX.
 */
#pragma once

namespace ME::Net {

/**
 * Returns false (and logs why) if the OS networking layer could not start; sockets will then fail to open.
 */
bool InitNetworking();
void ShutdownNetworking();

}  // namespace ME::Net
