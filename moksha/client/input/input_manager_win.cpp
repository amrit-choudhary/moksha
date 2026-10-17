#ifdef ME_WIN

#include "input_manager_win.h"

#include "moksha/client/misc/global_vars.h"
#include "moksha/logging/logging.h"

using namespace ME::Input;

// Native keycode (Virtual Key) to game KeyCode conversion map.
static std::unordered_map<uint16_t, KeyCode> KeyCodeMap = {
    // control / misc
    {0x08, KeyCode::Backspace},  // VK_BACK
    {0x09, KeyCode::Tab},        // VK_TAB
    {0x0D, KeyCode::Enter},      // VK_RETURN
    {0x13, KeyCode::Pause},      // VK_PAUSE
    {0x14, KeyCode::CapsLock},   // VK_CAPITAL
    {0x1B, KeyCode::Escape},     // VK_ESCAPE
    {0x20, KeyCode::Space},      // VK_SPACE

    // navigation
    {0x21, KeyCode::PageUp},      // VK_PRIOR
    {0x22, KeyCode::PageDown},    // VK_NEXT
    {0x23, KeyCode::End},         // VK_END
    {0x24, KeyCode::Home},        // VK_HOME
    {0x25, KeyCode::LeftArrow},   // VK_LEFT
    {0x26, KeyCode::UpArrow},     // VK_UP
    {0x27, KeyCode::RightArrow},  // VK_RIGHT
    {0x28, KeyCode::DownArrow},   // VK_DOWN
    {0x2D, KeyCode::Insert},      // VK_INSERT
    {0x2E, KeyCode::Delete},      // VK_DELETE

    // top row numbers ('0'..'9')
    {'0', KeyCode::Alpha0},
    {'1', KeyCode::Alpha1},
    {'2', KeyCode::Alpha2},
    {'3', KeyCode::Alpha3},
    {'4', KeyCode::Alpha4},
    {'5', KeyCode::Alpha5},
    {'6', KeyCode::Alpha6},
    {'7', KeyCode::Alpha7},
    {'8', KeyCode::Alpha8},
    {'9', KeyCode::Alpha9},

    // letters
    {'A', KeyCode::A},
    {'B', KeyCode::B},
    {'C', KeyCode::C},
    {'D', KeyCode::D},
    {'E', KeyCode::E},
    {'F', KeyCode::F},
    {'G', KeyCode::G},
    {'H', KeyCode::H},
    {'I', KeyCode::I},
    {'J', KeyCode::J},
    {'K', KeyCode::K},
    {'L', KeyCode::L},
    {'M', KeyCode::M},
    {'N', KeyCode::N},
    {'O', KeyCode::O},
    {'P', KeyCode::P},
    {'Q', KeyCode::Q},
    {'R', KeyCode::R},
    {'S', KeyCode::S},
    {'T', KeyCode::T},
    {'U', KeyCode::U},
    {'V', KeyCode::V},
    {'W', KeyCode::W},
    {'X', KeyCode::X},
    {'Y', KeyCode::Y},
    {'Z', KeyCode::Z},

    // numpad
    {0x60, KeyCode::NumPad0},
    {0x61, KeyCode::NumPad1},
    {0x62, KeyCode::NumPad2},
    {0x63, KeyCode::NumPad3},
    {0x64, KeyCode::NumPad4},
    {0x65, KeyCode::NumPad5},
    {0x66, KeyCode::NumPad6},
    {0x67, KeyCode::NumPad7},
    {0x68, KeyCode::NumPad8},
    {0x69, KeyCode::NumPad9},
    {0x6A, KeyCode::Multiply},
    {0x6B, KeyCode::Add},
    {0x6D, KeyCode::Subtract},
    {0x6E, KeyCode::Decimal},
    {0x6F, KeyCode::Divide},

    // function keys
    {0x70, KeyCode::F1},
    {0x71, KeyCode::F2},
    {0x72, KeyCode::F3},
    {0x73, KeyCode::F4},
    {0x74, KeyCode::F5},
    {0x75, KeyCode::F6},
    {0x76, KeyCode::F7},
    {0x77, KeyCode::F8},
    {0x78, KeyCode::F9},
    {0x79, KeyCode::F10},
    {0x7A, KeyCode::F11},
    {0x7B, KeyCode::F12},

    // locks / modifiers
    {0x90, KeyCode::NumLock},     // VK_NUMLOCK
    {0x91, KeyCode::ScrollLock},  // VK_SCROLL
    {0xA0, KeyCode::LeftShift},   // VK_LSHIFT
    {0xA1, KeyCode::RightShift},  // VK_RSHIFT
    {0xA2, KeyCode::LeftCtrl},    // VK_LCONTROL
    {0xA3, KeyCode::RightCtrl},   // VK_RCONTROL
    {0xA4, KeyCode::LeftAlt},     // VK_LMENU
    {0xA5, KeyCode::RightAlt},    // VK_RMENU

    // windows/meta keys
    {0x5B, KeyCode::LeftMeta},   // VK_LWIN
    {0x5C, KeyCode::RightMeta},  // VK_RWIN

    // punctuation / OEM keys (common US layouts)
    {0xBA, KeyCode::Semicolon},     // VK_OEM_1  ';:'
    {0xBB, KeyCode::Equal},         // VK_OEM_PLUS '='
    {0xBC, KeyCode::Comma},         // VK_OEM_COMMA ','
    {0xBD, KeyCode::Minus},         // VK_OEM_MINUS '-'
    {0xBE, KeyCode::Period},        // VK_OEM_PERIOD '.'
    {0xBF, KeyCode::Slash},         // VK_OEM_2  '/'
    {0xC0, KeyCode::Grave},         // VK_OEM_3  '`'
    {0xDB, KeyCode::LeftBracket},   // VK_OEM_4  '['
    {0xDC, KeyCode::Backslash},     // VK_OEM_5  '\'
    {0xDD, KeyCode::RightBracket},  // VK_OEM_6  ']'
    {0xDE, KeyCode::Apostrophe},    // VK_OEM_7  '''

    // fallback / sentinel
    {0xFFFF, KeyCode::MaxValue}};

