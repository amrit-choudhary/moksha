/**
 * Client side of the UDP game connection: talks to one game server, sends requests and turns replies into
 * delegate calls. Messages with game-defined verbs (GameProtocol::FIRST_GAME_VERB and up) go to an
 * IGameClientListener. Wire format in moksha/shared/net/game_protocol.h; socket in moksha/shared/net/udp_socket.h.
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "moksha/shared/math/fp_24_8.h"
#include "moksha/shared/misc/delegate.h"
#include "moksha/shared/net/game_protocol.h"
#include "moksha/shared/net/net_address.h"
#include "moksha/shared/net/udp_socket.h"

namespace ME {

class ByteReader;

/**
 * Game-side handler for messages with game-defined verbs from the server.
 */
class IGameClientListener {
   public:
    virtual ~IGameClientListener() = default;

    // A message with a game-defined verb; senderClientID is the client it came from (or this client's ID for
    // server replies). reader is positioned after the header.
    virtual void OnGameMessage(uint8_t senderClientID, uint8_t verb, ByteReader& reader) = 0;
};

/**
 * The game server this client talks to. clientID is assigned by the server on CONNECT; 0xFF = not connected.
 */
class ConnectedServer {
   public:
    Net::Address address;
    uint8_t clientID = 0xFF;
};

class GameClient {
   public:
    /**
     * Opens a UDP socket (OS-chosen port) for talking to the server. False if the address or socket is bad.
     */
    bool Init(const char* serverIP = "127.0.0.1", uint16_t serverPort = 9310);

    /**
     * Handles every datagram that has arrived since the last call. Call once per frame.
     */
    void Update(double deltaTime);
    void End();

    void SendConnectRequest();
    void SendPing();
    void SendChat(const char* message);
    void SendGameData(const FP_24_8& value1, const FP_24_8& value2, const FP_24_8& value3);

    // Sends a game-defined message: verb (FIRST_GAME_VERB and up) plus payloadSize bytes. Only while connected.
    void SendGameMessage(uint8_t verb, const uint8_t* payload, size_t payloadSize);

    // Receives messages with game-defined verbs; nullptr stops them. The listener must outlive this client.
    void SetListener(IGameClientListener* inListener);

    uint8_t GetClientID() const;
    bool IsConnected() const;

    Delegate onConnected;
    Delegate onPong;

   private:
    void ProcessDatagram(const uint8_t* data, int size);

    void RecvChat(ByteReader& reader, uint8_t clientID);
    void RecvGameData(ByteReader& reader);

    void SendDatagram(const uint8_t* data, size_t size);

    // Upper bound per Update, so a flood of datagrams can't stall a frame.
    static constexpr int MAX_DATAGRAMS_PER_UPDATE = 256;

    Net::UdpSocket socket;
    ConnectedServer connectedServer;
    IGameClientListener* listener = nullptr;
};

}  // namespace ME
