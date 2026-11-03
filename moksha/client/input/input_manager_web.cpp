#ifdef ME_WEB

#include "input_manager_web.h"

#include <string>
#include <unordered_map>

using namespace ME::Input;

// DOM KeyboardEvent.code to game KeyCode conversion map.
static const std::unordered_map<std::string, KeyCode> KeyCodeMap = {
    // letters
    {"KeyA", KeyCode::A},
    {"KeyB", KeyCode::B},
    {"KeyC", KeyCode::C},
    {"KeyD", KeyCode::D},
    {"KeyE", KeyCode::E},
    {"KeyF", KeyCode::F},
    {"KeyG", KeyCode::G},
    {"KeyH", KeyCode::H},
    {"KeyI", KeyCode::I},
    {"KeyJ", KeyCode::J},
    {"KeyK", KeyCode::K},
    {"KeyL", KeyCode::L},
    {"KeyM", KeyCode::M},
    {"KeyN", KeyCode::N},
    {"KeyO", KeyCode::O},
    {"KeyP", KeyCode::P},
    {"KeyQ", KeyCode::Q},
    {"KeyR", KeyCode::R},
    {"KeyS", KeyCode::S},
    {"KeyT", KeyCode::T},
    {"KeyU", KeyCode::U},
    {"KeyV", KeyCode::V},
    {"KeyW", KeyCode::W},
    {"KeyX", KeyCode::X},
    {"KeyY", KeyCode::Y},
    {"KeyZ", KeyCode::Z},

    // top row numbers
    {"Digit0", KeyCode::Alpha0},
    {"Digit1", KeyCode::Alpha1},
    {"Digit2", KeyCode::Alpha2},
    {"Digit3", KeyCode::Alpha3},
    {"Digit4", KeyCode::Alpha4},
    {"Digit5", KeyCode::Alpha5},
    {"Digit6", KeyCode::Alpha6},
    {"Digit7", KeyCode::Alpha7},
    {"Digit8", KeyCode::Alpha8},
    {"Digit9", KeyCode::Alpha9},

    // numpad
    {"Numpad0", KeyCode::NumPad0},
    {"Numpad1", KeyCode::NumPad1},
    {"Numpad2", KeyCode::NumPad2},
    {"Numpad3", KeyCode::NumPad3},
    {"Numpad4", KeyCode::NumPad4},
    {"Numpad5", KeyCode::NumPad5},
    {"Numpad6", KeyCode::NumPad6},
    {"Numpad7", KeyCode::NumPad7},
    {"Numpad8", KeyCode::NumPad8},
    {"Numpad9", KeyCode::NumPad9},
    {"NumpadMultiply", KeyCode::Multiply},
    {"NumpadAdd", KeyCode::Add},
    {"NumpadSubtract", KeyCode::Subtract},
    {"NumpadDecimal", KeyCode::Decimal},
    {"NumpadDivide", KeyCode::Divide},

    // control / misc
    {"Backspace", KeyCode::Backspace},
    {"Tab", KeyCode::Tab},
    {"Enter", KeyCode::Enter},
    {"Pause", KeyCode::Pause},
    {"CapsLock", KeyCode::CapsLock},
    {"Escape", KeyCode::Escape},
    {"Space", KeyCode::Space},

    // navigation
    {"PageUp", KeyCode::PageUp},
    {"PageDown", KeyCode::PageDown},
    {"End", KeyCode::End},
    {"Home", KeyCode::Home},
    {"ArrowLeft", KeyCode::LeftArrow},
    {"ArrowUp", KeyCode::UpArrow},
    {"ArrowRight", KeyCode::RightArrow},
    {"ArrowDown", KeyCode::DownArrow},
    {"Insert", KeyCode::Insert},
    {"Delete", KeyCode::Delete},

    // function keys
    {"F1", KeyCode::F1},
    {"F2", KeyCode::F2},
    {"F3", KeyCode::F3},
    {"F4", KeyCode::F4},
    {"F5", KeyCode::F5},
    {"F6", KeyCode::F6},
    {"F7", KeyCode::F7},
    {"F8", KeyCode::F8},
    {"F9", KeyCode::F9},
    {"F10", KeyCode::F10},
    {"F11", KeyCode::F11},
    {"F12", KeyCode::F12},

    // locks / modifiers
    {"NumLock", KeyCode::NumLock},
    {"ScrollLock", KeyCode::ScrollLock},
    {"ShiftLeft", KeyCode::LeftShift},
    {"ShiftRight", KeyCode::RightShift},
    {"ControlLeft", KeyCode::LeftCtrl},
    {"ControlRight", KeyCode::RightCtrl},
    {"AltLeft", KeyCode::LeftAlt},
    {"AltRight", KeyCode::RightAlt},
    {"MetaLeft", KeyCode::LeftMeta},
    {"MetaRight", KeyCode::RightMeta},

    // punctuation / symbols
    {"Semicolon", KeyCode::Semicolon},
    {"Equal", KeyCode::Equal},
    {"Comma", KeyCode::Comma},
    {"Minus", KeyCode::Minus},
    {"Period", KeyCode::Period},
    {"Slash", KeyCode::Slash},
    {"Backquote", KeyCode::Grave},
    {"BracketLeft", KeyCode::LeftBracket},
    {"Backslash", KeyCode::Backslash},
    {"BracketRight", KeyCode::RightBracket},
    {"Quote", KeyCode::Apostrophe},
};

void InputManagerWeb::PostUpdate() {
    InputManager::AdvanceStates();
}

void InputManagerWeb::HandleKeyEvent(const char* code, bool isDown) {
    auto it = KeyCodeMap.find(code);
    if (it == KeyCodeMap.end()) {
        return;
    }

    InputManager::SetKeyState(it->second, isDown);
}

void InputManagerWeb::HandleMouseMove(float x, float y) {
    InputManager::mousePos.x = static_cast<int32_t>(x);
    InputManager::mousePos.y = static_cast<int32_t>(y);
}

void InputManagerWeb::HandleMouseButton(int button, bool isDown) {
    if (button >= 0 && button < static_cast<int>(InputManager::kMouseButtonCount)) {
        InputManager::SetMouseButtonState(static_cast<MouseButton>(button), isDown);
    }
}

void InputManagerWeb::ReleaseAll() {
    for (const auto& entry : KeyCodeMap) {
        InputManager::SetKeyState(entry.second, false);
    }
    for (size_t i = 0; i < InputManager::kMouseButtonCount; ++i) {
        InputManager::SetMouseButtonState(static_cast<MouseButton>(i), false);
    }
}

#endif  // ME_WEB
