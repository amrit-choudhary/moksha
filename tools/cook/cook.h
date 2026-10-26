/**
 * Resource cook: turns the files a res_list.json names into what one target platform loads at runtime, under one
 * output folder. Several lists (the engine's, then a game's) cook into the same folder; two lists producing the
 * same output path is an error, so a game can't replace engine resources by accident.
 *
 * res_list.json, paths relative to the list's own folder, '/' separated:
 *   "copy":     files copied as they are (config, atlas and game data).
 *   "textures": png images, each converted to an RGBA8 .dds next to where the png would go (win-dx12 with texconv,
 *               mac-metal in the cook itself).
 *   "shaders":  {"hlsl": [...], "metal": [...]}; each target compiles the language its graphics API uses: win-dx12
 *               hlsl with dxc (<name>_vs.cso, <name>_ps.cso), mac-metal metal with `xcrun metal` (<name>.metallib).
 * Outputs newer than their sources are skipped; a shader is rebuilt when anything in its folder (its includes) changed.
 */
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "moksha/shared/datastructure/span.h"

struct cJSON;

namespace ME::Cook {

enum class CookTarget : uint8_t {
    None,           // unknown name
    WinDX12,        // Windows, DirectX 12: dxc shaders, texconv dds textures
    Headless,       // headless clients and servers: copies only, nothing is drawn
    MacMetal,       // macOS, Metal 4: metal -> .metallib via xcrun, png -> dds in the cook
    IosMetal,       // not supported yet
    AndroidVulkan,  // not supported yet
    WebWebGPU,      // not supported yet
};

// Parses a --target name such as "win-dx12". None if unknown.
CookTarget ParseTarget(const char* name);

// External tools the cook runs, as given on the command line (or found on PATH by name).
class CookTools {
   public:
    std::string dxc = "dxc";
    std::string texconv = "texconv";
    // Developer folder for `xcrun metal` (DEVELOPER_DIR); empty uses the one xcode-select picked.
    std::string metalDeveloperDir;
};

// One cooked file and the res_list.json that produces it.
class CookOutput {
   public:
    std::string path;
    std::string owner;
};

class Cooker {
   public:
    Cooker() = default;
    ~Cooker();

    Cooker(const Cooker&) = delete;
    Cooker& operator=(const Cooker&) = delete;
    Cooker(Cooker&&) = delete;
    Cooker& operator=(Cooker&&) = delete;

    // Prepares to cook for target into outDir (created if missing). False if the target isn't supported.
    bool Init(CookTarget inTarget, const std::filesystem::path& inOutDir, const CookTools& inTools);

    // Cooks every entry of one res_list.json. False on the first failure, after printing why.
    bool CookList(const std::filesystem::path& listPath);

    // Prints a one-line summary of the work done and frees the output table.
    void End();

   private:
    static constexpr uint32_t MaxOutputs = 4096;

    bool CookCopies(cJSON* root, const std::filesystem::path& srcRoot, const std::filesystem::path& listPath);
    bool CookTextures(cJSON* root, const std::filesystem::path& srcRoot, const std::filesystem::path& listPath);
    bool CookShaders(cJSON* root, const std::filesystem::path& srcRoot, const std::filesystem::path& listPath);
    bool CookMetalShaders(cJSON* shaders, const std::filesystem::path& srcRoot, const std::filesystem::path& listPath);

    // Records relPath as produced by listPath. False if another entry already produces it or the table is full.
    bool ClaimOutput(const std::string& relPath, const std::filesystem::path& listPath);

    CookTarget target = CookTarget::None;
    std::filesystem::path outDir;
    CookTools tools;

    Span<CookOutput> outputs;  // owned: allocated in Init, freed in End (or the destructor)
    uint32_t outputCount = 0;

    uint32_t cookedCount = 0;
    uint32_t upToDateCount = 0;
};

}  // namespace ME::Cook
