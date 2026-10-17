# Moksha Game Engine

A C++20 game engine with native renderer backends, built from scratch with minimal
external dependencies. This repository holds the engine only; every game, including the
engine's samples, lives in its own repository and builds against the engine.

## Features
- **Rendering**: DirectX 12 (Windows) and Metal (Apple) backends, plus a headless backend for
  windowless clients. A data-oriented scene (struct-of-arrays, not ECS) is consumed by the
  renderer each frame.
- **App loop**: the engine owns the entry point and the frame; a game provides a factory and
  callbacks. Fixed-step simulation with variable-rate update and rendering, and a timer system
  for scheduled one-shot and repeating calls.
- **Physics**: AABB colliders and a physics scene with collision callbacks.
- **Animation**: data-driven sprite animation clips and texture atlases (JSON).
- **Audio**: audio system with a miniaudio backend and a no-op fallback.
- **Input**: platform input managers, with a null input for headless clients.
- **UI**: a small retained-mode layout engine, plus an on-screen debug overlay.
- **Networking**: cross-platform UDP and TCP sockets (Winsock / POSIX), length-prefixed
  message framing, a binary game protocol, a generic game-server module, and a content
  protocol with file manifests for delivering game data without shipping a new client.
- **Core**: vector/matrix math, 24.8 fixed-point numbers for deterministic simulation, an
  xoshiro128** random number generator, fixed-capacity data structures (`Grid`,
  `RingBuffer`, `Pool`, `Span`), a virtual file system, INI/CSV/JSON/DDS parsers, and
  bounds-checked binary serialization.
- **Tooling**: an asset cook tool, a content server, a minimal logging library and a
  hand-rolled unit test framework.

## Platforms
| Target            | Graphics API         | Status |
|-------------------|----------------------|--------|
| Windows           | DirectX 12 (Vulkan)  | supported |
| macOS             | Metal                | supported |
| iOS               | Metal                | planned |
| Android           | Vulkan               | planned |
| Web (Emscripten)  | WebGPU / WebGL       | planned |
| Linux             | Vulkan               | planned client; servers and headless client supported (incl. Raspberry Pi, ARM64) |

## Design principles
- No exceptions and no RTTI; failures are reported through return values.
- Caller-owned, fixed-capacity memory over growing containers.
- No singletons; systems receive explicit context.
- A general engine first: no API is shaped around a single game.
- Games depend on the engine; the engine never depends on a game.

## Building
C++20 and CMake 3.31 or newer.

## Third-party code
Vendored libraries, each under its own license:
- [cJSON](https://github.com/DaveGamble/cJSON) — JSON parsing.
- [miniaudio](https://miniaud.io/) — audio playback.
- [metal-cpp](https://developer.apple.com/metal/cpp/) — Metal C++ headers.
- [stb_perlin](https://github.com/nothings/stb) — Perlin noise.

## License
MIT. See [LICENSE](LICENSE).
