# Roadmap: engine features for 2D games

What the engine needs next so that a small 2D game (a falling block puzzle) and a somewhat larger one (a platformer or
top-down action game with levels, many enemies and a menu) can be written without workarounds. Found by building the
Prism Drop sample (`moksha_samples/prism_drop`, 2026-11-07); each gap lists the workaround the sample used, so fixing
it also simplifies that code. Ordered by value per effort. Each item is a small, separately buildable step.

## Status at a glance

| # | Item | Effort | Blocks today |
|---|---|---|---|
| 1 | Audio: make stop, mute and volume work | S | Done 2026-11-07, not yet tested |
| 2 | Sprite pipelines: linear sampling, additive blend, no depth write | M | Glows, soft particles, correct transparency |
| 3 | Frame capture and scripted input | M | Automated visual checks by an agent |
| 4 | Screen, world and UI conversions; safe area | S | UI safe area done and checked on macOS 2026-11-07; conversions open |
| 5 | Tween and particle modules | M | Every game re-writes them |
| 6 | Audio, part 2: voices, pitch, buses, crossfade | M | Rich sound design |
| 7 | Text: scale, outline, more fonts | M | Animated and readable UI text |
| 8 | Multi-touch and gestures | M | Two-thumb controls, pinch |
| 9 | Smaller downloads: compressed audio and textures | S/M | Web load time |
| 10 | Instanced sets: more than two, resizable | M | Games with many sprite kinds |
| 11 | Tile maps | M/L | Level-based games |
| 12 | Collision response and queries | M/L | Platformers, top-down movement |
| 13 | Runtime textures and render targets | L | Procedural art, post effects |
| 14 | Scene flow (title, play, pause) | S | Every game re-writes it |

S = a day or less, M = a few days, L = a week or more.

## 1. Audio: make stop, mute and volume work (S), done 2026-11-07, not yet tested
`StopMusic`, `StopAudio`, `UnloadAudioFile` and mute now work; `PlayMusic` replaces the playing track; `SetMusicVolume`
and `SetSfxVolume` set two sound groups; unloading walks every slot and reloading an id frees the old sound. Below is
what it fixed.
- `AudioImplMiniaudio::StopMusic` and `StopAudio` are empty (`moksha/client/audio/audio_impl_miniaudio.cpp`), and
  `ToggleMuteAllSounds` only sets `isMuted`, which nothing reads. A game can't stop or switch music or silence itself.
- Workaround (Prism Drop): one looping track for the whole run; pause sets the master volume to 0.
- Step: implement both stops with `ma_sound_stop`, apply mute through the engine volume (keeping the master volume to
  restore), and add `SetMusicVolume` / `SetSfxVolume` (two `ma_sound_group`s). Note it in the cookbook.

## 2. Sprite pipelines: linear sampling, additive blend, no depth write (M)
Planned in detail: [planned/sprite_pipelines.md](planned/sprite_pipelines.md).
- Every sprite and UI shader samples with `filter::nearest` (`resources/shaders/metal/*.metal`, the WGSL sampler in
  `renderer_webgpu.cpp`), so soft art scaled up (glows, rings, gradients) shows stair steps.
- Blending is alpha only (`pso_metal.cpp` `SetAlphaBlend`, `pso_webgpu.cpp`). Glows and sparks want additive
  (`One`, `One`), so overlapping light brightens instead of covering.
- Sprites use depth Less with writes on, so translucent sprites hide what draws after them behind them. Workaround:
  every instance gets its own depth, nearer with its slot, and plain sprites go farthest because they draw first.
  `Material` already carries `depthWriteEnabled` and a compare function (`rendering/shared/material.h`), but the sprite
  passes don't read it.
- Step: a per-texture-slot sampler choice (point or linear) in `TextureAtlasProperties` or the scene, then a per-set
  render state (blend: alpha or additive; depth write on or off), built as extra pipeline variants once at startup.
  Mirror across DX12, Metal and WebGPU.
- Also small: sprite colour can only darken a texture (`texColor * color`). An additive "flash" term per instance
  (one more float in the 96-byte instance data, which has padding to spare) lets a sprite flash white without a
  separate white tile.

## 3. Frame capture and scripted input (M)
Step A planned in detail: [planned/frame_capture.md](planned/frame_capture.md).
- Nothing can save a rendered frame, so an agent (or CI) can't check visuals; macOS screen capture needs a permission
  agents don't have.
- Step A: `--captureFrame=<n> --capturePath=<file>` writes frame n to a PNG under `UserData`, then optionally quits:
  read back the drawable on Metal, the back buffer on DX12, `copyTextureToBuffer` on WebGPU. A PNG writer is ~60 lines
  with `zlib`-free stored blocks, or reuse the cook's.
- Step B: an input script (`--inputScript=<file>`: frame number, key or touch event) fed through `InputManager`, so
  windowed and headless runs can replay play sessions. With A, a sample can be checked end to end without a person.

## 4. Screen, world and UI conversions; safe area (S)
- Games convert mouse position to world units and world positions to UI offsets by hand (Prism Drop's `MouseWorld`
  and `SceneUIPrismDrop::PlaceLabel`).
