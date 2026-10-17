#include "csv_data.h"

ME::CSVData::CSVData() {
    rowCount = 0;
    columnCount = 0;
    totalCellCount = 0;
    data = nullptr;
}

ME::CSVData::~CSVData() {
    ClearData();
}

size_t ME::CSVData::GetRowCount() const {
    return rowCount;
}

size_t ME::CSVData::GetColumnCount() const {
    return columnCount;
}

size_t ME::CSVData::GetTotalCellCount() const {
    return totalCellCount;
}

uint32_t* ME::CSVData::GetData() const {
    return data;
}

void ME::CSVData::ClearData() {
    if (data) {
        delete[] data;
        data = nullptr;
    }
    rowCount = 0;
    columnCount = 0;
    totalCellCount = 0;
}

void ME::CSVData::SetSize(size_t row, size_t column) {
    ClearData();
    rowCount = row;
    columnCount = column;
    totalCellCount = row * column;
    data = new uint32_t[totalCellCount]();
}

void ME::CSVData::SetValue(size_t row, size_t column, uint32_t value) {
    if (data) {
        if (row < rowCount && column < columnCount) {
            data[row * columnCount + column] = value;
        }
    }
}

uint32_t ME::CSVData::GetValue(size_t row, size_t column) const {
    if (data && row < rowCount && column < columnCount) {
        return data[row * columnCount + column];
    }
    return 0;
}

uint32_t ME::CSVData::GetValue(size_t index) const {
    if (data && index < totalCellCount) {
        return data[index];
    }
    return 0;
}
