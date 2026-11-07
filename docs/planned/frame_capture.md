# Planned: frame capture

Roadmap item 3, step A ([roadmap_2d.md](../roadmap_2d.md)). Goal: save a drawn frame as a PNG so rendering can be checked without looking at the screen, e.g. by an AI assistant
that runs a sample, then reads the image. Not implemented yet; a first version was written and reverted (2026-11-07)
as too big a change at once. Below is that design, split into small steps.

## Usage (target)
```
build-live/prism_drop/Sample_PrismDrop --captureFrame=120 --captureQuit=1
```
Saves the 120th drawn frame to `user_data/captures/frame_00120.png` next to the executable, logs
`FrameCapture: wrote <path>`, then quits. The same keys work in `settings.ini` under `[debug]`.

## Steps
1. **PNG writer** (`moksha/shared/file_io/png/png_writer.*`, `ME::Png`), with tests in `tests/file_io/`.
   - `size_t GetEncodedSize(width, height)`, and `bool EncodeRgba(pixels, width, height, out, capacity, outSize)`
     into caller-owned memory (no `std::vector`).
   - Stored (uncompressed) deflate: zlib header `78 01`, blocks of at most 65,535 bytes (BFINAL, LEN, NLEN), Adler-32;
     chunks IHDR (8-bit, colour type 6), one IDAT, IEND; CRC-32 from a `constexpr` table. Each row starts with filter
     byte 0. No dependencies; files are large (width × height × 4 bytes and some).
   - Tests: the size matches `GetEncodedSize`, the signature, IEND's CRC is `AE426082`, a 200×100 image spans two
     blocks with a row split across them and reads back byte for byte, and too little capacity fails.
2. **Renderer interface** (`rendering/shared/renderer.h`): `virtual bool RequestCapture()` (the next Draw keeps a copy
   of what it presents) and `virtual bool ReadCapture(const uint8_t*& outPixels, uint32_t& outWidth, uint32_t& outHeight)`
   (tightly packed RGBA8 rows, top row first, in drawable pixels). Both default to returning false, so DX12, WebGPU and
   null need no change until they implement them.
3. **Metal** (`RendererMetal`):
   - `RequestCapture` calls `view->GetLayer()->setFramebufferOnly(false)` and sets a pending flag.
   - In `Draw`, after the render encoder ends: if the drawable's texture `isFramebufferOnly()`, leave the request for a
     later frame. Accept only `BGRA8Unorm(_sRGB)` / `RGBA8Unorm(_sRGB)`. Otherwise make a shared `MTL::Buffer`, add it
     to the residency set and commit, then open a compute encoder: `barrierAfterEncoderStages(StageVertex |
     StageFragment, StageBlit, VisibilityOptionDevice)`, `copyFromTexture(... toBuffer ...)`, end.
   - After the present and fence: `FlushCommandQueue()` (a one-frame stall, fine for something rare), swizzle BGRA to
     RGBA into a `new[]` array with alpha forced to 255, remove the buffer from the residency set and release it.
     `End` frees the array.
   - Risks: MTKView may set the layer's `framebufferOnly` back (then add `SetFramebufferOnly` to `MetalView`); check
     the barrier with `MTL_DEBUG_LAYER=1`.
4. **`FrameCapture`** (`client/debug/frame_capture.*`), owned by `ClientApp`:
   - `Init(frame, quitAfter, window)`, `BeforeDraw(renderer)` (counts drawn frames, requests the capture at the
     configured frame), `AfterDraw(renderer)` (writes a ready capture; returns true when the app should quit).
   - High-DPI: when the drawable is a whole multiple of the window size, average each block down to window size, so
     files stay readable (450×800 is about 1.4 MB).
   - Write through `Vfs::WriteBytes(FileRoot::UserData, "captures/frame_%05u.png")`.
   - Give up after 30 frames without a capture, or when `RequestCapture` returns false (headless, other renderers):
     log an error, and still quit with `captureQuit`, so a scripted run never hangs.
   - `ClientApp::Tick`: `frameCapture.BeforeDraw(renderer)` before `renderer.Draw()`, then
     `if (frameCapture.AfterDraw(renderer)) engine.Quit();`.
5. **Config**: `AppConfig::captureFrame` (`uint32_t`, 0 is off; needs a `ParseU32`) and `captureQuit` (`bool`), in `[debug]`.
6. **Docs**: the cookbook (a "Frame capture" section and the settings list), `moksha/client/CLAUDE.md` (config list,
   `debug/frame_capture`), and a "Frame capture" section in `moksha_samples/CLAUDE.md` with the command line.

## Later
- A capture key (F12) while playing; several frames in one run (`captureFrames=60,120,180`) for animation.
- Count fixed steps instead of drawn frames, so the same number gives the same image.
- DX12 (copy the back buffer to a readback heap) and WebGPU (`copyTextureToBuffer` + async map; the canvas needs
  `CopySrc` usage).
