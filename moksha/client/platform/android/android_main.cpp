#ifdef ME_ANDROID

/**
 * Android entry and frame driver. The NDK's native_app_glue (compiled into the entry library) receives the
 * NativeActivity's callbacks and calls android_main on its own thread. The app starts at the first window; while it is
 * resumed and has its window, events are polled without waiting and Tick runs once per loop (the swapchain's FIFO
 * present paces it); otherwise the loop waits for the next event.
 */

#include <android/configuration.h>
#include <android/input.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include "android_vulkan_view.h"
#include "moksha/client/app/client_app.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/misc/utils.h"

namespace {

// The driver's state, bound to the glue's callbacks through android_app::userData.
class AndroidDriver {
   public:
    android_app* androidApp = nullptr;
    ME::ClientApp* app = nullptr;
    ME::AndroidVulkanView view;
    float dpiScale = 1.0f;
    // The finger driving the mouse, or -1 for none.
    int32_t primaryPointerId = -1;
    bool started = false;
    bool resumed = false;
    bool hasWindow = false;
    // The window was destroyed after Init. TODO: recreate the renderer's surface; until then the app stays paused.
    bool surfaceLost = false;
    bool quit = false;

    bool IsActive() const {
        return app != nullptr && resumed && hasWindow && !surfaceLost && !quit;
    }
};

// Resources are the APK's assets/ (read through the asset manager; the path is only a label). Writable roots live in
// the app's internal storage, private to it and removed with it.
ME::Utils::AppPaths MakeAndroidPaths(ANativeActivity* activity) {
    ME::Utils::AppPaths paths;
    const std::string internal = activity->internalDataPath != nullptr ? activity->internalDataPath : "";
    paths.executableDir = internal;
    paths.resources = "assets/";
    paths.dlc = internal + "/dlc/";
    paths.userData = internal + "/user_data/";
    paths.assetManager = activity->assetManager;
    return paths;
}

// Pixels per window unit: the screen density over Android's baseline of 160 dpi.
float GetDpiScale(AConfiguration* config) {
    const int32_t density = AConfiguration_getDensity(config);
    if (density <= 0 || density >= ACONFIGURATION_DENSITY_ANY) {
        return 1.0f;
    }
    return static_cast<float>(density) / static_cast<float>(ACONFIGURATION_DENSITY_MEDIUM);
}

void StartApp(AndroidDriver& driver) {
    driver.started = true;
    android_app* androidApp = driver.androidApp;
    ME::Utils::SetPaths(MakeAndroidPaths(androidApp->activity));

    // No command line on Android.
    ME::AppArgs args;
    driver.app = new ME::ClientApp();
    driver.app->Configure(args);

    driver.dpiScale = GetDpiScale(androidApp->config);
    const float width = static_cast<float>(ANativeWindow_getWidth(androidApp->window)) / driver.dpiScale;
    const float height = static_cast<float>(ANativeWindow_getHeight(androidApp->window)) / driver.dpiScale;
    driver.app->SetWindowMetrics(static_cast<uint16_t>(width), static_cast<uint16_t>(height), driver.dpiScale);

    driver.view.window = androidApp->window;
    if (!driver.app->Init(&driver.view)) {
        ME::LogError("Android: the app failed to initialize");
        delete driver.app;  // shuts the app down
        driver.app = nullptr;
        driver.quit = true;
        ANativeActivity_finish(androidApp->activity);
    }
}

void HandleCommand(android_app* androidApp, int32_t command) {
    AndroidDriver& driver = *static_cast<AndroidDriver*>(androidApp->userData);
    switch (command) {
        case APP_CMD_INIT_WINDOW:
            driver.hasWindow = androidApp->window != nullptr;
            if (driver.hasWindow && !driver.started) {
                StartApp(driver);
            } else if (driver.hasWindow && driver.surfaceLost) {
                ME::LogWarning("Android: the window is back, but the renderer can't recreate its surface yet");
            }
            break;
        case APP_CMD_TERM_WINDOW:
            driver.hasWindow = false;
            driver.view.window = nullptr;
            if (driver.app != nullptr) {
                driver.surfaceLost = true;
            }
            break;
        case APP_CMD_RESUME:
            driver.resumed = true;
            break;
        case APP_CMD_PAUSE:
            driver.resumed = false;
            break;
        default:
            break;
    }

    if (driver.app != nullptr) {
        if (driver.IsActive()) {
            driver.app->Resume();
        } else {
            driver.app->Pause();
        }
    }
}

bool FindPointer(const AInputEvent* event, int32_t pointerId, size_t& outIndex) {
    const size_t count = AMotionEvent_getPointerCount(event);
    for (size_t i = 0; i < count; ++i) {
        if (AMotionEvent_getPointerId(event, i) == pointerId) {
            outIndex = i;
            return true;
        }
    }
    return false;
}

// Position in window units, top-left origin: the engine's mouse convention.
void ForwardPrimaryTouch(AndroidDriver& driver, const AInputEvent* event, size_t index, bool isDown) {
    const float x = AMotionEvent_getX(event, index) / driver.dpiScale;
    const float y = AMotionEvent_getY(event, index) / driver.dpiScale;
    driver.app->HandlePrimaryTouch(x, y, isDown);
}

// The first finger goes to the app as the mouse. Keys aren't handled, so the system acts on Back (finishing the
// activity).
int32_t HandleInput(android_app* androidApp, AInputEvent* event) {
    AndroidDriver& driver = *static_cast<AndroidDriver*>(androidApp->userData);
    if (driver.app == nullptr || AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION) {
        return 0;
    }

    const int32_t action = AMotionEvent_getAction(event);
    const size_t actionIndex = static_cast<size_t>((action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                                                   AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
    size_t index = 0;
    switch (action & AMOTION_EVENT_ACTION_MASK) {
        case AMOTION_EVENT_ACTION_DOWN:
            driver.primaryPointerId = AMotionEvent_getPointerId(event, 0);
            ForwardPrimaryTouch(driver, event, 0, true);
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            if (driver.primaryPointerId >= 0 && FindPointer(event, driver.primaryPointerId, index)) {
                ForwardPrimaryTouch(driver, event, index, true);
            }
            break;
        case AMOTION_EVENT_ACTION_POINTER_UP:
            if (AMotionEvent_getPointerId(event, actionIndex) == driver.primaryPointerId) {
                ForwardPrimaryTouch(driver, event, actionIndex, false);
                driver.primaryPointerId = -1;
            }
            break;
        // The last finger lifted, or the system took the gesture: both read as a release.
        case AMOTION_EVENT_ACTION_UP:
        case AMOTION_EVENT_ACTION_CANCEL:
            if (driver.primaryPointerId >= 0) {
                if (!FindPointer(event, driver.primaryPointerId, index)) {
                    index = 0;
                }
                ForwardPrimaryTouch(driver, event, index, false);
                driver.primaryPointerId = -1;
            }
            break;
        default:
            break;
    }
    return 1;
}

}  // namespace

void android_main(android_app* androidApp) {
    AndroidDriver driver;
    driver.androidApp = androidApp;
    androidApp->userData = &driver;
    androidApp->onAppCmd = HandleCommand;
    androidApp->onInputEvent = HandleInput;

    while (androidApp->destroyRequested == 0) {
        // Handle every pending event; with nothing to draw, wait for the next one instead.
        for (;;) {
            android_poll_source* source = nullptr;
            const int timeout = driver.IsActive() ? 0 : -1;
            if (ALooper_pollOnce(timeout, nullptr, nullptr, reinterpret_cast<void**>(&source)) < 0) {
                break;
            }
            if (source != nullptr) {
                source->process(androidApp, source);
            }
            if (androidApp->destroyRequested != 0) {
                break;
            }
        }

        if (driver.IsActive() && !driver.app->Tick()) {
            // The game asked to quit: finish the activity, which ends with APP_CMD_DESTROY.
            driver.quit = true;
            driver.app->Pause();
            ANativeActivity_finish(androidApp->activity);
        }
    }

    delete driver.app;  // shuts the app down
    driver.app = nullptr;
}

#endif  // ME_ANDROID
