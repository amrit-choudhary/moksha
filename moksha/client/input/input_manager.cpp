#include "input_manager.h"
#if defined(ME_MAC) && !defined(ME_HEADLESS)
#include "input_manager_mac.h"
#endif
#if defined(ME_WIN) && !defined(ME_HEADLESS)
#include "input_manager_win.h"
#endif

using namespace ME::Input;

// Global key map with key states.
std::unordered_map<KeyCode, KeyState> InputManager::GlobalKeyState = {
    // None
    {KeyCode::None, KeyState::None},

    // Control / misc
    {KeyCode::Backspace, KeyState::UpWasUp},
    {KeyCode::Tab, KeyState::UpWasUp},
    {KeyCode::Enter, KeyState::UpWasUp},
    {KeyCode::Pause, KeyState::UpWasUp},
    {KeyCode::CapsLock, KeyState::UpWasUp},
    {KeyCode::Escape, KeyState::UpWasUp},
    {KeyCode::Space, KeyState::UpWasUp},

    // Navigation
    {KeyCode::PageUp, KeyState::UpWasUp},
    {KeyCode::PageDown, KeyState::UpWasUp},
    {KeyCode::End, KeyState::UpWasUp},
    {KeyCode::Home, KeyState::UpWasUp},
    {KeyCode::LeftArrow, KeyState::UpWasUp},
    {KeyCode::UpArrow, KeyState::UpWasUp},
    {KeyCode::RightArrow, KeyState::UpWasUp},
    {KeyCode::DownArrow, KeyState::UpWasUp},
    {KeyCode::Insert, KeyState::UpWasUp},
    {KeyCode::Delete, KeyState::UpWasUp},

    // Top row numbers
    {KeyCode::Alpha0, KeyState::UpWasUp},
    {KeyCode::Alpha1, KeyState::UpWasUp},
    {KeyCode::Alpha2, KeyState::UpWasUp},
    {KeyCode::Alpha3, KeyState::UpWasUp},
    {KeyCode::Alpha4, KeyState::UpWasUp},
    {KeyCode::Alpha5, KeyState::UpWasUp},
    {KeyCode::Alpha6, KeyState::UpWasUp},
    {KeyCode::Alpha7, KeyState::UpWasUp},
    {KeyCode::Alpha8, KeyState::UpWasUp},
    {KeyCode::Alpha9, KeyState::UpWasUp},

    // Letters
    {KeyCode::A, KeyState::UpWasUp},
    {KeyCode::B, KeyState::UpWasUp},
    {KeyCode::C, KeyState::UpWasUp},
    {KeyCode::D, KeyState::UpWasUp},
    {KeyCode::E, KeyState::UpWasUp},
    {KeyCode::F, KeyState::UpWasUp},
    {KeyCode::G, KeyState::UpWasUp},
    {KeyCode::H, KeyState::UpWasUp},
    {KeyCode::I, KeyState::UpWasUp},
    {KeyCode::J, KeyState::UpWasUp},
    {KeyCode::K, KeyState::UpWasUp},
    {KeyCode::L, KeyState::UpWasUp},
    {KeyCode::M, KeyState::UpWasUp},
    {KeyCode::N, KeyState::UpWasUp},
    {KeyCode::O, KeyState::UpWasUp},
    {KeyCode::P, KeyState::UpWasUp},
    {KeyCode::Q, KeyState::UpWasUp},
    {KeyCode::R, KeyState::UpWasUp},
    {KeyCode::S, KeyState::UpWasUp},
    {KeyCode::T, KeyState::UpWasUp},
    {KeyCode::U, KeyState::UpWasUp},
    {KeyCode::V, KeyState::UpWasUp},
    {KeyCode::W, KeyState::UpWasUp},
    {KeyCode::X, KeyState::UpWasUp},
    {KeyCode::Y, KeyState::UpWasUp},
    {KeyCode::Z, KeyState::UpWasUp},

    // Numpad
    {KeyCode::NumPad0, KeyState::UpWasUp},
    {KeyCode::NumPad1, KeyState::UpWasUp},
    {KeyCode::NumPad2, KeyState::UpWasUp},
    {KeyCode::NumPad3, KeyState::UpWasUp},
    {KeyCode::NumPad4, KeyState::UpWasUp},
    {KeyCode::NumPad5, KeyState::UpWasUp},
    {KeyCode::NumPad6, KeyState::UpWasUp},
    {KeyCode::NumPad7, KeyState::UpWasUp},
    {KeyCode::NumPad8, KeyState::UpWasUp},
    {KeyCode::NumPad9, KeyState::UpWasUp},
    {KeyCode::Multiply, KeyState::UpWasUp},
    {KeyCode::Add, KeyState::UpWasUp},
    {KeyCode::Subtract, KeyState::UpWasUp},
    {KeyCode::Decimal, KeyState::UpWasUp},
    {KeyCode::Divide, KeyState::UpWasUp},

    // Function keys
    {KeyCode::F1, KeyState::UpWasUp},
    {KeyCode::F2, KeyState::UpWasUp},
    {KeyCode::F3, KeyState::UpWasUp},
    {KeyCode::F4, KeyState::UpWasUp},
    {KeyCode::F5, KeyState::UpWasUp},
    {KeyCode::F6, KeyState::UpWasUp},
    {KeyCode::F7, KeyState::UpWasUp},
    {KeyCode::F8, KeyState::UpWasUp},
    {KeyCode::F9, KeyState::UpWasUp},
    {KeyCode::F10, KeyState::UpWasUp},
    {KeyCode::F11, KeyState::UpWasUp},
    {KeyCode::F12, KeyState::UpWasUp},

    // Locks / modifiers
    {KeyCode::NumLock, KeyState::UpWasUp},
    {KeyCode::ScrollLock, KeyState::UpWasUp},
    {KeyCode::LeftShift, KeyState::UpWasUp},
    {KeyCode::RightShift, KeyState::UpWasUp},
    {KeyCode::LeftCtrl, KeyState::UpWasUp},
    {KeyCode::RightCtrl, KeyState::UpWasUp},
    {KeyCode::LeftAlt, KeyState::UpWasUp},
    {KeyCode::RightAlt, KeyState::UpWasUp},
    {KeyCode::LeftMeta, KeyState::UpWasUp},
    {KeyCode::RightMeta, KeyState::UpWasUp},

    // Punctuation / symbols
    {KeyCode::Semicolon, KeyState::UpWasUp},
    {KeyCode::Equal, KeyState::UpWasUp},
    {KeyCode::Comma, KeyState::UpWasUp},
    {KeyCode::Minus, KeyState::UpWasUp},
    {KeyCode::Period, KeyState::UpWasUp},
    {KeyCode::Slash, KeyState::UpWasUp},
    {KeyCode::Grave, KeyState::UpWasUp},
    {KeyCode::LeftBracket, KeyState::UpWasUp},
    {KeyCode::Backslash, KeyState::UpWasUp},
    {KeyCode::RightBracket, KeyState::UpWasUp},
    {KeyCode::Apostrophe, KeyState::UpWasUp},

    // Sentinel / max
    {KeyCode::MaxValue, KeyState::None}};

