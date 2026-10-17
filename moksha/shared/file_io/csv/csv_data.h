#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>

namespace ME {

/**
 * A simple CSV data structure to hold parsed CSV content.
 * Only holds unsigned 32-bit integer values.
 * Data is stored in a 1D array in row-major order.
 * Starting from top-left corner.
 */
class CSVData {
   public:
    CSVData();
    ~CSVData();

    size_t GetRowCount() const;
    size_t GetColumnCount() const;
    size_t GetTotalCellCount() const;
    uint32_t* GetData() const;

    // Clears the data before loading new data.
    void ClearData();

    void SetSize(size_t row, size_t column);

    void SetValue(size_t row, size_t column, uint32_t value);

    uint32_t GetValue(size_t row, size_t column) const;

    uint32_t GetValue(size_t index) const;

   private:
    size_t rowCount = 0;
    size_t columnCount = 0;
    size_t totalCellCount = 0;
    uint32_t* data = nullptr;
};

}  // namespace ME
