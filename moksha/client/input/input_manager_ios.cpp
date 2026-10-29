#ifdef ME_IOS

#include "input_manager_ios.h"

using namespace ME::Input;

void InputManagerIos::PostUpdate() {
    InputManager::AdvanceStates();
}

void InputManagerIos::HandlePrimaryTouch(float x, float y, bool isDown) {
    InputManager::mousePos.x = static_cast<int32_t>(x);
    InputManager::mousePos.y = static_cast<int32_t>(y);
    InputManager::SetMouseButtonState(MouseButton::Left, isDown);
}

#endif  // ME_IOS
