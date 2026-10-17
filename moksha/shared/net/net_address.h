/**
 * An IPv4 address and port: where a datagram came from or is sent to. Plain value, safe to copy and compare.
 */
#pragma once

#include <cstdint>

namespace ME::Net {

class Address {
   public:
    /**
     * Fills out from text such as "192.168.1.50". False, leaving out unchanged, if ip is not valid IPv4.
     */
    static bool Parse(const char* ip, uint16_t port, Address& out);

    // Host byte order, so 127.0.0.1 is 0x7F000001. Conversion to network order happens only in SocketPlatform.
    uint32_t ip = 0;
    uint16_t port = 0;
};

bool operator==(const Address& a, const Address& b);
bool operator!=(const Address& a, const Address& b);

}  // namespace ME::Net
