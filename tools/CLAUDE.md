# tools/

Host-side engine tools. They may use `moksha/shared` and `moksha/logging` only.
Project rules: [root CLAUDE.md](../CLAUDE.md).

## cook/ (`Moksha_Cook`)
- `Moksha_Cook --target <t> --out <dir> [--dxc <exe>] [--texconv <exe>] <res_list.json>...` cooks each list
  into one output folder. `moksha_add_game` runs it before each game build: the engine's
  `resources/res_list.json` first, then the game's. Two lists producing the same output path is an error.
- `res_list.json` (paths relative to the list's folder): `copy` (files as they are), `textures` (png → an
  uncompressed RGBA8 `.dds`: texconv for `win-dx12`, the cook itself for `mac-metal` via `png_to_dds` and the
  vendored `stb_image.h`), `shaders` (`{"hlsl": [...], "metal": [...]}`; each target cooks its
  graphics API's language; hlsl → `<name>_vs.cso` + `<name>_ps.cso` via dxc; metal → `<name>.metallib` via
  `xcrun metal -mmacosx-version-min=26.0`, includes from the shader's folder). Unknown keys are errors.
- The Metal compiler needs Xcode with its Metal toolchain (Xcode > Settings > Components). `moksha_add_game` finds a
  developer folder whose `xcrun` has it (the selected one, else `/Applications/Xcode.app`, so xcode-select may stay on
  the Command Line Tools) and passes it as `--metal-developer-dir`; `MOKSHA_METAL_DEVELOPER_DIR` overrides.
- Targets: `win-dx12`, `headless` (copies only; headless clients and servers), `mac-metal`. `ios-metal`,
  `android-vulkan`, `web-webgpu` are reserved and report "not supported yet".
- Skips outputs newer than their sources; a shader is rebuilt when any file in its folder changes.
- Cross-compiled builds pass a host-built cook with `MOKSHA_COOK_EXECUTABLE`.

## content_server/ (`Moksha_ContentServer`)
- TCP server (port 9311, `resources/config/settings.ini` `[settings] port`) that hands the files listed in
  `dlc/manifest.json` (next to the exe) to `ContentClient`s. Files are loaded into memory at startup, so a
  request never reaches the disk; edit files, bump their versions in the manifest, restart.
- Up to 32 clients, single-threaded polling loop, 30 s idle timeout. Built when the engine is the top-level
  project or with `MOKSHA_BUILD_TOOLS=ON`. `test_dlc/` is a minimal manifest to try it with.

## check_layers.py
`python tools/check_layers.py`: checks module dependencies and include style (see root CLAUDE.md).
