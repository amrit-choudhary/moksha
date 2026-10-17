/**
 * Minimal unit test framework, usable by any test executable.
 * TEST(category, name) defines and auto-registers a test; RunAll() in the executable's main runs them.
 * EXPECT records a failure and continues; ASSERT records a failure and returns from the test.
 * No exceptions are used, so it works with exceptions disabled.
 */

#pragma once

#include <cstdint>

namespace ME::Test {

using TestFunc = void (*)();

inline constexpr uint32_t MAX_TESTS = 1024;

class TestCase {
   public:
    const char* category = nullptr;
    const char* name = nullptr;
    TestFunc func = nullptr;
};

/**
 * Adds a test to the registry. Called by TEST during static initialization.
 * Returns false if the registry is full.
 */
bool Register(const char* category, const char* name, TestFunc func);

/**
 * Marks the running test as failed and prints where.
 */
void ReportFailure(const char* file, int line, const char* expression);

/**
 * Runs every registered test and prints a summary.
 * Returns 0 if all passed, 1 otherwise; use it as main's return value.
 */
int RunAll();

}  // namespace ME::Test

#define TEST(category, name)                                            \
    static void Test_##category##_##name();                             \
    [[maybe_unused]] static const bool registered_##category##_##name = \
        ME::Test::Register(#category, #name, Test_##category##_##name); \
    static void Test_##category##_##name()

#define EXPECT(condition)                                            \
    do {                                                             \
        if (!(condition)) {                                          \
            ME::Test::ReportFailure(__FILE__, __LINE__, #condition); \
        }                                                            \
    } while (0)

#define ASSERT(condition)                                            \
    do {                                                             \
        if (!(condition)) {                                          \
            ME::Test::ReportFailure(__FILE__, __LINE__, #condition); \
            return;                                                  \
        }                                                            \
    } while (0)

// For floats: passes when |a - b| <= epsilon.
#define EXPECT_NEAR(a, b, epsilon) EXPECT(((a) - (b)) <= (epsilon) && ((b) - (a)) <= (epsilon))
