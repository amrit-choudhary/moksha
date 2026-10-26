#ifdef ME_MAC
/**
 * Input manager for Mac
 */

#pragma once

#include <cstdint>

#include "input_manager.h"

namespace ME::Input {

class InputManagerMac : public PlatformInputManager {
   public:
    void Init() override;
    void Update(double deltaTime) override;
    void PostUpdate() override;
    void End() override;

    // macOS input events, forwarded from the view (platform/mac) through ClientApp.
    void HandleKeyEvent(uint16_t keyCode, bool isDown);
    void HandleMouseMove(float x, float y);
    void HandleMouseButton(int button, bool isDown);
};

}  // namespace ME::Input

#endif  // ME_MAC
