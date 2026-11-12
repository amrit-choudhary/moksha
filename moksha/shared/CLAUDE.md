# moksha/shared/

Code every program uses: clients, servers and tools (`Moksha_Shared`). Never includes `client/` or `server/`.
Project rules: [root CLAUDE.md](../../CLAUDE.md).

## Networking (`net/`, namespace `ME::Net`)
- `networking.h`: `InitNetworking` / `ShutdownNetworking`, once per program (WSAStartup on Windows, no-op on
  POSIX). The app classes call it; socket users never do.
- `socket_platform_win.cpp` / `_posix.cpp` / `_web.cpp`: the only per-OS socket code (non-blocking TCP and UDP, SIGPIPE,
  address reuse, UDP connection-reset off on Windows; on web every socket fails to open, since browsers have no raw
  sockets). Each is wrapped in its OS `#ifdef`; all always compile.
- UDP: `net_address.h` (IPv4 + port), `udp_socket.h` (`UdpSocket`, `UdpResult`).
- TCP: `tcp_socket.h` (`TcpSocket`, `TcpResult`; `WouldBlock` = try again), `message_framing.h` (u32 length
  prefix, caller-owned buffers), `content_protocol.h` + `content_manifest.h` (content server: manifest and
  file requests, safe-path validation, max 256 entries).
- `game_protocol.h` (`GameProtocol`): one datagram = u8 version | u8 verb | u8 clientID | payload, at most
  `MAX_DATAGRAM_SIZE` (1200). Verbs `0x00-0x7F` are the engine's (system, chat, data); `0x80-0xFF`
  (`FIRST_GAME_VERB` and up) belong to the game, which defines its own payloads. Helpers for headers,
  `FP_24_8`, and strings (u8 length + bytes, max 255; chat max 63 chars).
- All messages and binary files are read/written with `serialization/` (`ByteWriter` / `ByteReader`:
  bounds-checked, caller-owned memory, native byte order).
- Avoid `windows.h` macro names in new APIs (`SendMessage`, `DeleteFile`, ...).

## Time (`time/`)
- `TimeManager`: frame clock with fixed steps (`BeginFrame`, `GetPendingFixedSteps`, `GetFixedDeltaTime`,
  `GetTimeUntilNextFixedStep`). `frame_pacer.h` (`WaitForNextFixedStep`) sleeps until the next step for loops
  nothing else paces (servers, headless clients).
- `TimerManager`: one-shot / interval / next-tick timers on the fixed step, `Delegate` callbacks, handles
  where 0 = none. Clear timers before the objects their callbacks point to are destroyed.

## Math, data, random
- `math/`: vectors, `Mat4` (row- or column-major per graphics API, set on this library from the windowed
  client's API), `Transform` (every setter sets its dirty flag, `IsDirty`/`ClearDirty`; `GetModelMatrix` fills
  z-only rotations in closed form, the general `T * R * S` only for x or y rotation), `Vec16`, and `fp_24_8.h` — 24.8 fixed point for anything that must stay
  deterministic across machines (sent raw on the wire).
- `datastructure/`: `Grid<T>`, `RingBuffer<T>`, `Pool<T>`, `Span<T>`. Owning containers delete copy/move.
- `random/`: `Random` (xoshiro128**), `RandomWt` (weighted table), Perlin noise (`stb_perlin`, vendored).

## Physics (`physics/`)
`ColliderAABB`, `PhysicsScene` (static/dynamic collider spans), `PhysicsSystem` (layer-based checks, reports
through `ICollisionListener`, which `Game` implements).

## Files (`file_io/`)
- `vfs.h` (`Vfs`, `FileRoot { Resources, Dlc, UserData }`): the only place that maps a root to a folder and
  touches the disk. `ReadText`, `GetFileSize` + `ReadBytes` (caller-owned buffer), `WriteBytes` (creates
  folders), `RemoveFile`, `RemoveEmptyFolders`. Each driver sets the root folders once at startup with
  `Utils::SetPaths(AppPaths)` (`misc/utils.h`). Desktop and servers use `MakeExecutableRelativePaths`
  (`resources/`, `dlc/`, `user_data/` next to the exe); iOS maps them to the bundle and Application Support, web to an
  in-memory file system with `user_data/` kept in IndexedDB. On Android `Resources` is packed in the APK: reads go
  through `Platform::GetAssetSize` / `ReadAsset` with the `AAssetManager*` the driver puts in `AppPaths::assetManager`,
  and writes and removals there fail; `Dlc` and `UserData` stay plain files.
- Parsers work on text already in memory: INI (`[section] key=value`, `;`/`#` comments), CSV, DDS.
- `std::filesystem` calls use the `std::error_code` overloads only.

## Platform (`platform/`)
One file per OS for code every program needs: `GetExecutablePath(argv0)` (Windows `GetModuleFileNameA`, macOS
`_NSGetExecutablePath` + `realpath`; `exe_path_posix.cpp` covers the rest with `argv0` for now). `assets.h`: files
packed in the app package, only for `Vfs` (`assets_android.cpp`: `AAssetManager`; links `android`).

## Vendored
`third_party/json` (cJSON), `random/stb_perlin.*`: opaque, don't modify.
