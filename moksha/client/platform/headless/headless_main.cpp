#ifdef ME_HEADLESS

/**
 * Headless entry and frame driver: a plain main() that runs ClientApp without a window, GPU or audio device, paced
 * to the fixed step, until the game quits. Builds on every desktop OS, Linux included.
 */

#include <string>

#include "moksha/client/app/client_app.h"
#include "moksha/shared/misc/utils.h"
#include "moksha/shared/platform/exe_path.h"
#include "moksha/shared/time/frame_pacer.h"

int main(int argc, char** argv) {
    std::string exePath = ME::Platform::GetExecutablePath(argc > 0 ? argv[0] : nullptr);
    ME::Utils::SetPaths(exePath.data(), nullptr);

    ME::AppArgs args;
    args.Set(argc, argv);

    ME::ClientApp app;
    app.Configure(args);
    if (!app.Init()) {
        app.Shutdown();
        return 1;
    }

    // Nothing waits on a display, so sleep until the next fixed step instead of spinning.
    while (app.Tick()) {
        ME::Time::WaitForNextFixedStep(app.GetTime());
    }

    app.Shutdown();
    return 0;
}

#endif  // ME_HEADLESS
