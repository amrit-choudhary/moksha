#include "cook.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

#include "moksha/shared/third_party/json/cJSON.h"

namespace fs = std::filesystem;

namespace {

void PrintError(const fs::path& listPath, const char* entry, const std::string& reason) {
    std::cerr << "Cook: error: " << listPath.generic_string() << ": " << (entry != nullptr ? entry : "") << ": "
              << reason << '\n';
}

bool ReadFileText(const fs::path& path, std::string& outText) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    outText = buffer.str();
    return true;
}

// Entries must stay inside their list's folder and the output folder: relative, no "..".
bool IsSafeRelative(const char* entry) {
    if (entry == nullptr || entry[0] == '\0') {
        return false;
    }
    const fs::path path(entry);
    if (path.is_absolute() || path.has_root_name() || path.has_root_directory()) {
        return false;
    }
    for (const fs::path& part : path) {
        if (part == "..") {
            return false;
        }
    }
    return true;
}

// True if dst exists and is at least as new as srcTime.
bool IsUpToDate(fs::file_time_type srcTime, const fs::path& dst) {
    std::error_code ec;
    const fs::file_time_type dstTime = fs::last_write_time(dst, ec);
    return !ec && dstTime >= srcTime;
}

// Newest modification time of the regular files directly in folder; shaders depend on their includes there.
bool GetNewestInFolder(const fs::path& folder, fs::file_time_type& outTime) {
    std::error_code ec;
    fs::directory_iterator it(folder, ec);
    if (ec) {
        return false;
    }
    outTime = fs::file_time_type::min();
    for (; it != fs::directory_iterator(); it.increment(ec)) {
        if (ec) {
            return false;
        }
        std::error_code entryEc;
        if (!it->is_regular_file(entryEc)) {
            continue;
        }
        const fs::file_time_type time = it->last_write_time(entryEc);
        if (!entryEc && time > outTime) {
            outTime = time;
        }
    }
    return true;
}

std::string Quote(const std::string& text) {
    return "\"" + text + "\"";
}

int RunCommand(const std::string& command) {
#ifdef ME_WIN
    // cmd.exe strips the first and last quote of a line that starts with one; the extra pair keeps quoted paths.
    const std::string line = Quote(command);
#else
    const std::string& line = command;
#endif
    return std::system(line.c_str());
}

// Returns the array named name, or null when it is missing. False if it exists but is not an array.
bool GetArray(cJSON* object, const char* name, const fs::path& listPath, cJSON*& outArray) {
    outArray = cJSON_GetObjectItemCaseSensitive(object, name);
    if (outArray != nullptr && !cJSON_IsArray(outArray)) {
        PrintError(listPath, name, "must be an array of paths");
        return false;
    }
    return true;
}

// The entry's path string, or null after printing why it can't be used.
const char* GetEntry(const cJSON* item, const fs::path& listPath) {
    if (!cJSON_IsString(item) || item->valuestring == nullptr) {
        PrintError(listPath, nullptr, "every entry must be a path string");
        return nullptr;
    }
    if (!IsSafeRelative(item->valuestring)) {
        PrintError(listPath, item->valuestring, "must be a relative path without '..'");
        return nullptr;
    }
    return item->valuestring;
}

bool GetSourceTime(const fs::path& src, const fs::path& listPath, const char* entry, fs::file_time_type& outTime) {
    std::error_code ec;
    outTime = fs::last_write_time(src, ec);
    if (ec) {
        PrintError(listPath, entry, "source file not found: " + src.generic_string());
        return false;
    }
    return true;
}

bool EnsureFolder(const fs::path& folder, const fs::path& listPath, const char* entry) {
    std::error_code ec;
    fs::create_directories(folder, ec);
    if (ec) {
        PrintError(listPath, entry, "can't create folder " + folder.generic_string() + ": " + ec.message());
        return false;
    }
    return true;
}

}  // namespace

ME::Cook::Cooker::~Cooker() {
    delete[] outputs.data;
}

ME::Cook::CookTarget ME::Cook::ParseTarget(const char* name) {
    if (name == nullptr) {
        return CookTarget::None;
    }
    if (std::strcmp(name, "win-dx12") == 0) {
        return CookTarget::WinDX12;
    }
    if (std::strcmp(name, "headless") == 0) {
        return CookTarget::Headless;
    }
    if (std::strcmp(name, "mac-metal") == 0) {
        return CookTarget::MacMetal;
    }
    if (std::strcmp(name, "ios-metal") == 0) {
        return CookTarget::IosMetal;
    }
    if (std::strcmp(name, "android-vulkan") == 0) {
        return CookTarget::AndroidVulkan;
    }
    if (std::strcmp(name, "web-webgpu") == 0) {
        return CookTarget::WebWebGPU;
    }
    return CookTarget::None;
}

