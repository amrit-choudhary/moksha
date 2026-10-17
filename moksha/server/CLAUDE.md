# moksha/server/

Game server module (`Moksha_Server`): builds on every desktop OS, including Linux ARM64 (Raspberry Pi).
Project rules: [root CLAUDE.md](../../CLAUDE.md); wire format: [shared/CLAUDE.md](../shared/CLAUDE.md).

## Shape
- A game server is a plain `main` plus the game's `IGameServerListener`:
  `int main(int argc, char** argv) { MyListener l; ME::ServerApp app; return app.Run(argc, argv, l); }`.
  Build it with `moksha_add_server(<target> SOURCES … [RES_LIST …] [LIBRARIES …])`.
- `ServerApp` (`server_app.*`): exe paths, `settings.ini` (`[settings] port`, default
  `DEFAULT_GAME_SERVER_PORT` 9310), networking, time and timers at 60 Hz, the `GameServer`.
  `Init` / `Tick` / `Shutdown`, driven by `Run` with `Time::WaitForNextFixedStep` between ticks.
  `RequestQuit` ends the loop. Timers tick first in each fixed step, then the server.
- `GameServer` (`game_server.*`): one non-blocking UDP socket; up to 256 datagrams per tick. Handles the
  engine verbs itself: `CONNECT` → `CONNECTED` with a new clientID, `PING` → `PONG`, chat and data relayed
  to the other clients. Game verbs go to `IGameServerListener::OnGameMessage(clientID, verb, reader)`; the game
  replies with `Send`, `Broadcast`, `SendToOthers`.
- Listener calls: `OnStart(ServerApp&)` (get timers and the server here), `OnClientConnected`, `OnGameMessage`,
  `OnEnd` (clear timers, close resources).

## Limits (known issues)
- Clients are a fixed array of `Constants::MaxServerClients` (255). IDs are given in connection order and
  never reused: there is no disconnect or timeout yet, so restart the server after 255 connects.
- No authentication, no acknowledgement or resend layer; the version byte isn't checked.
- No SIGTERM handler: stopping the process ends it without `OnEnd`.
- Every datagram is logged at info level.

## Hosting
Open UDP 9310 (or the configured port) in the host's firewall. `resources/config/settings.ini` sits next
to the executable; without it the default port is used.
