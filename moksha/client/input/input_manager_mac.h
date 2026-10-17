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

    // Handle macOS-specific input events, forwarded synchronously from MEView (NSEvent-derived).
    void HandleKeyEvent(uint16_t keyCode, bool isDown);
    void HandleMouseMove(float x, float y);
    void HandleMouseButton(int button, bool isDown);
};

}  // namespace ME::Input

#endif  // ME_MAC
