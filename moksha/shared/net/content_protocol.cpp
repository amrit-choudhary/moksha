#include "content_protocol.h"

const char* ME::Net::ContentProtocol::GetVerbName(uint8_t verb) {
    switch (static_cast<Verb>(verb)) {
        case Verb::GET_MANIFEST:
            return "GET_MANIFEST";
        case Verb::GET_FILE:
            return "GET_FILE";
        case Verb::MANIFEST:
            return "MANIFEST";
        case Verb::FILE:
            return "FILE";
        case Verb::FILE_NOT_FOUND:
            return "FILE_NOT_FOUND";
    }
    return "UNKNOWN";
}
