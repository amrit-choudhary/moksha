#include "ini_parser.h"

#include <string_view>

ME::INIMap ME::INIParser::Load(const char* relPath, FileRoot root) {
    std::string text;
    if (!Vfs::ReadText(root, relPath, text)) {
        std::cout << "Unable to open file: " << Vfs::GetRootPath(root) << relPath << std::endl;
        return INIMap{};
    }

    return Parse(text);
}

namespace {

// Strips leading/trailing spaces, tabs and '\r' (left by "\r\n" line endings when read in binary mode).
std::string_view Trim(std::string_view text) {
    const char* whitespace = " \t\r";
    const size_t first = text.find_first_not_of(whitespace);
    if (first == std::string_view::npos) {
        return {};
    }
    const size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

}  // namespace

ME::INIMap ME::INIParser::Parse(const std::string& text) {
    INIMap iniMap;
    std::string section;
    const std::string_view allText(text);

    size_t lineStart = 0;
    while (lineStart < allText.size()) {
        size_t lineEnd = allText.find('\n', lineStart);
        if (lineEnd == std::string_view::npos) {
            lineEnd = allText.size();
        }
        const std::string_view line = Trim(allText.substr(lineStart, lineEnd - lineStart));
        lineStart = lineEnd + 1;

        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        if (line[0] == '[') {
            const size_t close = line.find(']');
            if (close == std::string_view::npos) {
                std::cout << "INI: missing ']', line skipped: " << line << '\n';
                continue;
            }
            section = Trim(line.substr(1, close - 1));
            // Lists the section even if it has no keys.
            iniMap[section];
            continue;
        }

        // Split at the first '=', so values may contain '='.
        const size_t equals = line.find('=');
        if (equals == std::string_view::npos || Trim(line.substr(0, equals)).empty()) {
            std::cout << "INI: expected key=value, line skipped: " << line << '\n';
            continue;
        }
        const std::string key(Trim(line.substr(0, equals)));
        iniMap[section][key] = std::string(Trim(line.substr(equals + 1)));
    }

    return iniMap;
}

void ME::INIParser::Print(const INIMap& iniMap) {
    for (const auto& [k, v] : iniMap) {
        std::cout << k << '\n';
        for (const auto& [k2, v2] : v) {
            std::cout << "\t" << k2 << " = " << v2 << '\n';
        }
    }
}
