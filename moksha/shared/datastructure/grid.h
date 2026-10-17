#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ME {
/**
 * Directions in the grid.
 */
enum class GridDirection : uint8_t {
    None = 0,
    E = 1,   // Right
    NE = 2,  // Up-Right
    N = 3,   // Up
    NW = 4,  // Up-Left
    W = 5,   // Left
    SW = 6,  // Down-Left
    S = 7,   // Down
    SE = 8   // Down-Right
};

/**
 * Grid in 2D space.
 * Template type T represents the type of elements stored in the grid: Cell Type
 * Grid starts at (0,0) in the bottom-left corner.
 * X axis goes right, Y axis goes up. Similar to standard Cartesian coordinates.
 */
template <typename T>
class Grid {
   public:
    Grid() = delete;

    /**
     * Constructor for creating a grid of specified dimensions.
     * Initializes all elements to default value of T.
     * Cell type should be default-constructible or POD.
     */
    Grid(size_t width, size_t height) : width(width), height(height) {
        data = new T[width * height]();
    }

    ~Grid() {
        delete[] data;
    }

    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;
    Grid(Grid&&) = delete;
    Grid& operator=(Grid&&) = delete;

    // Get mutable pointer to internal data array.
    T* GetData() {
        return data;
    }
    // Returns const pointer to internal data array.
    const T* GetData() const {
        return data;
    }

    size_t GetWidth() const {
        return width;
    }

    size_t GetHeight() const {
        return height;
    }

    /** No bounds checking. Use carefully. */
    T* GetUnsafe(size_t x, size_t y) {
        return &data[width * y + x];
    }

    /** No bounds checking. Use carefully. */
    const T* GetUnsafe(size_t x, size_t y) const {
        return &data[width * y + x];
    }

    /** No bounds checking. Use carefully. Index is a linear index (y * width + x). */
    T* GetUnsafe(size_t index) {
        return &data[index];
    }
    /** No bounds checking. Use carefully. Index is a linear index (y * width + x). */
    const T* GetUnsafe(size_t index) const {
        return &data[index];
    }

    /** Does upper bounds checking. */
    T* Get(size_t x, size_t y) {
        if (x >= width || y >= height) {
            return nullptr;
        }
        return &data[width * y + x];
    }
    /** Does upper bounds checking. */
    const T* Get(size_t x, size_t y) const {
        if (x >= width || y >= height) {
            return nullptr;
        }
        return &data[width * y + x];
    }

    /** Does upper bounds checking. Index is a linear index (y * width + x). */
    T* Get(size_t index) {
        if (index >= width * height) {
            return nullptr;
        }
        return &data[index];
    }
    /** Does upper bounds checking. Index is a linear index (y * width + x). */
    const T* Get(size_t index) const {
        if (index >= width * height) {
            return nullptr;
        }
        return &data[index];
    }

    /** Get neighbor in specified grid direction. Returns nullptr if out of bounds. */
    T* GetNeighbor(size_t x, size_t y, GridDirection direction) {
        switch (direction) {
            case GridDirection::E:
                return GetInternal(x + 1, y);
            case GridDirection::NE:
                return GetInternal(x + 1, y + 1);
            case GridDirection::N:
                return GetInternal(x, y + 1);
            case GridDirection::NW:
                return GetInternal(x - 1, y + 1);
            case GridDirection::W:
                return GetInternal(x - 1, y);
            case GridDirection::SW:
                return GetInternal(x - 1, y - 1);
            case GridDirection::S:
                return GetInternal(x, y - 1);
            case GridDirection::SE:
                return GetInternal(x + 1, y - 1);
            default:
                return nullptr;
        }
    }

    /** Get neighbor in specified grid direction. Returns nullptr if out of bounds. */
    const T* GetNeighbor(size_t x, size_t y, GridDirection direction) const {
        switch (direction) {
            case GridDirection::E:
                return GetInternal(x + 1, y);
            case GridDirection::NE:
                return GetInternal(x + 1, y + 1);
            case GridDirection::N:
                return GetInternal(x, y + 1);
            case GridDirection::NW:
                return GetInternal(x - 1, y + 1);
            case GridDirection::W:
                return GetInternal(x - 1, y);
            case GridDirection::SW:
                return GetInternal(x - 1, y - 1);
            case GridDirection::S:
                return GetInternal(x, y - 1);
            case GridDirection::SE:
                return GetInternal(x + 1, y - 1);
            default:
                return nullptr;
        }
    }

    /**
     * Get 4 orthogonal neighbors: E, N, W, S. Fills outNeighbors array of size 4.
     * Pass it an T* array of size 4.
     * */
    void GetNeighbors4Ortho(size_t x, size_t y, T** outNeighbors) {
        outNeighbors[0] = GetInternal(x + 1, y);  // E
        outNeighbors[1] = GetInternal(x, y + 1);  // N
        outNeighbors[2] = GetInternal(x - 1, y);  // W
        outNeighbors[3] = GetInternal(x, y - 1);  // S
    }

    /**
     * Get 4 orthogonal neighbors: E, N, W, S. Fills outNeighbors array of size 4.
     * Pass it a const T* array of size 4.
     * */
    void GetNeighbors4Ortho(size_t x, size_t y, const T** outNeighbors) const {
        outNeighbors[0] = GetInternal(x + 1, y);  // E
        outNeighbors[1] = GetInternal(x, y + 1);  // N
        outNeighbors[2] = GetInternal(x - 1, y);  // W
        outNeighbors[3] = GetInternal(x, y - 1);  // S
    }

