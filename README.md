# Moksha Engine

A C++20 game engine with native renderer backends, built from scratch with minimal
external dependencies.

Moksha grew out of [The Village Shop](https://github.com/amrit-choudhary/village_shop), a multiplayer village
simulation game, where the engine was built alongside the game. It now lives in its own
repository so that several games can be built on top of it; game code lives in each game's
own repository.

> **Status:** the engine is being extracted from The Village Shop. This repository does not
> contain engine source yet.

## Features
- **Rendering**: DirectX 12 (Windows) and Metal (Mac) backends, plus an ASCII renderer for
  console-only builds. A data-oriented scene (struct-of-arrays, not ECS) is consumed by the
  renderer each frame.
- **Simulation loop**: fixed-step simulation with variable-rate update and rendering, and a
  timer system for scheduled one-shot and repeating calls.
- **Physics**: AABB colliders and a physics scene with collision callbacks.
- **Animation**: data-driven sprite animation clips and texture atlases (JSON).
- **Audio**: audio system with a miniaudio backend and a no-op fallback.
- **Input**: platform input managers for Windows, Mac and a headless CLI mode.
- **UI**: a small retained-mode layout engine, plus an on-screen debug overlay.
- **Networking**: cross-platform UDP and TCP sockets (Winsock / POSIX), length-prefixed
  message framing, a binary game protocol, and a content protocol with file manifests for
  delivering game data without shipping a new client.
- **Core**: vector/matrix math, 24.8 fixed-point numbers for deterministic simulation, an
  xoshiro128** random number generator, fixed-capacity data structures (`Grid`,
  `RingBuffer`, `Pool`, `Span`), a virtual file system, INI/CSV/JSON/DDS parsers, and
  bounds-checked binary serialization.
- **Tooling**: a minimal logging library and a hand-rolled unit test framework.

## Platforms
| Target          | Windows | Mac   | Linux |
|-----------------|---------|-------|-------|
| Client / games  | DX12    | Metal | —     |
| Servers         | Yes     | Yes   | Yes (incl. Raspberry Pi, ARM64) |

## Design principles
- No exceptions and no RTTI; failures are reported through return values.
- Caller-owned, fixed-capacity memory over growing containers.
- No singletons; systems receive explicit context.
- A general engine first: no API is shaped around a single game.

## Building
C++20 and CMake 3.31 or newer. Build instructions will be added once the engine source and
its CMake targets are in place.

## Third-party code
Vendored libraries, each under its own license:
- [cJSON](https://github.com/DaveGamble/cJSON) — JSON parsing.
- [miniaudio](https://miniaud.io/) — audio playback.
- [metal-cpp](https://developer.apple.com/metal/cpp/) — Metal C++ headers.
- [stb_perlin](https://github.com/nothings/stb) — Perlin noise.

## License
MIT. See [LICENSE](LICENSE).
