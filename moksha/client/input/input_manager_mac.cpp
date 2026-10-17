#ifdef ME_MAC

#include "input_manager_mac.h"

#include <unordered_map>

using namespace ME::Input;

// macOS virtual keycode (from Carbon HIToolbox/Events.h) to game KeyCode conversion map.
static std::unordered_map<uint16_t, KeyCode> KeyCodeMap = {
    // letters
    {0x00, KeyCode::A},
    {0x0B, KeyCode::B},
    {0x08, KeyCode::C},
    {0x02, KeyCode::D},
    {0x0E, KeyCode::E},
    {0x03, KeyCode::F},
    {0x05, KeyCode::G},
    {0x04, KeyCode::H},
    {0x22, KeyCode::I},
    {0x26, KeyCode::J},
    {0x28, KeyCode::K},
    {0x25, KeyCode::L},
    {0x2E, KeyCode::M},
    {0x2D, KeyCode::N},
    {0x1F, KeyCode::O},
    {0x23, KeyCode::P},
    {0x0C, KeyCode::Q},
    {0x0F, KeyCode::R},
    {0x01, KeyCode::S},
    {0x11, KeyCode::T},
    {0x20, KeyCode::U},
    {0x09, KeyCode::V},
    {0x0D, KeyCode::W},
    {0x07, KeyCode::X},
    {0x10, KeyCode::Y},
    {0x06, KeyCode::Z},

    // top row numbers
    {0x12, KeyCode::Alpha1},
    {0x13, KeyCode::Alpha2},
    {0x14, KeyCode::Alpha3},
    {0x15, KeyCode::Alpha4},
    {0x17, KeyCode::Alpha5},
    {0x16, KeyCode::Alpha6},
    {0x1A, KeyCode::Alpha7},
    {0x1C, KeyCode::Alpha8},
    {0x19, KeyCode::Alpha9},
    {0x1D, KeyCode::Alpha0},

    // control / misc
    {0x33, KeyCode::Backspace},  // kVK_Delete (labeled "delete" on mac keyboards, behaves as backspace)
    {0x30, KeyCode::Tab},        // kVK_Tab
    {0x24, KeyCode::Enter},      // kVK_Return
    {0x35, KeyCode::Escape},     // kVK_Escape
    {0x31, KeyCode::Space},      // kVK_Space
    {0x39, KeyCode::CapsLock},   // kVK_CapsLock

    // navigation
    {0x74, KeyCode::PageUp},    // kVK_PageUp
    {0x79, KeyCode::PageDown},  // kVK_PageDown
    {0x77, KeyCode::End},       // kVK_End
    {0x73, KeyCode::Home},      // kVK_Home
    {0x7B, KeyCode::LeftArrow},
    {0x7E, KeyCode::UpArrow},
    {0x7C, KeyCode::RightArrow},
    {0x7D, KeyCode::DownArrow},
    {0x75, KeyCode::Delete},  // kVK_ForwardDelete

    // numpad
    {0x52, KeyCode::NumPad0},
    {0x53, KeyCode::NumPad1},
    {0x54, KeyCode::NumPad2},
    {0x55, KeyCode::NumPad3},
    {0x56, KeyCode::NumPad4},
    {0x57, KeyCode::NumPad5},
    {0x58, KeyCode::NumPad6},
    {0x59, KeyCode::NumPad7},
    {0x5B, KeyCode::NumPad8},
    {0x5C, KeyCode::NumPad9},
    {0x43, KeyCode::Multiply},
    {0x45, KeyCode::Add},
    {0x4E, KeyCode::Subtract},
    {0x41, KeyCode::Decimal},
    {0x4B, KeyCode::Divide},

    // function keys
    {0x7A, KeyCode::F1},
    {0x78, KeyCode::F2},
    {0x63, KeyCode::F3},
    {0x76, KeyCode::F4},
    {0x60, KeyCode::F5},
    {0x61, KeyCode::F6},
    {0x62, KeyCode::F7},
    {0x64, KeyCode::F8},
    {0x65, KeyCode::F9},
    {0x6D, KeyCode::F10},
    {0x67, KeyCode::F11},
    {0x6F, KeyCode::F12},

    // locks / modifiers
    {0x38, KeyCode::LeftShift},
    {0x3C, KeyCode::RightShift},
    {0x3B, KeyCode::LeftCtrl},
    {0x3E, KeyCode::RightCtrl},
    {0x3A, KeyCode::LeftAlt},   // kVK_Option
    {0x3D, KeyCode::RightAlt},  // kVK_RightOption
    {0x37, KeyCode::LeftMeta},  // kVK_Command
    {0x36, KeyCode::RightMeta},

    // punctuation / symbols
    {0x29, KeyCode::Semicolon},
    {0x18, KeyCode::Equal},
    {0x2B, KeyCode::Comma},
    {0x1B, KeyCode::Minus},
    {0x2F, KeyCode::Period},
    {0x2C, KeyCode::Slash},
    {0x32, KeyCode::Grave},
    {0x21, KeyCode::LeftBracket},
    {0x2A, KeyCode::Backslash},
    {0x1E, KeyCode::RightBracket},
    {0x27, KeyCode::Apostrophe},
};

void InputManagerMac::Init() {}

void InputManagerMac::Update(double deltaTime) {}

void InputManagerMac::PostUpdate() {
    // Loop through all the keys and update their states for the next frame.
    for (auto& pair : InputManager::GlobalKeyState) {
        KeyCode key = pair.first;
        KeyState state = pair.second;

        switch (state) {
            case KeyState::DownWasUp:
                InputManager::GlobalKeyState[key] = KeyState::DownWasDown;
                break;
            case KeyState::UpWasDown:
                InputManager::GlobalKeyState[key] = KeyState::UpWasUp;
                break;
            default:
                break;
        }
    }

    InputManager::AdvanceMouseButtonStates();
}

void InputManagerMac::End() {}

void InputManagerMac::HandleKeyEvent(uint16_t keyCode, bool isDown) {
    auto it = KeyCodeMap.find(keyCode);
    if (it == KeyCodeMap.end()) {
        return;
    }

    KeyCode key = it->second;
    KeyState current = InputManager::GlobalKeyState[key];
    bool wasPreviouslyDown = (current == KeyState::DownWasDown || current == KeyState::DownWasUp);

    if (isDown) {
        InputManager::GlobalKeyState[key] = wasPreviouslyDown ? KeyState::DownWasDown : KeyState::DownWasUp;
    } else {
        InputManager::GlobalKeyState[key] = wasPreviouslyDown ? KeyState::UpWasDown : KeyState::UpWasUp;
    }
}

void InputManagerMac::HandleMouseMove(float x, float y) {
    InputManager::mousePos.x = static_cast<int32_t>(x);
    InputManager::mousePos.y = static_cast<int32_t>(y);
}

void InputManagerMac::HandleMouseButton(int button, bool isDown) {
    // MEView (metal_view.mm) only forwards button 0 (left) and 1 (right) today.
    if (button == 0) {
        InputManager::SetMouseButtonState(MouseButton::Left, isDown);
    } else if (button == 1) {
        InputManager::SetMouseButtonState(MouseButton::Right, isDown);
    }
}

#endif  // ME_MAC
