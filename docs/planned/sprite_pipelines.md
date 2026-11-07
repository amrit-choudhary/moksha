# Planned: sprite pipelines (linear sampling, additive blend, no depth write, flash)

Roadmap item 2 ([roadmap_2d.md](../roadmap_2d.md)). Not implemented. Goal: glows, soft particles and translucent
sprites look right without per-game workarounds. Each step builds and runs alone, on DX12, Metal and WebGPU together
(shaders in `resources/shaders/{hlsl,metal,wgsl}/` mirror each other file for file). Frame capture
([frame_capture.md](frame_capture.md)) would let an agent check each step's look; until then, check by eye.

## Today
- **Sampling**: every sprite and UI shader samples point-clamped: a `constexpr sampler(filter::nearest, ...)` in each
  `.metal`, the DX static sampler `s1` (point clamp; the root signature also has `s3`, linear clamp, unused), and one
  nearest `WGPUSampler` in `renderer_webgpu.cpp` (bind group binding 1). Soft art scaled up shows stair steps.
- **Blend**: alpha only (`SetAlphaBlend` in `pso_metal.cpp`, the blend desc in `pso_dx.cpp`, `pso_webgpu.cpp`).
- **Depth**: sprites draw with Less and writes on (`CreateDepthStateDefault`; DX and WebGPU bake it into the PSO), so a
  translucent sprite hides what draws after it behind it. Workaround in games: give every instance its own depth.
- **Colour**: `texColor * color` can only darken; a white flash needs a separate white tile.
- `Material` (`rendering/shared/material.h`) has depth fields but nothing uses it; don't build on it.

## Steps
1. **Linear sampling per texture slot.**
   - Data: a `sampling` field (0 point, 1 linear) taking one of `TextureAtlasProperties::gpuPadding`'s words, so the
     atlas constant buffer keeps its size. Set from the atlas JSON (`JsonUtils::LoadTextureAtlasProps`, default point)
     and overridable by the scene.
   - Shaders: declare both samplers and pick with the uniform flag (`sampling != 0 ? linear : point`). DX uses `s3`;
     Metal adds a second `constexpr sampler`; WebGPU adds a second sampler at a new binding (layout change in
     `PSOWebGPU`), or picks the sampler when the per-texture bind group is made (no shader change; choose one way).
   - Keep the font (UI text) on point. Risk: linear filtering bleeds neighbouring atlas tiles at their edges; atlases
     need padded/extruded tiles (`padding`/`paddingType` already exist) or a half-texel UV inset.
2. **Render state per instanced set: depth write off.**
   - Data: a small `SpriteSetState` class (`depthWrite` bool, later the blend) per instanced set, filled by the scene
     (default: today's behaviour).
   - Metal: a third depth state (Less, writes off), chosen per set in `DrawInstancedSpriteSet`. DX12 and WebGPU bake
     depth into the pipeline, so build a variant of the instanced pipeline once at startup.
   - Translucent sets then draw after opaque ones and test against them without hiding each other.
3. **Additive blend per set.** Add `blend` (alpha or additive) to `SpriteSetState`. Additive is `SrcAlpha, One` for
   colour (alpha still fades the glow), alpha channel unchanged. Built as pipeline variants at startup: up to 4 for the
   instanced pipeline on DX12 and WebGPU (blend × depth write), 2 on Metal (depth state is separate). Additive sets
   normally also turn depth writes off.
4. **Flash term.** One `float flash` (0-1) per instance from `padding[0]` of `SpriteRendererInstanceData` (and of
   `CBPerSprite` for plain sprites), so the 96-byte stride stays. Shader: `rgb = mix(texColor.rgb * color.rgb, 1, flash)`,
   alpha unchanged. Update `structs.hlsl/.metal/.wgsl`.
5. **Plain sprites** (the non-instanced pass) get the same state choice if a game needs it; otherwise leave them.
6. **Docs and samples.** The cookbook (a recipe for a glow set), `moksha/client/CLAUDE.md` (pipeline variants), and in
   `moksha_samples`: drop Prism Drop's per-instance depth workaround where the new state replaces it, and add a glow or
   additive load to Graphics Stress to measure the variants.

## Later
- With roadmap item 10 (more than two instanced sets), the state moves onto each set in the array.
- Premultiplied alpha for all sprites (fixes dark fringes with linear filtering); needs the cook to premultiply.
