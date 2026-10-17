/**
 * UDP game server: accepts clients, answers pings and relays chat and data between them. Messages with
 * game-defined verbs (GameProtocol::FIRST_GAME_VERB and up) go to the game's IGameServerListener.
 * Wire format in moksha/shared/net/game_protocol.h; socket in moksha/shared/net/udp_socket.h.
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "moksha/shared/misc/engine_limits.h"
#include "moksha/shared/net/net_address.h"
#include "moksha/shared/net/udp_socket.h"

namespace ME {

class ByteReader;
class ServerApp;

// Default UDP port; settings.ini ([settings] port) overrides it.
constexpr uint16_t DEFAULT_GAME_SERVER_PORT = 9310;

/**
 * Game-side handler for a GameServer. Only OnGameMessage is required; the other calls default to nothing.
 */
class IGameServerListener {
   public:
    virtual ~IGameServerListener() = default;

    // Called once the server is listening; app gives access to the GameServer and the timers.
    virtual void OnStart(ServerApp& /*app*/) {}

    // Called before the server shuts down; clear timers and release resources here.
    virtual void OnEnd() {}

    // A new client was accepted and has been sent CONNECTED with its clientID.
    virtual void OnClientConnected(uint8_t /*clientID*/) {}

    // A message with a game-defined verb from a connected client; reader is positioned after the header.
    virtual void OnGameMessage(uint8_t clientID, uint8_t verb, ByteReader& reader) = 0;
};

/**
 * One client the server has accepted. clientID is assigned on CONNECT and sent back in every datagram.
 */
class ConnectedClient {
   public:
    uint8_t clientID = 0;
    Net::Address address;
};

class GameServer {
   public:
    GameServer() = default;
    ~GameServer() = default;

    GameServer(const GameServer&) = delete;
    GameServer& operator=(const GameServer&) = delete;
    GameServer(GameServer&&) = delete;
    GameServer& operator=(GameServer&&) = delete;

    // Opens the UDP port; listener receives game messages until End. False if the port could not be opened.
    bool Init(uint16_t port, IGameServerListener* listener);

    // Handles every datagram that has arrived since the last call, up to MAX_DATAGRAMS_PER_UPDATE.
    void Update(double deltaTime);

    // Closes the socket and forgets all clients.
    void End();

    // Sends one datagram to a connected client. Unknown clientIDs are ignored with a warning.
    void Send(const uint8_t* data, size_t size, uint8_t clientID);

    // Sends one datagram to every connected client.
    void Broadcast(const uint8_t* data, size_t size);

    // Sends one datagram to every connected client except exceptClientID.
    void SendToOthers(const uint8_t* data, size_t size, uint8_t exceptClientID);

    // Number of connected clients; their IDs are 0 .. count-1.
    size_t GetClientCount() const;

   private:
    void ProcessDatagram(const uint8_t* data, int size, const Net::Address& from);
    bool IsConnected(uint8_t clientID) const;

    void HandleConnect(const Net::Address& from);
    void HandleChat(ByteReader& reader, uint8_t clientID);
    void HandleData(ByteReader& reader, uint8_t clientID);

    void SendConnected(uint8_t clientID);
    void SendPong(uint8_t clientID);

    // Upper bound per Update, so a flood of datagrams can't hold up the server's tick.
    static constexpr int MAX_DATAGRAMS_PER_UPDATE = 256;

    Net::UdpSocket socket;
    ConnectedClient clients[Constants::MaxServerClients];
    size_t clientCount = 0;
    IGameServerListener* listener = nullptr;
};

}  // namespace ME
