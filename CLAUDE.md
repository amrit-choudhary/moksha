# Moksha Engine

C++20 game engine (namespace `ME`), extracted from The Village Shop game
(`github.com/amrit-choudhary/village_shop`, local: `D:\GameDev\Projects\village_shop\village_shop`).
This repo holds engine code only; each game lives in its own repo and consumes the engine.

**Current state**: the separation is in progress. The engine source still lives in the
village_shop repo (`shared/`, `logging/`, `test_framework/`, and the engine parts of
`client/`). The village_shop `CLAUDE.md` files (root, `client/`, `shared/`, `server/`,
`content_server/`) describe the engine's architecture until it moves here; move those notes
into this repo as the code arrives.

## Engine vs. game
- Engine code is generic: no API, comment or default is shaped around one game. Game-specific
  behavior goes into the game's repo, usually as a subclass or data the game supplies.
- Tech-demo scenes (breakout, dice, falling sand, game of life, RPG/character test) exercise
  engine subsystems; demos carry no backward-compat weight. If an API change makes the engine
  more consistent, change it and fix the demo call sites in the same pass.
- When it isn't clear whether something belongs in the engine or a game, ask.

## Build system
- C++20, CMake 3.31+.
- **RTTI and exceptions are disabled globally** (MSVC: `/GR- /EHs-c-` + `_HAS_EXCEPTIONS=0`,
  C4530 promoted to an error; else: `-fno-rtti -fno-exceptions`). Do not write or suggest code
  using `try`/`catch`/`throw`, `dynamic_cast`, or `typeid`.
- Platforms: Windows (DX12, Winsock2, `VG_WIN`), Mac (Metal, Cocoa, `VG_MAC`), plus an ASCII
  renderer and headless input for console builds. Server-side code also builds on Linux
  (`VG_LINUX`, POSIX sockets); test host is a Raspberry Pi 4B (Linux ARM64).
- `file(GLOB ...)` source lists are collected at configure time: after adding or removing a
  `.cpp`, re-run `cmake -B build`.

## Code conventions
Apply to all engine code (not vendored third-party code). Existing code that breaks a rule
isn't rewritten unprompted; new code follows them.
- **No exceptions or RTTI**. Report failure with return values (`bool`, null pointer, error enum).
  `std::filesystem` calls must use the `std::error_code` overloads.
- **`class`, never `struct`**, even for plain-data aggregates; use an explicit `public:` label.
- **No `std::vector` in new code**: prefer caller-owned memory (pointer + capacity), fixed-size
  arrays, or `Span<T>`, always with bounds checks. If growth is truly needed, ask first.
- **No singletons / static instance pointers** to give callbacks or systems access to object
  state. Pass explicit bound context (the `Delegate` pattern: `void* object` + function pointer,
  or a listener interface). If a global looks like the only option, raise it as a design problem.
- **Includes are module-qualified from the repo root** (no bare `src/...`, no `../`). The exact
  prefixes for this repo are decided as part of the separation.
- **Comments: max 2 lines**, stating the engine-level contract. Never justify code by what one
  game or demo currently does.
- Headers include what they use directly (e.g. `<cstdint>`): MSVC/Apple clang hide missing
  includes, g++ (Linux builds) does not.
- Avoid names that are `windows.h` macros in new APIs (`GetFreeSpace`, `PeekMessage`,
  `SendMessage`, `DeleteFile`, `MoveFile`, `CreateDirectory`, ...).
- Format with the repo's `.clang-format` (Google-based, 4-space indent, 120 columns).

## Working with AI assistants
- Never `git commit` or stage; the user reviews and commits manually.
- Don't build or run anything to verify; the user does that. Ask before running anything.
- For new systems: small, independently buildable steps, simplest first, explain each, then stop.
- The user works on 3 machines (2 Windows, 1 Mac) synced through git: put project knowledge in
  `CLAUDE.md` files, not local memory.

## Dependencies
Vendored, treat as opaque (don't modify or deep-dive): cJSON, miniaudio, metal-cpp,
stb_perlin. SQLite is used by the village_shop game server only.
