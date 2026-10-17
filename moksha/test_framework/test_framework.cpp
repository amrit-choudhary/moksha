#include "test_framework.h"

#include <iostream>

namespace {

class Registry {
   public:
    ME::Test::TestCase tests[ME::Test::MAX_TESTS];
    uint32_t count = 0;
};

// Function-local static: TESTs register during static init, before any file-scope registry is guaranteed to exist.
Registry& GetRegistry() {
    static Registry registry;
    return registry;
}

uint32_t currentTestFailures = 0;

}  // namespace

bool ME::Test::Register(const char* category, const char* name, TestFunc func) {
    Registry& registry = GetRegistry();
    if (registry.count >= MAX_TESTS) {
        std::cout << "Test registry full, dropped: " << category << "." << name << '\n';
        return false;
    }
    registry.tests[registry.count] = TestCase{category, name, func};
    registry.count++;
    return true;
}

void ME::Test::ReportFailure(const char* file, int line, const char* expression) {
    currentTestFailures++;
    std::cout << "    FAILED: " << file << ":" << line << "  " << expression << '\n';
}

int ME::Test::RunAll() {
    Registry& registry = GetRegistry();
    uint32_t passed = 0;

    std::cout << "Running " << registry.count << " tests...\n";

    for (uint32_t i = 0; i < registry.count; ++i) {
        const TestCase& test = registry.tests[i];
        currentTestFailures = 0;
        test.func();

        if (currentTestFailures == 0) {
            passed++;
            std::cout << "[PASS] " << test.category << "." << test.name << '\n';
        } else {
            std::cout << "[FAIL] " << test.category << "." << test.name << '\n';
        }
    }

    std::cout << "\nTest Summary: " << passed << "/" << registry.count << " passed\n";
    return passed == registry.count ? 0 : 1;
}