bool ME::Cook::Cooker::Init(CookTarget inTarget, const fs::path& inOutDir, const CookTools& inTools) {
    if (inTarget != CookTarget::WinDX12 && inTarget != CookTarget::Headless) {
        std::cerr << "Cook: error: this target is not supported yet\n";
        return false;
    }
    target = inTarget;
    outDir = inOutDir;
    tools = inTools;

    std::error_code ec;
    fs::create_directories(outDir, ec);
    if (ec) {
        std::cerr << "Cook: error: can't create output folder " << outDir.generic_string() << ": " << ec.message()
                  << '\n';
        return false;
    }

    delete[] outputs.data;
    outputs.data = new CookOutput[MaxOutputs];
    outputs.count = MaxOutputs;
    outputCount = 0;
    cookedCount = 0;
    upToDateCount = 0;
    return true;
}

bool ME::Cook::Cooker::CookList(const fs::path& listPath) {
    std::string text;
    if (!ReadFileText(listPath, text)) {
        PrintError(listPath, nullptr, "can't read the resource list");
        return false;
    }

    cJSON* root = cJSON_Parse(text.c_str());
    if (root == nullptr || !cJSON_IsObject(root)) {
        PrintError(listPath, nullptr, "not a JSON object");
        cJSON_Delete(root);
        return false;
    }

    // Unknown keys are usually typos that would silently drop resources.
    const cJSON* key = nullptr;
    cJSON_ArrayForEach(key, root) {
        if (std::strcmp(key->string, "copy") != 0 && std::strcmp(key->string, "textures") != 0 &&
            std::strcmp(key->string, "shaders") != 0) {
            PrintError(listPath, key->string, "unknown key (expected copy, textures, shaders)");
            cJSON_Delete(root);
            return false;
        }
    }

    const fs::path srcRoot = listPath.parent_path();
    const bool cooked = CookCopies(root, srcRoot, listPath) && CookTextures(root, srcRoot, listPath) &&
                        CookShaders(root, srcRoot, listPath);
    cJSON_Delete(root);
    return cooked;
}

void ME::Cook::Cooker::End() {
    std::cout << "Cook: " << cookedCount << " cooked, " << upToDateCount << " up to date -> " << outDir.generic_string()
              << '\n';
    delete[] outputs.data;
    outputs.data = nullptr;
    outputs.count = 0;
    outputCount = 0;
}

bool ME::Cook::Cooker::CookCopies(cJSON* root, const fs::path& srcRoot, const fs::path& listPath) {
    cJSON* entries = nullptr;
    if (!GetArray(root, "copy", listPath, entries)) {
        return false;
    }
    const cJSON* item = nullptr;
    cJSON_ArrayForEach(item, entries) {
        const char* entry = GetEntry(item, listPath);
        if (entry == nullptr || !ClaimOutput(fs::path(entry).generic_string(), listPath)) {
            return false;
        }

        const fs::path src = srcRoot / entry;
        const fs::path dst = outDir / entry;
        fs::file_time_type srcTime;
        if (!GetSourceTime(src, listPath, entry, srcTime)) {
            return false;
        }
        if (IsUpToDate(srcTime, dst)) {
            ++upToDateCount;
            continue;
        }

        if (!EnsureFolder(dst.parent_path(), listPath, entry)) {
            return false;
        }
        std::error_code ec;
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            PrintError(listPath, entry, "copy failed: " + ec.message());
            return false;
        }
        ++cookedCount;
    }
    return true;
}

