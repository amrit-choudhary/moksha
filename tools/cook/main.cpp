/**
 * Moksha_Cook: cooks res_list.json files for one target platform into one output folder (see cook.h).
 *   Moksha_Cook --target <win-dx12|mac-metal|headless> --out <dir> [--dxc <exe>] [--texconv <exe>]
 *               [--metal-developer-dir <dir>] <res_list.json>...
 * Lists cook in the order given; a later list may not produce a path an earlier one already produced.
 */

#include <cstdint>
#include <cstring>
#include <iostream>

#include "cook.h"

namespace {

constexpr uint32_t MaxLists = 16;

void PrintUsage() {
    std::cerr
        << "Usage: Moksha_Cook --target <win-dx12|mac-metal|headless> --out <dir> [--dxc <exe>] [--texconv <exe>] "
           "[--metal-developer-dir <dir>] <res_list.json>...\n";
}

}  // namespace

int main(int argc, char** argv) {
    const char* targetName = nullptr;
    const char* outDir = nullptr;
    ME::Cook::CookTools tools;
    const char* lists[MaxLists] = {};
    uint32_t listCount = 0;

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        const bool hasValue = i + 1 < argc;
        if (std::strcmp(arg, "--target") == 0 && hasValue) {
            targetName = argv[++i];
        } else if (std::strcmp(arg, "--out") == 0 && hasValue) {
            outDir = argv[++i];
        } else if (std::strcmp(arg, "--dxc") == 0 && hasValue) {
            tools.dxc = argv[++i];
        } else if (std::strcmp(arg, "--texconv") == 0 && hasValue) {
            tools.texconv = argv[++i];
        } else if (std::strcmp(arg, "--metal-developer-dir") == 0 && hasValue) {
            tools.metalDeveloperDir = argv[++i];
        } else if (std::strncmp(arg, "--", 2) == 0) {
            std::cerr << "Cook: error: unknown option or missing value: " << arg << '\n';
            PrintUsage();
            return 1;
        } else if (listCount < MaxLists) {
            lists[listCount++] = arg;
        } else {
            std::cerr << "Cook: error: too many resource lists (max " << MaxLists << ")\n";
            return 1;
        }
    }

    const ME::Cook::CookTarget target = ME::Cook::ParseTarget(targetName);
    if (target == ME::Cook::CookTarget::None || outDir == nullptr || listCount == 0) {
        PrintUsage();
        return 1;
    }

    ME::Cook::Cooker cooker;
    if (!cooker.Init(target, outDir, tools)) {
        return 1;
    }
    for (uint32_t i = 0; i < listCount; ++i) {
        if (!cooker.CookList(lists[i])) {
            return 1;
        }
    }
    cooker.End();
    return 0;
}
