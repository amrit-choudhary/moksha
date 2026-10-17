/**
 * Tests for Pool and Span.
 */

#include "moksha/shared/datastructure/pool.h"
#include "moksha/shared/datastructure/span.h"
#include "moksha/test_framework/test_framework.h"

TEST(Pool, AcquireUntilFull) {
    ME::Pool<int> pool(2);

    EXPECT(pool.Acquire() != nullptr);
    EXPECT(pool.Acquire() != nullptr);
    EXPECT(pool.Acquire() == nullptr);
    EXPECT(pool.GetActiveCount() == 2);
}

TEST(Pool, AcquireMany) {
    ME::Pool<int> pool(5);

    EXPECT(pool.Acquire(3) != nullptr);
    EXPECT(pool.Acquire(3) == nullptr);
    EXPECT(pool.GetActiveCount() == 3);
    EXPECT(pool.Acquire(2) != nullptr);
    EXPECT(pool.GetActiveCount() == 5);
}

TEST(Pool, ReleaseMovesLastIntoHole) {
    ME::Pool<int> pool(3);
    *pool.Acquire() = 10;
    int* middle = pool.Acquire();
    *middle = 20;
    *pool.Acquire() = 30;

    pool.Release(middle);

    EXPECT(pool.GetActiveCount() == 2);
    EXPECT(pool[0] == 10);
    EXPECT(pool[1] == 30);
}

TEST(Pool, ReleaseLastAndFreedSlotIsReused) {
    ME::Pool<int> pool(2);
    pool.Acquire();
    int* last = pool.Acquire();

    pool.Release(last);
    EXPECT(pool.GetActiveCount() == 1);
    EXPECT(pool.Acquire() == last);
}

TEST(Pool, ReleaseOfFreeSlotIsIgnored) {
    ME::Pool<int> pool(2);
    pool.Acquire();
    int* second = pool.Acquire();
    pool.Release(second);

    pool.Release(second);
    EXPECT(pool.GetActiveCount() == 1);
}

TEST(Span, ViewsCallerArray) {
    int values[3] = {1, 2, 3};
    ME::Span<int> span{values, 3};

    EXPECT(span.count == 3);
    EXPECT(span[1] == 2);
    span[1] = 20;
    EXPECT(values[1] == 20);

    ME::Span<int> copy = span;
    EXPECT(copy.data == values);
}