bool ME::Cook::Cooker::CookTextures(cJSON* root, const fs::path& srcRoot, const fs::path& listPath) {
    cJSON* entries = nullptr;
    if (!GetArray(root, "textures", listPath, entries)) {
        return false;
    }
    // Nothing is drawn headless, so textures are left out.
    if (target == CookTarget::Headless) {
        return true;
    }

    const cJSON* item = nullptr;
    cJSON_ArrayForEach(item, entries) {
        const char* entry = GetEntry(item, listPath);
        if (entry == nullptr) {
            return false;
        }
        fs::path relOut(entry);
        relOut.replace_extension(".dds");
        if (!ClaimOutput(relOut.generic_string(), listPath)) {
            return false;
        }

        const fs::path src = srcRoot / entry;
        const fs::path dst = outDir / relOut;
        fs::file_time_type srcTime;
        if (!GetSourceTime(src, listPath, entry, srcTime)) {
            return false;
        }
        if (IsUpToDate(srcTime, dst)) {
            ++upToDateCount;
            continue;
        }

        if (!EnsureFolder(dst.parent_path(), listPath, entry)) {
            return false;
        }
        const std::string command = Quote(tools.texconv) + " -nologo -f R8G8B8A8_UNORM -m 1 -ft DDS -y -o " +
                                    Quote(dst.parent_path().string()) + " " + Quote(src.string());
        std::error_code ec;
        if (RunCommand(command) != 0 || !fs::exists(dst, ec)) {
            PrintError(listPath, entry, "texconv failed (is it on PATH, or passed with --texconv?)");
            return false;
        }
        ++cookedCount;
    }
    return true;
}

bool ME::Cook::Cooker::CookShaders(cJSON* root, const fs::path& srcRoot, const fs::path& listPath) {
    cJSON* shaders = cJSON_GetObjectItemCaseSensitive(root, "shaders");
    if (shaders == nullptr) {
        return true;
    }
    if (!cJSON_IsObject(shaders)) {
        PrintError(listPath, "shaders", "must be an object of languages, e.g. {\"hlsl\": [...]}");
        return false;
    }
    // Nothing is drawn headless; other languages belong to other graphics APIs.
    if (target != CookTarget::WinDX12) {
        return true;
    }

    cJSON* entries = nullptr;
    if (!GetArray(shaders, "hlsl", listPath, entries)) {
        return false;
    }
    const cJSON* item = nullptr;
    cJSON_ArrayForEach(item, entries) {
        const char* entry = GetEntry(item, listPath);
        if (entry == nullptr) {
            return false;
        }
        const fs::path relEntry(entry);
        const std::string stem = relEntry.stem().string();
        const fs::path relVS = relEntry.parent_path() / (stem + "_vs.cso");
        const fs::path relPS = relEntry.parent_path() / (stem + "_ps.cso");
        if (!ClaimOutput(relVS.generic_string(), listPath) || !ClaimOutput(relPS.generic_string(), listPath)) {
            return false;
        }

        const fs::path src = srcRoot / relEntry;
        fs::file_time_type srcTime;
        if (!GetSourceTime(src, listPath, entry, srcTime)) {
            return false;
        }
        // Any change in the shader's folder (its includes live there) rebuilds it.
        fs::file_time_type folderTime;
        if (GetNewestInFolder(src.parent_path(), folderTime) && folderTime > srcTime) {
            srcTime = folderTime;
        }
        const fs::path vsOut = outDir / relVS;
        const fs::path psOut = outDir / relPS;
        if (IsUpToDate(srcTime, vsOut) && IsUpToDate(srcTime, psOut)) {
            upToDateCount += 2;
            continue;
        }

        if (!EnsureFolder(vsOut.parent_path(), listPath, entry)) {
            return false;
        }
        const std::string common = " -O3 -I " + Quote(src.parent_path().string()) + " " + Quote(src.string());
        const std::string vsCommand = Quote(tools.dxc) + " -T vs_6_0 -E VS -Fo " + Quote(vsOut.string()) + common;
        const std::string psCommand = Quote(tools.dxc) + " -T ps_6_0 -E PS -Fo " + Quote(psOut.string()) + common;
        std::error_code ec;
        if (RunCommand(vsCommand) != 0 || !fs::exists(vsOut, ec)) {
            PrintError(listPath, entry, "dxc failed for the vertex shader (is dxc on PATH, or passed with --dxc?)");
            return false;
        }
        if (RunCommand(psCommand) != 0 || !fs::exists(psOut, ec)) {
            PrintError(listPath, entry, "dxc failed for the pixel shader");
            return false;
        }
        cookedCount += 2;
    }
    return true;
}

bool ME::Cook::Cooker::ClaimOutput(const std::string& relPath, const fs::path& listPath) {
    for (uint32_t i = 0; i < outputCount; ++i) {
        if (outputs[i].path == relPath) {
            PrintError(listPath, relPath.c_str(), "output already produced by " + outputs[i].owner);
            return false;
        }
    }
    if (outputCount >= outputs.count) {
        PrintError(listPath, relPath.c_str(), "too many outputs (max " + std::to_string(MaxOutputs) + ")");
        return false;
    }
    outputs[outputCount].path = relPath;
    outputs[outputCount].owner = listPath.generic_string();
    ++outputCount;
    return true;
}
