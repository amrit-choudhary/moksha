#include "csv_parser.h"

#include <algorithm>
#include <charconv>
#include <sstream>
#include <vector>

ME::CSVParser::CSVParser() {}

ME::CSVParser::~CSVParser() {}

static inline std::string Trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

void ME::CSVParser::Load(CSVData* csvData, const char* relPath, const bool flipVertical, FileRoot root) {
    std::string text;
    if (!Vfs::ReadText(root, relPath, text)) {
        csvData->ClearData();
        std::cerr << "Failed to open file: " << Vfs::GetRootPath(root) << relPath << std::endl;
        return;
    }

    Parse(csvData, text, flipVertical);
}

void ME::CSVParser::Parse(CSVData* csvData, const std::string& text, const bool flipVertical) {
    csvData->ClearData();
    std::istringstream stream(text);

    std::vector<std::vector<uint32_t>> rows;
    std::string line;
    size_t maxCols = 0;

    while (std::getline(stream, line)) {
        // skip empty lines
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string token;
        std::vector<uint32_t> rowValues;
        while (std::getline(ss, token, ',')) {
            std::string t = Trim(token);
            if (t.empty()) {
                rowValues.push_back(0);
                continue;
            }
            // Exceptions are disabled, so parse with from_chars instead of stoul.
            uint32_t value = 0;
            auto [ptr, ec] = std::from_chars(t.data(), t.data() + t.size(), value);
            if (ec != std::errc() || ptr != t.data() + t.size()) {
                // non-numeric token -> treat as 0
                value = 0;
            }
            rowValues.push_back(value);
        }

        if (!rowValues.empty()) {
            maxCols = std::max(maxCols, rowValues.size());
            rows.push_back(std::move(rowValues));
        }
    }

    // populate csvData in row-major order
    size_t rowCount = rows.size();
    size_t colCount = maxCols;
    if (rowCount == 0 || colCount == 0) {
        // nothing to do
        return;
    }

    csvData->SetSize(rowCount, colCount);

    if (flipVertical) {
        for (size_t r = 0; r < rowCount; ++r) {
            const auto& rv = rows[rowCount - 1 - r];
            for (size_t c = 0; c < rv.size(); ++c) {
                csvData->SetValue(r, c, rv[c]);
            }
        }
    } else {
        for (size_t r = 0; r < rowCount; ++r) {
            const auto& rv = rows[r];
            for (size_t c = 0; c < rv.size(); ++c) {
                csvData->SetValue(r, c, rv[c]);
            }
        }
    }
}