KeyState InputManager::GlobalMouseButtonState[InputManager::kMouseButtonCount] = {
    KeyState::UpWasUp, KeyState::UpWasUp, KeyState::UpWasUp, KeyState::UpWasUp, KeyState::UpWasUp};

InputManager::InputManager() {}

InputManager::~InputManager() {}

void InputManager::Init(const ME::WindowState* inWindow) {
    window = inWindow;
#if defined(ME_MAC) && !defined(ME_HEADLESS)
    platformInputManager = new InputManagerMac();
#elif defined(ME_WIN) && !defined(ME_HEADLESS)
    platformInputManager = new InputManagerWin();
#else
    // Headless, or an OS without an input backend yet: the base manager does nothing and every key stays up.
    platformInputManager = new PlatformInputManager();
#endif

    platformInputManager->Init();
}

void ME::Input::InputManager::PreUpdate() {
    platformInputManager->PreUpdate();
}

void InputManager::Update(double deltaTime) {
    platformInputManager->Update(deltaTime);
}

void ME::Input::InputManager::PostUpdate() {
    platformInputManager->PostUpdate();
}

void InputManager::End() {
    platformInputManager->End();
}

bool InputManager::GetKeyDown(KeyCode keyCode) {
    return (GlobalKeyState[keyCode] == KeyState::DownWasDown || GlobalKeyState[keyCode] == KeyState::DownWasUp);
}

