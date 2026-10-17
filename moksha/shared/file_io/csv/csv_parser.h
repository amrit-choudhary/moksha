#pragma once

#include <string>

#include "csv_data.h"
#include "moksha/shared/file_io/vfs.h"

namespace ME {

class CSVParser {
   public:
    CSVParser();
    ~CSVParser();

    /**
     * Reads relPath from the given root and parses it. Clears csvData if the file can't be opened.
     * For tilemaps, set flipVertical to true to flip the rows vertically.
     * Because it is common for tilemaps data to have (0,0) at bottom-left.
     */
    static void Load(CSVData* csvData, const char* relPath, const bool flipVertical = false,
                     FileRoot root = FileRoot::Resources);

    /**
     * Parses CSV text already in memory.
     */
    static void Parse(CSVData* csvData, const std::string& text, const bool flipVertical = false);

   private:
};

}  // namespace ME
