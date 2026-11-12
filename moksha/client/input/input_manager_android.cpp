#ifdef ME_ANDROID

#include "input_manager_android.h"

using namespace ME::Input;

void InputManagerAndroid::PostUpdate() {
    InputManager::AdvanceStates();
}

void InputManagerAndroid::HandlePrimaryTouch(float x, float y, bool isDown) {
    InputManager::mousePos.x = static_cast<int32_t>(x);
    InputManager::mousePos.y = static_cast<int32_t>(y);
    InputManager::SetMouseButtonState(MouseButton::Left, isDown);
}

#endif  // ME_ANDROID