bool InputManager::GetKeyUp(KeyCode keyCode) {
    return (GlobalKeyState[keyCode] == KeyState::UpWasUp || GlobalKeyState[keyCode] == KeyState::UpWasDown);
}

bool ME::Input::InputManager::GetKeyReleased(ME::Input::KeyCode keyCode) {
    const size_t index = static_cast<size_t>(keyCode);
    return index < kKeyCount && keyReleased[index];
}

bool ME::Input::InputManager::GetKeyPressed(ME::Input::KeyCode keyCode) {
    const size_t index = static_cast<size_t>(keyCode);
    return index < kKeyCount && keyPressed[index];
}

bool InputManager::GetMouseButtonDown(MouseButton button) {
    KeyState state = GlobalMouseButtonState[static_cast<uint8_t>(button)];
    return state == KeyState::DownWasDown || state == KeyState::DownWasUp;
}

bool InputManager::GetMouseButtonUp(MouseButton button) {
    KeyState state = GlobalMouseButtonState[static_cast<uint8_t>(button)];
    return state == KeyState::UpWasUp || state == KeyState::UpWasDown;
}

bool InputManager::GetMouseButtonReleased(MouseButton button) {
    const size_t index = static_cast<size_t>(button);
    return index < kMouseButtonCount && mouseButtonReleased[index];
}

bool InputManager::GetMouseButtonPressed(MouseButton button) {
    const size_t index = static_cast<size_t>(button);
    return index < kMouseButtonCount && mouseButtonPressed[index];
}

// Moves state to down or up and reports the edge: 1 pressed, -1 released, 0 no change (e.g. key repeat).
static int ApplyTransition(KeyState& state, bool isDown) {
    const bool wasDown = (state == KeyState::DownWasDown || state == KeyState::DownWasUp);
    if (isDown) {
        state = wasDown ? KeyState::DownWasDown : KeyState::DownWasUp;
        return wasDown ? 0 : 1;
    }
    state = wasDown ? KeyState::UpWasDown : KeyState::UpWasUp;
    return wasDown ? -1 : 0;
}

void InputManager::SetKeyState(KeyCode keyCode, bool isDown) {
    const int edge = ApplyTransition(GlobalKeyState[keyCode], isDown);
    const size_t index = static_cast<size_t>(keyCode);
    if (index >= kKeyCount) {
        return;
    }
    keyPressed[index] = keyPressed[index] || edge > 0;
    keyReleased[index] = keyReleased[index] || edge < 0;
}

void InputManager::SetMouseButtonState(MouseButton button, bool isDown) {
    const size_t index = static_cast<size_t>(button);
    if (index >= kMouseButtonCount) {
        return;
    }
    const int edge = ApplyTransition(GlobalMouseButtonState[index], isDown);
    mouseButtonPressed[index] = mouseButtonPressed[index] || edge > 0;
    mouseButtonReleased[index] = mouseButtonReleased[index] || edge < 0;
}

// Settles a just-pressed or just-released state into held or up.
static void AdvanceState(KeyState& state) {
    if (state == KeyState::DownWasUp) {
        state = KeyState::DownWasDown;
    } else if (state == KeyState::UpWasDown) {
        state = KeyState::UpWasUp;
    }
}

void InputManager::AdvanceStates() {
    for (auto& pair : GlobalKeyState) {
        AdvanceState(pair.second);
    }
    for (size_t i = 0; i < kMouseButtonCount; ++i) {
        AdvanceState(GlobalMouseButtonState[i]);
    }

    for (size_t i = 0; i < kKeyCount; ++i) {
        keyPressed[i] = false;
        keyReleased[i] = false;
    }
    for (size_t i = 0; i < kMouseButtonCount; ++i) {
        mouseButtonPressed[i] = false;
        mouseButtonReleased[i] = false;
    }
}

PlatformInputManager* ME::Input::InputManager::GetPlatformInputManager() {
    return platformInputManager;
}

PlatformInputManager::PlatformInputManager() {}

PlatformInputManager::~PlatformInputManager() {}

void PlatformInputManager::Init() {}

void ME::Input::PlatformInputManager::PreUpdate() {}

void PlatformInputManager::Update(double deltaTime) {}

void ME::Input::PlatformInputManager::PostUpdate() {}

void PlatformInputManager::End() {}
