#include "game_server.h"

#include "moksha/logging/logging.h"
#include "moksha/shared/math/fp_24_8.h"
#include "moksha/shared/net/game_protocol.h"
#include "moksha/shared/serialization/byte_reader.h"
#include "moksha/shared/serialization/byte_writer.h"

namespace GameProtocol = ME::Net::GameProtocol;
using ME::Net::GameProtocol::Verb;

bool ME::GameServer::Init(uint16_t port, IGameServerListener* inListener) {
    listener = inListener;
    clientCount = 0;

    if (!socket.Open(port)) {
        return false;
    }
    LogSuccess("Server listening on UDP port ", port);
    return true;
}

void ME::GameServer::Update(double /*deltaTime*/) {
    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];

    for (int i = 0; i < MAX_DATAGRAMS_PER_UPDATE; ++i) {
        int received = 0;
        Net::Address from;
        const Net::UdpResult result = socket.RecvFrom(buffer, GameProtocol::MAX_DATAGRAM_SIZE, received, from);
        if (result == Net::UdpResult::WouldBlock) {
            return;
        }
        if (result == Net::UdpResult::Error) {
            // Stop for this tick instead of retrying; a socket that keeps failing would spin the loop.
            LogWarning("Receiving datagram failed");
            return;
        }
        ProcessDatagram(buffer, received, from);
    }
}

void ME::GameServer::End() {
    socket.Close();
    clientCount = 0;
    listener = nullptr;
}

void ME::GameServer::Send(const uint8_t* data, size_t size, uint8_t clientID) {
    if (!IsConnected(clientID)) {
        LogWarning("Not sending to unknown client ", static_cast<unsigned>(clientID));
        return;
    }
    if (socket.SendTo(data, static_cast<int>(size), clients[clientID].address) != Net::UdpResult::Ok) {
        LogWarning("Sending to client ", static_cast<unsigned>(clientID), " failed");
    }
}

void ME::GameServer::Broadcast(const uint8_t* data, size_t size) {
    for (size_t i = 0; i < clientCount; ++i) {
        Send(data, size, clients[i].clientID);
    }
}

void ME::GameServer::SendToOthers(const uint8_t* data, size_t size, uint8_t exceptClientID) {
    for (size_t i = 0; i < clientCount; ++i) {
        if (clients[i].clientID != exceptClientID) {
            Send(data, size, clients[i].clientID);
        }
    }
}

size_t ME::GameServer::GetClientCount() const {
    return clientCount;
}

void ME::GameServer::ProcessDatagram(const uint8_t* data, int size, const Net::Address& from) {
    ByteReader reader(data, static_cast<size_t>(size));
    GameProtocol::Header header;
    if (!GameProtocol::ReadHeader(reader, header)) {
        LogWarning("Dropped datagram shorter than a header (", size, " bytes)");
        return;
    }

    LogInfo("Datagram: ", GameProtocol::GetVerbName(header.verb), ", client ", static_cast<unsigned>(header.clientID));

    if (static_cast<Verb>(header.verb) == Verb::CONNECT) {
        HandleConnect(from);
        return;
    }

    // Every other message must come from a client the server knows.
    if (!IsConnected(header.clientID)) {
        LogWarning("Dropped datagram from unknown client ", static_cast<unsigned>(header.clientID));
        return;
    }

    if (header.verb >= GameProtocol::FIRST_GAME_VERB) {
        if (listener != nullptr) {
            listener->OnGameMessage(header.clientID, header.verb, reader);
        }
        return;
    }

    switch (static_cast<Verb>(header.verb)) {
        case Verb::PING:
            SendPong(header.clientID);
            break;
        case Verb::CHAT_SEND:
            HandleChat(reader, header.clientID);
            break;
        case Verb::DATA_SEND:
            HandleData(reader, header.clientID);
            break;
        default:
            break;
    }
}

bool ME::GameServer::IsConnected(uint8_t clientID) const {
    return clientID < clientCount;
}

void ME::GameServer::HandleConnect(const Net::Address& from) {
    if (clientCount >= Constants::MaxServerClients) {
        LogWarning("Server full (", Constants::MaxServerClients, " clients), ignoring CONNECT");
        return;
    }

    ConnectedClient& newClient = clients[clientCount];
    newClient.clientID = static_cast<uint8_t>(clientCount);
    newClient.address = from;
    ++clientCount;

    SendConnected(newClient.clientID);
    if (listener != nullptr) {
        listener->OnClientConnected(newClient.clientID);
    }
}

void ME::GameServer::HandleChat(ByteReader& reader, uint8_t clientID) {
    char message[GameProtocol::CHAT_CAPACITY];
    if (!GameProtocol::ReadString(reader, message, sizeof(message))) {
        LogWarning("Dropped invalid chat from client ", static_cast<unsigned>(clientID));
        return;
    }

    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];
    ByteWriter writer(buffer, sizeof(buffer));
    GameProtocol::WriteHeader(writer, Verb::CHAT_RECV, clientID);
    GameProtocol::WriteString(writer, message);
    SendToOthers(buffer, writer.GetSize(), clientID);
}

void ME::GameServer::HandleData(ByteReader& reader, uint8_t clientID) {
    FP_24_8 value1;
    FP_24_8 value2;
    FP_24_8 value3;
    if (!GameProtocol::ReadFP(reader, value1) || !GameProtocol::ReadFP(reader, value2) ||
        !GameProtocol::ReadFP(reader, value3)) {
        LogWarning("Dropped short data from client ", static_cast<unsigned>(clientID));
        return;
    }

    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];
    ByteWriter writer(buffer, sizeof(buffer));
    GameProtocol::WriteHeader(writer, Verb::DATA_RECV, clientID);
    GameProtocol::WriteFP(writer, value1);
    GameProtocol::WriteFP(writer, value2);
    GameProtocol::WriteFP(writer, value3);
    SendToOthers(buffer, writer.GetSize(), clientID);
}

void ME::GameServer::SendConnected(uint8_t clientID) {
    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];
    ByteWriter writer(buffer, sizeof(buffer));
    GameProtocol::WriteHeader(writer, Verb::CONNECTED, clientID);
    Send(buffer, writer.GetSize(), clientID);
}

void ME::GameServer::SendPong(uint8_t clientID) {
    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];
    ByteWriter writer(buffer, sizeof(buffer));
    GameProtocol::WriteHeader(writer, Verb::PONG, clientID);
    Send(buffer, writer.GetSize(), clientID);
}
