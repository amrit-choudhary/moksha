#include "networking.h"

#include "moksha/shared/net/socket_platform.h"

bool ME::Net::InitNetworking() {
    return SocketPlatform::Init();
}

void ME::Net::ShutdownNetworking() {
    SocketPlatform::Shutdown();
}