- iOS reports no safe area: HUD near the top can sit under the Dynamic Island and the status bar, and near the bottom
  under the home indicator.
- Step: `Camera::ScreenToWorld` / `WorldToScreen` (orthographic first), and `WindowState` safe-area insets filled
  from `UIView.safeAreaInsets` on iOS (zero elsewhere), plus a `UIAnchor` mode that respects them.

## 5. Tween and particle modules (M)
- Prism Drop wrote both (`prism_drop/game/prism_fx.*`, the `Approach` easing in `game_prism_drop.cpp`). Every
  effect-heavy game will need them.
- Step A: `ME::Ease` functions (quad, cubic, back, elastic, exponential approach) in `shared/math`.
- Step B: `ME::ParticlePool` over a caller-supplied range of one instanced set: fixed-capacity array (no `std::vector`),
  `Emit()` that reuses the oldest slot when full, per-particle velocity, gravity, drag, spin, size and colour over life,
  and a burst descriptor. Hidden slots get zero scale and are marked dirty only once.

## 6. Audio, part 2 (M)
- One `ma_sound` per loaded effect: playing it again restarts it, so rapid repeats cut each other off.
- No per-play volume, pitch or pan; no crossfade between music tracks.
- Step: a small voice pool per effect (copies made with `ma_sound_init_copy`), `PlayAudio(id, volume, pitch)`,
  and `CrossfadeMusic(id, seconds)` using `ma_sound_set_fade_in_milliseconds`.

## 7. Text: scale, outline, more fonts (M)
- A label's glyph size is fixed at construction (`Label` constructor), so text can't pop or grow; one bitmap font;
  no outline or shadow, so text over busy art is hard to read.
- Step: a scale on the label's transform used by the text pass, a second font slot, and an outline or drop shadow
  (draw the run twice, offset and tinted, or a distance-field font later).

## 8. Multi-touch and gestures (M)
- iOS and web forward only the first finger as the mouse (`platform/ios/ios_main.mm` `primaryTouch`,
  `InputManagerWeb`). Prism Drop's swipe and tap detection is game code.
- Step: up to N touches with id, position and phase in `InputManager`; keep the first finger as the mouse. Then an
  optional gesture helper (tap, swipe with direction, drag with steps) games can use.

## 9. Smaller downloads (S/M)
- Audio: done without an engine change. miniaudio is built with its MP3 and FLAC decoders and detects the format.
  Verified 2026-11-07 (macOS): Prism Drop's music as a 96 kbps MP3 (2.5 MB WAV to 0.7 MB) decodes to exactly the
  WAV's frame count at zero offset, so the loop stays gapless; it loads in the game and in the iOS and web builds.
- Textures: the cook writes uncompressed RGBA8 `.dds`. Block-compressed formats (BC7 on desktop, ASTC on iOS, and
  for WebGPU whichever the adapter supports) would cut texture memory and download size by 4-8x, but need per-platform
  cook output. Lower priority than audio.

## 10. Instanced sets: more than two, resizable (M)
- Exactly two instanced sets, fixed to texture slots 1 and 2, sized once from `.count` at startup
  (`scene.h`; the DX12 note about an empty set 0 in `moksha/client/CLAUDE.md`). A game with a tile layer, enemies,
  bullets and particles must share atlases or fall back to plain sprites (1,000 cap, one draw each).
- Step: an array of sets, each naming its texture slot and render state (item 2), with a capacity and a live count.

## 11. Tile maps (M/L)
- `client/world/tilemap.cpp` is empty. Level-based games need a grid layer drawn from an atlas, loaded from data
  (CSV through `shared/file_io/csv`, or a Tiled JSON export), with per-tile flags for collision.
- Step: a `TileMap` that fills an instanced set from a grid and redraws only changed tiles; camera culling later.

## 12. Collision response and queries (M/L)
- Physics reports AABB overlaps only and never moves anything (game guide). A platformer needs swept movement against
  tiles and boxes, slopes later, and queries: raycast, point and box overlap.
- Step: `MoveAndCollide(collider, delta)` against static colliders and the tile map, returning the contact normal;
  then raycasts against the same broad phase.

## 13. Runtime textures and render targets (L)
- Textures come only from cooked files (`Texture::Load`). A game can't upload pixels it generated at runtime, and can't
  draw into a texture (bloom, CRT, transitions, minimaps).
- Step A: `CreateTexture(width, height, rgba)` and `UpdateTexture` per backend. Step B: offscreen render targets and
  a full-screen pass with a game shader.

## 14. Scene flow (S)
- Title, play, pause and game-over states, with the pause wiring (`OnPause`, master volume) written per game.
- Step: a tiny state stack helper in the client with enter and exit hooks, plus a recipe in the cookbook. Keep it
  optional: games may keep their own switch.

## Order of work
Items 1, 4 and the MP3 check in 9 are quick, so do them first. Item 2 matters most for how effect-heavy games look.
Item 3 lets an agent verify every later change on its own. Then items 5-8 for polish, and 10-13 for larger games.
