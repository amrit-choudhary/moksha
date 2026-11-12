#ifdef ME_ANDROID
/**
 * Input manager for Android. The first finger on the screen drives the left mouse button and the mouse position, so
 * mouse-driven games work unchanged; other fingers are ignored until the engine has a touch API.
 */

#pragma once

#include "input_manager.h"

namespace ME::Input {

class InputManagerAndroid : public PlatformInputManager {
   public:
    void PostUpdate() override;

    // The first finger, forwarded from the driver (platform/android) through ClientApp: its position in window units
    // (top-left origin) and whether it is still on the screen.
    void HandlePrimaryTouch(float x, float y, bool isDown);
};

}  // namespace ME::Input

#endif  // ME_ANDROID
