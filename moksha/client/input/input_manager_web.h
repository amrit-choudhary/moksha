#ifdef ME_WEB
/**
 * Input manager for the web. Keys arrive as DOM `KeyboardEvent.code` names, which follow the key's position, not the
 * keyboard layout. The first finger on a touch screen drives the left mouse button and the mouse position, as on iOS.
 */

#pragma once

#include "input_manager.h"

namespace ME::Input {

class InputManagerWeb : public PlatformInputManager {
   public:
    void PostUpdate() override;

    // Browser input events, forwarded from the page (platform/web) through ClientApp. Positions are in window units,
    // top-left origin. Button numbers follow MouseButton.
    void HandleKeyEvent(const char* code, bool isDown);
    void HandleMouseMove(float x, float y);
    void HandleMouseButton(int button, bool isDown);

    // The page lost focus, so no key-up will arrive: every key and button reads as released.
    void ReleaseAll();
};

}  // namespace ME::Input

#endif  // ME_WEB
