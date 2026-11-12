#ifdef ME_ANDROID

/**
 * Android entry and frame driver. The NDK's native_app_glue (compiled into the entry library) receives the
 * NativeActivity's callbacks and calls android_main on its own thread, then forwards events through a looper.
 */

#include <android/looper.h>
#include <android_native_app_glue.h>

// TODO: create and drive ClientApp (Init on the first window, Tick once per loop, pause and resume). For now it only
// handles the activity's events until the activity is destroyed.
void android_main(android_app* app) {
    while (app->destroyRequested == 0) {
        android_poll_source* source = nullptr;
        if (ALooper_pollOnce(-1, nullptr, nullptr, reinterpret_cast<void**>(&source)) >= 0 && source != nullptr) {
            source->process(app, source);
        }
    }
}

#endif  // ME_ANDROID
