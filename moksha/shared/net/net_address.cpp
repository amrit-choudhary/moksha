#include "net_address.h"

#include "moksha/shared/net/socket_platform.h"

bool ME::Net::Address::Parse(const char* ip, uint16_t port, Address& out) {
    uint32_t parsedIp = 0;
    if (!SocketPlatform::ParseIPv4(ip, parsedIp)) {
        return false;
    }
    out.ip = parsedIp;
    out.port = port;
    return true;
}

bool ME::Net::operator==(const Address& a, const Address& b) {
    return a.ip == b.ip && a.port == b.port;
}

bool ME::Net::operator!=(const Address& a, const Address& b) {
    return !(a == b);
}