InputManagerWin::InputManagerWin() {
    bufferFirst = new WinKeyEventBuffer();
    bufferSecond = new WinKeyEventBuffer();
    readBuffer = bufferFirst;
    writeBuffer = bufferSecond;
    bReadingFirstBuffer = true;
}

InputManagerWin::~InputManagerWin() {
    delete bufferFirst;
    bufferFirst = nullptr;
    delete bufferSecond;
    bufferSecond = nullptr;
    readBuffer = nullptr;
    writeBuffer = nullptr;
}

void InputManagerWin::Init() {}

void ME::Input::InputManagerWin::PreUpdate() {
    // Swap the read and write buffers.
    if (bReadingFirstBuffer) {
        readBuffer = bufferSecond;
        writeBuffer = bufferFirst;
    } else {
        readBuffer = bufferFirst;
        writeBuffer = bufferSecond;
    }
    bReadingFirstBuffer = !bReadingFirstBuffer;

    // Reset the write buffer for new input events.
    writeBuffer->Reset();
}

void InputManagerWin::Update(double deltaTime) {
    for (int i = 0; i < readBuffer->count; i++) {
        WinKeyEvent& event = readBuffer->buffer[i];

        // use pointer-sized type to avoid truncation on x64
        uintptr_t lp = static_cast<uintptr_t>(event.lParam);
        constexpr uintptr_t TRANSITION_MASK = (uintptr_t{1} << 31);  // 1 => transition = key released
        constexpr uintptr_t PREV_STATE_MASK = (uintptr_t{1} << 30);  // 1 => key was previously down

        bool isKeyUp = (lp & TRANSITION_MASK) != 0;
        bool wasPreviouslyDown = (lp & PREV_STATE_MASK) != 0;

        // On Key Press Down
        if (!isKeyUp) {
            KeyCode keyCode = KeyCodeMap[static_cast<uint16_t>(event.wParam)];
            if (wasPreviouslyDown) {
                // Key is being held down
                InputManager::GlobalKeyState[keyCode] = KeyState::DownWasDown;
            } else {
                // Key was just pressed
                InputManager::GlobalKeyState[keyCode] = KeyState::DownWasUp;
            }
        }

        // On Key Released
        if (isKeyUp) {
            KeyCode keyCode = KeyCodeMap[static_cast<uint16_t>(event.wParam)];
            if (wasPreviouslyDown) {
                // Key was just released
                InputManager::GlobalKeyState[keyCode] = KeyState::UpWasDown;
            } else {
                // Key is up and was previously up
                InputManager::GlobalKeyState[keyCode] = KeyState::UpWasUp;
            }
        }
    }

    // After processing all the events, reset the read buffer.
    readBuffer->Reset();
}

void ME::Input::InputManagerWin::PostUpdate() {
    // Loop through all the keys and update their states for the next frame.
    for (auto& pair : InputManager::GlobalKeyState) {
        KeyCode key = pair.first;
        KeyState state = pair.second;

        switch (state) {
            case KeyState::DownWasUp:
                // Transition from just pressed to being held down.
                InputManager::GlobalKeyState[key] = KeyState::DownWasDown;
                break;
            case KeyState::UpWasDown:
                // Transition from just released to being up.
                InputManager::GlobalKeyState[key] = KeyState::UpWasUp;
                break;
            default:
                // No state change needed for DownWasDown and UpWasUp.
                break;
        }
    }

    InputManager::AdvanceMouseButtonStates();
}

void InputManagerWin::End() {}

void InputManagerWin::HandleInput(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_MOUSEMOVE || msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_RBUTTONDOWN ||
        msg == WM_RBUTTONUP) {
        if (msg == WM_MOUSEMOVE) {
            // Win32 client coords are already top-left/Y-down. Signed cast handles negative
            // multi-monitor coordinates correctly, unlike plain LOWORD/HIWORD.
            InputManager::mousePos.x = static_cast<int16_t>(LOWORD(lParam));
            InputManager::mousePos.y = static_cast<int16_t>(HIWORD(lParam));
        } else {
            MouseButton button =
                (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) ? MouseButton::Left : MouseButton::Right;
            bool isDown = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN);
            InputManager::SetMouseButtonState(button, isDown);
        }
    } else {
        writeBuffer->Push(msg, wParam, lParam);
    }
}

ME::Input::WinKeyEventBuffer::WinKeyEventBuffer() {
    buffer = new WinKeyEvent[MAX_KEY_EVENTS];
    count = 0;
}

ME::Input::WinKeyEventBuffer::~WinKeyEventBuffer() {
    delete[] buffer;
}

void ME::Input::WinKeyEventBuffer::Push(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (count >= MAX_KEY_EVENTS) {
        ME::LogError("WinKeyEventBuffer: Key event buffer overflow.");
        return;
    }
    buffer[count] = WinKeyEvent(msg, wParam, lParam);
    ++count;
}

void ME::Input::WinKeyEventBuffer::Reset() {
    count = 0;
}

ME::Input::WinKeyEvent::WinKeyEvent(UINT msg, WPARAM wParam, LPARAM lParam)
    : msg(msg), wParam(wParam), lParam(lParam) {}

#endif  // ME_WIN