    /**
     * Get 4 diagonal neighbors: NE, NW, SW, SE. Fills outNeighbors array of size 4.
     * Pass it an T* array of size 4.
     * */
    void GetNeighbors4Diag(size_t x, size_t y, T** outNeighbors) {
        outNeighbors[0] = GetInternal(x + 1, y + 1);  // NE
        outNeighbors[1] = GetInternal(x - 1, y + 1);  // NW
        outNeighbors[2] = GetInternal(x - 1, y - 1);  // SW
        outNeighbors[3] = GetInternal(x + 1, y - 1);  // SE
    }

    /**
     * Get 4 diagonal neighbors: NE, NW, SW, SE. Fills outNeighbors array of size 4.
     * Pass it a const T* array of size 4.
     * */
    void GetNeighbors4Diag(size_t x, size_t y, const T** outNeighbors) const {
        outNeighbors[0] = GetInternal(x + 1, y + 1);  // NE
        outNeighbors[1] = GetInternal(x - 1, y + 1);  // NW
        outNeighbors[2] = GetInternal(x - 1, y - 1);  // SW
        outNeighbors[3] = GetInternal(x + 1, y - 1);  // SE
    }

    /**
     * Get all 8 neighbors. Fills outNeighbors array of size 8.
     * Pass it an T* array of size 8.
     */
    void GetNeighbors8(size_t x, size_t y, T** outNeighbors) {
        outNeighbors[0] = GetInternal(x + 1, y);      // E
        outNeighbors[1] = GetInternal(x + 1, y + 1);  // NE
        outNeighbors[2] = GetInternal(x, y + 1);      // N
        outNeighbors[3] = GetInternal(x - 1, y + 1);  // NW
        outNeighbors[4] = GetInternal(x - 1, y);      // W
        outNeighbors[5] = GetInternal(x - 1, y - 1);  // SW
        outNeighbors[6] = GetInternal(x, y - 1);      // S
        outNeighbors[7] = GetInternal(x + 1, y - 1);  // SE
    }

    /**
     * Get all 8 neighbors. Fills outNeighbors array of size 8.
     * Pass it a const T* array of size 8.
     */
    void GetNeighbors8(size_t x, size_t y, const T** outNeighbors) const {
        outNeighbors[0] = GetInternal(x + 1, y);      // E
        outNeighbors[1] = GetInternal(x + 1, y + 1);  // NE
        outNeighbors[2] = GetInternal(x, y + 1);      // N
        outNeighbors[3] = GetInternal(x - 1, y + 1);  // NW
        outNeighbors[4] = GetInternal(x - 1, y);      // W
        outNeighbors[5] = GetInternal(x - 1, y - 1);  // SW
        outNeighbors[6] = GetInternal(x, y - 1);      // S
        outNeighbors[7] = GetInternal(x + 1, y - 1);  // SE
    }

    // Get X,Y coordinates from linear index.
    void GetCoordsFromIndex(size_t index, size_t& outX, size_t& outY) const {
        outX = index % width;
        outY = index / width;
    }

    // Total number of cells in the grid.
    size_t GetCount() const {
        return width * height;
    }

    size_t GetIndex(size_t x, size_t y) const {
        return y * width + x;
    }

    size_t GetSizeBytes() const {
        return sizeof(T) * width * height;
    }

    // Fill the entire grid with the specified value.
    void Fill(const T& value) {
        for (size_t i = 0; i < width * height; ++i) {
            data[i] = value;
        }
    }

    /**
     * Copy data from source grid.
     * No resizing is done. Grids must be of the same size.
     */
    void CopyFrom(const Grid<T>& source) {
        if (source.GetWidth() != width || source.GetHeight() != height) {
            return;
        }
        std::memcpy(data, source.data, GetSizeBytes());
    }

    /**
     * Fill this grid by nearest-neighbor upscaling a smaller source grid.
     * This grid's width/height must equal source dimensions * scalingFactor.
     * No resizing is done.
     */
    void FillFrom(const Grid<T>& source, size_t scalingFactor) {
        if (scalingFactor == 0 || source.GetWidth() * scalingFactor != width ||
            source.GetHeight() * scalingFactor != height) {
            return;
        }
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                data[width * y + x] = *source.GetUnsafe(x / scalingFactor, y / scalingFactor);
            }
        }
    }

   private:
    size_t width = 0;
    size_t height = 0;
    T* data = nullptr;

    // Does less than 0 and upper bounds checking.
    // This is private because public Get() only accepts size_t which is never negative.
    // This internal function can be used for neighbor calculations.
    T* GetInternal(int64_t x, int64_t y) {
        if (x < 0 || y < 0 || static_cast<size_t>(x) >= width || static_cast<size_t>(y) >= height) {
            return nullptr;
        }
        return &data[width * y + x];
    }

    const T* GetInternal(int64_t x, int64_t y) const {
        if (x < 0 || y < 0 || static_cast<size_t>(x) >= width || static_cast<size_t>(y) >= height) {
            return nullptr;
        }
        return &data[width * y + x];
    }
};

}  // namespace ME
