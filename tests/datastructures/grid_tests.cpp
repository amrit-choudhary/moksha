/**
 * Tests for Grid data structure.
 */

#include "moksha/shared/datastructure/grid.h"
#include "moksha/test_framework/test_framework.h"

TEST(Grid, ConstructionAndSize) {
    ME::Grid<int> g(3, 2);

    EXPECT(g.GetWidth() == 3);
    EXPECT(g.GetHeight() == 2);
    EXPECT(g.GetCount() == 6);
    EXPECT(g.GetSizeBytes() == sizeof(int) * 6);
}

TEST(Grid, FillAndAccess) {
    ME::Grid<int> g(4, 4);
    g.Fill(7);

    for (size_t y = 0; y < g.GetHeight(); ++y) {
        for (size_t x = 0; x < g.GetWidth(); ++x) {
            int* p = g.Get(x, y);
            ASSERT(p != nullptr);
            EXPECT(*p == 7);
            int* up = g.GetUnsafe(x, y);
            ASSERT(up != nullptr);
            EXPECT(*up == 7);
        }
    }
}

TEST(Grid, OutOfBounds) {
    ME::Grid<int> g(2, 2);

    EXPECT(g.Get(2, 0) == nullptr);
    EXPECT(g.Get(0, 2) == nullptr);
    EXPECT(g.Get(100, 100) == nullptr);

    EXPECT(g.Get(1, 1) != nullptr);
    EXPECT(g.GetUnsafe(1, 1) != nullptr);
}

TEST(Grid, NeighborsCenterAndCorner) {
    ME::Grid<int> g(3, 3);

    // Each cell holds its own linear index.
    for (size_t y = 0; y < g.GetHeight(); ++y) {
        for (size_t x = 0; x < g.GetWidth(); ++x) {
            *g.GetUnsafe(x, y) = static_cast<int>(g.GetIndex(x, y));
        }
    }

    int* center[8] = {nullptr};
    g.GetNeighbors8(1, 1, center);
    // E (2,1) => index 1*3 + 2 = 5
    ASSERT(center[0] != nullptr);
    EXPECT(*center[0] == 5);
    // N (1,2) => index 2*3 + 1 = 7
    ASSERT(center[2] != nullptr);
    EXPECT(*center[2] == 7);

    int* corner[8] = {nullptr};
    g.GetNeighbors8(0, 0, corner);
    // Bottom-left corner: W, SW and S fall outside the grid.
    EXPECT(corner[4] == nullptr);
    EXPECT(corner[5] == nullptr);
    EXPECT(corner[6] == nullptr);
    // E (1,0) => index 1
    ASSERT(corner[0] != nullptr);
    EXPECT(*corner[0] == 1);
}

TEST(Grid, IndexAndCoords) {
    ME::Grid<int> g(5, 4);
    size_t x = 3;
    size_t y = 2;
    size_t index = g.GetIndex(x, y);
    size_t outX = 0;
    size_t outY = 0;
    g.GetCoordsFromIndex(index, outX, outY);

    EXPECT(outX == x);
    EXPECT(outY == y);
    EXPECT(g.GetCount() == 20);
}
