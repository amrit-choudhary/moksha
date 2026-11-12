# Roadmap: engine features for 2D games

What the engine needs next so that a small 2D game (a falling block puzzle) and a somewhat larger one (a platformer or
top-down action game with levels, many enemies and a menu) can be written without workarounds. Found by building the
Prism Drop sample (`moksha_samples/prism_drop`, 2026-11-07); each gap lists the workaround the sample used, so fixing
it also simplifies that code. Ordered by value per effort. Each item is a small, separately buildable step.

## High priority: simulations (found by Wind Tunnel, 2026-11-12)
Building the Wind Tunnel sample (`moksha_samples/wind_tunnel`, a 2D fluid on a grid of up to 100k cells) showed what
limits CPU simulations and their look most. These go first, in this order:
1. [Item 13, step A](#13-runtime-textures-and-render-targets-l): runtime textures. The biggest win for look, grid size
   and upload cost.
2. [Item 15](#15-instanced-sprites-cheaper-per-frame-updates-s): appearance-only updates and a 2D model matrix for
   instanced sprites. Done and tested 2026-11-13.
3. [Item 16](#16-parallel-for-on-a-worker-pool-m): parallel-for on a worker pool.
4. [Item 17](#17-cpu-profiling-scopes-sm): CPU profiling scopes on the debug overlay.
5. [Item 3](#3-frame-capture-and-scripted-input-m): frame capture, plus hiding the UI for clean shots.
6. [Item 18](#18-pointer-over-ui-s): "pointer is over UI" from the engine.
7. [Item 19](#19-line-batch-sm): a line batch.
8. [Item 20](#20-gpu-compute-l): GPU compute, the long-term step for large simulations.

## Status at a glance

| # | Item | Effort | Blocks today |
|---|---|---|---|
| 1 | Audio: make stop, mute and volume work | S | Done 2026-11-07, not yet tested |
| 2 | Sprite pipelines: linear sampling, additive blend, no depth write | M | Glows, soft particles, correct transparency |
| 3 | Frame capture and scripted input (high priority) | M | Automated visual checks by an agent; clean screenshots |
| 4 | Screen, world and UI conversions; safe area | S | UI safe area done and checked on macOS 2026-11-07; conversions open |
| 5 | Tween and particle modules | M | Every game re-writes them |
| 6 | Audio, part 2: voices, pitch, buses, crossfade | M | Rich sound design |
| 7 | Text: scale, outline, more fonts | M | Animated and readable UI text |
| 8 | Multi-touch and gestures | M | Two-thumb controls, pinch |
| 9 | Smaller downloads: compressed audio and textures | S/M | Web load time |
| 10 | Instanced sets: more than two, resizable | M | Games with many sprite kinds |
| 11 | Tile maps | M/L | Level-based games |
| 12 | Collision response and queries | M/L | Platformers, top-down movement |
| 13 | Runtime textures and render targets (step A high priority) | L | Procedural art, post effects, grid simulations |
| 14 | Scene flow (title, play, pause) | S | Every game re-writes it |
| 15 | Instanced sprites: appearance-only updates, 2D model matrix | S | Done and tested 2026-11-13 |
| 16 | Parallel-for on a worker pool (high priority) | M | Using more than one core |
| 17 | CPU profiling scopes (high priority) | S/M | Finding the slow part without outside tools |
| 18 | Pointer over UI (high priority) | S | Dragging in the world next to buttons |
| 19 | Line batch (high priority) | S/M | Streamlines, graphs, debug drawing |
| 20 | GPU compute (high priority, long term) | L | Simulations past what the CPU can run |

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
- Found again by Wind Tunnel (2026-11-12): checking its look and making the website's poster and home page image took
  PowerShell screen captures, with workarounds for display scaling and the window not being ready yet, then cropping
  the UI out by hand. Add a capture key (F12: save the current frame) and a way to hide the UI scene for a capture
  (`--captureHideUI`, or a key), so a clean shot of the world needs no editing.
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
- Step A is high priority since Wind Tunnel (2026-11-12). Without it, a grid simulation draws one instanced sprite per
  cell:
  - The grid is capped by the 100k instances of a set (`MaxInstancedSpriteTransformCount`): 421 x 237 at 16:9.
  - Each frame uploads 96 bytes per cell (about 9.6 MB at 100k cells), where an RGBA8 texture is 4 bytes per cell.
  - Cells are hard-edged squares; one quad sampling the texture with linear filtering (item 2) would give smooth smoke
    at any grid size.
  Step A with `UpdateTexture` taking a sub-rectangle covers it; the game writes a colour per cell and draws one sprite.

## 14. Scene flow (S)
- Title, play, pause and game-over states, with the pause wiring (`OnPause`, master volume) written per game.
- Step: a tiny state stack helper in the client with enter and exit hooks, plus a recipe in the cookbook. Keep it
  optional: games may keep their own switch.

## 15. Instanced sprites: cheaper per-frame updates (S), done and tested 2026-11-13
`Transform` has its own dirty flag, set by every setter (`IsDirty`, `ClearDirty`); `Scene` rebuilds an instance's matrix
only when its transform is dirty, and copies colour, atlas index and flags when its renderer is. `GetModelMatrix` fills
z-only rotations in closed form. Unit tests in `tests/math/transform_tests.cpp`. Below is the plan it followed.

Two small, independent steps in `Scene::Update` (`moksha/client/scene/scene.cpp`, `UpdateInstancedSpriteRenderers`) and
`Transform::GetModelMatrix` (`moksha/shared/math/transform.h`). No API change for games that only set `bDirty`; every
backend gains, since the work is on the CPU before the upload.

What happens today, per drawn instance whose renderer is `bDirty` (three loops per set):
1. `spriteInstanceData[i].modelMatrixData = transform.GetModelMatrix().GetDataForShader()`, where `GetModelMatrix()`
   builds `Translation * Rotation * Scale`: three 4x4 matrices, 3 `sin` and 3 `cos` in `Mat4::Rotation` (always, also
   at angle 0), and two full 4x4 multiplies (about 128 multiplies, 96 adds).
2. Copy `atlasIndex`, `color` and `flags` into the same slot.
3. Clear `bDirty`.
Wind Tunnel recolours up to 100k cells that never move: per step that is about 600k trig calls and 200k matrix
multiplies for nothing, as only (2) was needed. Prism Drop, Graphics Stress and Particle Life recolour and re-tile
instances the same way.

### Step A: appearance-only updates
- Split the flag: `bTransformDirty` (rebuild the matrix, loop 1) and `bDirty` (copy colour, atlas index and flags,
  loop 2). Simplest for games: `Transform` gets its own dirty flag, set by `SetPosition`, `SetRotation` and
  `SetScale`, and loop 1 checks it; then a game that only recolours sets `renderer.bDirty` as today and pays nothing
  for the matrix. Clear both flags in loop 3.
- Keep the draw-count rule as today: instances past the draw count stay dirty until they are drawn.
- Check that every place that moves an instance goes through a setter (the samples' `Layout`/`Sync` code, Graphics
  Stress's moving stars, Prism Drop's blocks), or it keeps a stale matrix.

### Step B: a 2D model matrix
- Sprites only rotate about z. With `rotation.x == 0` and `rotation.y == 0`, `T * R * S` is known in closed form:
  ```
  | c*sx  -s*sy  0   tx |
  | s*sx   c*sy  0   ty |     c = cos(rotation.z), s = sin(rotation.z)
  | 0      0     sz  tz |
  | 0      0     0   1  |
  ```
  Fill the `Mat4` directly in `GetModelMatrix()`: one sin/cos pair and four multiplies, and no trig at all when
  `rotation.z == 0` (c = 1, s = 0). Anything with x or y rotation (3D meshes) keeps the general path, unchanged.
- The sketch's signs and placement are for illustration: copy the element positions and signs from `Mat4::Rotation`
  (`moksha/shared/math/matrix4.cpp`; it puts `sin` where the sketch has `-sin`) and the engine's row/column order, so
  the result equals today's.
- Cost per matrix: 6 trig calls and ~128 multiplies today; 2 trig calls (or one `sincos`) and 4 multiplies rotated;
  none at angle 0. Roughly 10-30x cheaper per matrix. Gains wherever sprites really move: Graphics Stress's moving
  stars (up to 100k), Survivors, Particle Life, Prism Drop's particles.

### Verify
- Unit tests (`moksha/tests`): for random positions, scales and z angles, the 2D path equals `T * R * S` within a small
  epsilon, and exactly at angle 0; a transform with x or y rotation takes the general path. For step A: a recolour
  alone leaves the matrix data untouched, a `SetPosition` rebuilds it.
- Samples, live engine: Graphics Stress (rotated clouds and stars, moving stars), Prism Drop (rotated particles,
  re-tiled blocks), Survivors, Wind Tunnel, Particle Life look exactly as before. A wrong sign shows as sprites turning
  the wrong way; a missed transform flag as sprites that stop moving.
- Measure: Graphics Stress's moving stars and Survivors' enemy count at the frame-drop point, and Wind Tunnel's frame
  time, before and after (Release).

## 16. Parallel-for on a worker pool (M)
- The engine has no threads for game code: every simulation runs on the main thread. Wind Tunnel uses about 0.8 of
  one core at 421 x 237 (9.2 ms a step, Release), and Particle Life one core at 100k particles, while the dev machine
  has 12.
- Grid and particle work splits cleanly: a red-black pressure sweep updates one colour with no cell reading another's
  writes, advection and colouring are per cell, and Particle Life's forces are per particle.
- Step: a fixed worker pool started once (thread count from the config, 0 = run inline), and
  `ParallelFor(begin, end, minChunk, fn)` that splits a range into chunks and waits for them all. No allocation per
  call. Deterministic results as long as each index only writes its own outputs.
- Web: threads need `-pthread` and `SharedArrayBuffer`, which needs COOP/COEP headers on the site (mokshaengine.com
  sends none today). Keep the inline fallback, so the web build works before that.

## 17. CPU profiling scopes (S/M)
- Finding that Wind Tunnel's pressure solve took 14 of 15 ms needed a separate program built outside the engine to
  time each phase. The debug overlay shows only the frame rate.
- Step: `ME_PROFILE_SCOPE("name")` (an RAII timer writing into a fixed table, compiled out with a define), averaged
  over a second and shown as rows on the debug overlay; a key toggles them. Later: per-frame history for spikes.

## 18. Pointer over UI (S)
- A tap on a button also reaches the game, so a game that drags things in the world must hit-test its own buttons first.
  Wind Tunnel wrote `SceneUIWindTunnel::IsOverButton` for this.
- Step: `SceneUI::IsPointerOverUI()`, true when the pointer is over any interactive element (or any element marked as
  blocking), set during the UI's interaction update before `Game::Update` runs. Note it in the cookbook.

## 19. Line batch (S/M)
- Nothing draws lines. Wind Tunnel draws its streamlines as rotated, stretched quads in an additive instanced set,
  which uses up one of the two sets (item 10) and needs a rotation per segment.
- Step: a per-frame line list (start, end, colour, width) with a fixed capacity, drawn as one batch of quads built on
  the GPU or the CPU, in world or UI space. Also useful for debug drawing (colliders, paths, graphs).

## 20. GPU compute (L)
- Grid simulations (fluids, cellular automata, heat) and large particle systems are made of small, independent
  per-cell updates, which a GPU runs far faster than one CPU core. Wind Tunnel's solver would fit compute shaders as is.
- Step: compute pipelines and storage buffers per backend (DX12, Metal, WebGPU, Vulkan), dispatch from the game, and a
  way to draw from a buffer or texture the compute pass wrote (builds on item 13). Long term: after items 13 and 16.

## Order of work
The high-priority list at the top comes first: item 13 step A, then 16, 17, 3, 18, 19, and 20 when the rest are
done. After that: items 1, 4 and the MP3 check in 9 are quick, so do them first. Item 2 matters most for how
effect-heavy games look. Then items 5-8 for polish, and 10-12 for larger games.
