#include "game_client.h"

#include <cstring>

#include "moksha/logging/logging.h"
#include "moksha/shared/math/fp_24_8.h"
#include "moksha/shared/net/game_protocol.h"
#include "moksha/shared/serialization/byte_reader.h"
#include "moksha/shared/serialization/byte_writer.h"

namespace GameProtocol = ME::Net::GameProtocol;
using ME::Net::GameProtocol::Verb;

bool ME::GameClient::Init(const char* serverIP, uint16_t serverPort) {
    if (!Net::Address::Parse(serverIP, serverPort, connectedServer.address)) {
        LogError("Invalid game server address: ", serverIP);
        return false;
    }

    // Port 0: the OS picks a free port; the server replies to whatever port our datagrams come from.
    if (!socket.Open(0)) {
        return false;
    }
    LogInfo("Game server: ", serverIP, ":", serverPort);
    return true;
}

void ME::GameClient::Update(double deltaTime) {
    if (!socket.IsOpen()) {
        return;
    }

    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];

    for (int i = 0; i < MAX_DATAGRAMS_PER_UPDATE; ++i) {
        int received = 0;
        Net::Address from;
        const Net::UdpResult result = socket.RecvFrom(buffer, GameProtocol::MAX_DATAGRAM_SIZE, received, from);
        if (result == Net::UdpResult::WouldBlock) {
            return;
        }
        if (result == Net::UdpResult::Error) {
            // Stop for this frame instead of retrying; a socket that keeps failing would spin the loop.
            LogWarning("Receiving datagram failed");
            return;
        }
        ProcessDatagram(buffer, received);
    }
}

void ME::GameClient::End() {
    socket.Close();
}

void ME::GameClient::ProcessDatagram(const uint8_t* data, int size) {
    ByteReader reader(data, static_cast<size_t>(size));
    GameProtocol::Header header;
    if (!GameProtocol::ReadHeader(reader, header)) {
        LogWarning("Dropped datagram shorter than a header (", size, " bytes)");
        return;
    }

    if (header.verb >= GameProtocol::FIRST_GAME_VERB) {
        if (listener != nullptr) {
            listener->OnGameMessage(header.clientID, header.verb, reader);
        }
        return;
    }

    switch (static_cast<Verb>(header.verb)) {
        case Verb::CONNECTED:
            connectedServer.clientID = header.clientID;
            onConnected.Execute();
            break;
        case Verb::PONG:
            onPong.Execute();
            break;
        case Verb::CHAT_RECV:
            RecvChat(reader, header.clientID);
            break;
        case Verb::DATA_RECV:
            RecvGameData(reader);
            break;
        default:
            break;
    }
}

void ME::GameClient::SendConnectRequest() {
    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];
    ByteWriter writer(buffer, sizeof(buffer));
    GameProtocol::WriteHeader(writer, Verb::CONNECT, connectedServer.clientID);
    SendDatagram(buffer, writer.GetSize());
}

void ME::GameClient::SendPing() {
    if (!IsConnected()) {
        return;
    }

    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];
    ByteWriter writer(buffer, sizeof(buffer));
    GameProtocol::WriteHeader(writer, Verb::PING, connectedServer.clientID);
    SendDatagram(buffer, writer.GetSize());
}

void ME::GameClient::SendChat(const char* message) {
    if (!IsConnected()) {
        return;
    }
    if (std::strlen(message) >= GameProtocol::CHAT_CAPACITY) {
        LogWarning("Chat not sent: longer than ", GameProtocol::CHAT_CAPACITY - 1, " characters");
        return;
    }

    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];
    ByteWriter writer(buffer, sizeof(buffer));
    GameProtocol::WriteHeader(writer, Verb::CHAT_SEND, connectedServer.clientID);
    GameProtocol::WriteString(writer, message);
    SendDatagram(buffer, writer.GetSize());
}

void ME::GameClient::SendGameData(const FP_24_8& value1, const FP_24_8& value2, const FP_24_8& value3) {
    if (!IsConnected()) {
        return;
    }

    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];
    ByteWriter writer(buffer, sizeof(buffer));
    GameProtocol::WriteHeader(writer, Verb::DATA_SEND, connectedServer.clientID);
    GameProtocol::WriteFP(writer, value1);
    GameProtocol::WriteFP(writer, value2);
    GameProtocol::WriteFP(writer, value3);
    SendDatagram(buffer, writer.GetSize());
}

void ME::GameClient::SendGameMessage(uint8_t verb, const uint8_t* payload, size_t payloadSize) {
    if (!IsConnected()) {
        return;
    }
    if (verb < GameProtocol::FIRST_GAME_VERB) {
        LogWarning("Game message not sent: verb ", static_cast<unsigned>(verb), " belongs to the engine");
        return;
    }

    uint8_t buffer[GameProtocol::MAX_DATAGRAM_SIZE];
    ByteWriter writer(buffer, sizeof(buffer));
    if (!GameProtocol::WriteHeader(writer, verb, connectedServer.clientID) ||
        !writer.WriteBytes(payload, payloadSize)) {
        LogWarning("Game message not sent: ", payloadSize, " bytes is more than a datagram holds");
        return;
    }
    SendDatagram(buffer, writer.GetSize());
}

void ME::GameClient::SetListener(IGameClientListener* inListener) {
    listener = inListener;
}

void ME::GameClient::RecvChat(ByteReader& reader, uint8_t clientID) {
    char message[GameProtocol::CHAT_CAPACITY];
    if (!GameProtocol::ReadString(reader, message, sizeof(message))) {
        LogWarning("Dropped invalid chat");
        return;
    }
    LogInfo("Chat from client ", static_cast<unsigned>(clientID), ": ", message);
}

void ME::GameClient::RecvGameData(ByteReader& reader) {
    FP_24_8 value1;
    FP_24_8 value2;
    FP_24_8 value3;
    if (!GameProtocol::ReadFP(reader, value1) || !GameProtocol::ReadFP(reader, value2) ||
        !GameProtocol::ReadFP(reader, value3)) {
        LogWarning("Dropped short game data");
        return;
    }
    LogInfo("Received data: ", value1.ToFloat(), ", ", value2.ToFloat(), ", ", value3.ToFloat());
}

void ME::GameClient::SendDatagram(const uint8_t* data, size_t size) {
    if (!socket.IsOpen()) {
        return;
    }
    if (socket.SendTo(data, static_cast<int>(size), connectedServer.address) != Net::UdpResult::Ok) {
        LogWarning("Sending to game server failed");
    }
}

uint8_t ME::GameClient::GetClientID() const {
    return connectedServer.clientID;
}

bool ME::GameClient::IsConnected() const {
    return connectedServer.clientID != 0xFF;
}
