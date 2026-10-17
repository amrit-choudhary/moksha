#include "app_config.h"

#include <cstdlib>
#include <cstring>

#include "moksha/logging/logging.h"

namespace {

bool ParseU16(const std::string& text, uint16_t& out) {
    char* end = nullptr;
    const long value = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0' || value < 0 || value > 65535) {
        return false;
    }
    out = static_cast<uint16_t>(value);
    return true;
}

bool ParseBool(const std::string& text, bool& out) {
    if (text == "1" || text == "true") {
        out = true;
        return true;
    }
    if (text == "0" || text == "false") {
        out = false;
        return true;
    }
    return false;
}

bool ParsePositiveDouble(const std::string& text, double& out) {
    char* end = nullptr;
    const double value = std::strtod(text.c_str(), &end);
    if (end == text.c_str() || *end != '\0' || value <= 0.0) {
        return false;
    }
    out = value;
    return true;
}

}  // namespace

void ME::AppArgs::Set(int argc, char** argv) {
    count = 0;
    for (int i = 0; i < argc && count < MaxCount; ++i) {
        if (argv[i] != nullptr) {
            values[count++] = argv[i];
        }
    }
}

int ME::AppArgs::GetCount() const {
    return count;
}

const char* ME::AppArgs::Get(int index) const {
    return (index >= 0 && index < count) ? values[index] : nullptr;
}

const char* ME::AppArgs::GetValue(const char* name) const {
    const size_t nameLength = std::strlen(name);
    for (int i = 1; i < count; ++i) {
        const char* arg = values[i];
        if (arg[0] == '-' && arg[1] == '-' && std::strncmp(arg + 2, name, nameLength) == 0 &&
            arg[2 + nameLength] == '=') {
            return arg + 2 + nameLength + 1;
        }
    }
    return nullptr;
}

void ME::AppConfig::ApplySettings(const INIMap& settings) {
    constexpr const char* sections[] = {"window", "time", "net"};
    for (const char* sectionName : sections) {
        const auto section = settings.find(sectionName);
        if (section == settings.end()) {
            continue;
        }
        for (const auto& [key, value] : section->second) {
            if (!ApplyValue(key, value)) {
                LogWarning("settings.ini: ignoring [", sectionName, "] ", key, "=", value);
            }
        }
    }
}

void ME::AppConfig::ApplyArgs(const AppArgs& args) {
    for (int i = 1; i < args.GetCount(); ++i) {
        const char* arg = args.Get(i);
        const char* equals = std::strchr(arg, '=');
        if (arg[0] != '-' || arg[1] != '-' || equals == nullptr) {
            continue;
        }
        const std::string key(arg + 2, equals);
        // Unknown keys are allowed: games read their own arguments through AppArgs.
        ApplyValue(key, equals + 1);
    }
}

bool ME::AppConfig::ApplyValue(const std::string& key, const std::string& value) {
    if (key == "title") {
        windowTitle = value;
        return true;
    }
    if (key == "width") return ParseU16(value, windowWidth);
    if (key == "height") return ParseU16(value, windowHeight);
    if (key == "vsync") return ParseBool(value, vsync);
    if (key == "fullscreen") return ParseBool(value, fullscreen);
    if (key == "fixedStepFPS") return ParsePositiveDouble(value, fixedStepFPS);
    if (key == "serverIP") {
        serverIP = value;
        return true;
    }
    if (key == "serverPort") return ParseU16(value, serverPort);
    if (key == "contentServerIP") {
        contentServerIP = value;
        return true;
    }
    if (key == "contentServerPort") return ParseU16(value, contentServerPort);
    return false;
}
