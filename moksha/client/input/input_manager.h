/**
 * Input related functionality.
 */

#pragma once

#include <cstdint>
#include <string>
#include <thread>
#include <unordered_map>

#include "moksha/client/app/app_config.h"
#include "moksha/shared/math/vec2.h"
#include "moksha/shared/math/vec2i.h"

namespace ME::Input {

enum class KeyState : uint8_t {
    None = 0x00,
    UpWasUp = 0x01,      // Key is currently up and was previously up. Default state.
    UpWasDown = 0x02,    // Key is currently up but was previously down. Key was released.
    DownWasDown = 0x04,  // Key is currently down and was previously down. Key is being held.
    DownWasUp = 0x08,    // Key is currently down but was previously up. Key was pressed.
};

enum class KeyCode : uint16_t {
    None = 0,

    // Control / misc
    Backspace = 1,
    Tab = 2,
    Enter = 3,
    Pause = 4,
    CapsLock = 5,
    Escape = 6,
    Space = 7,
    PageUp = 8,
    PageDown = 9,
    End = 10,
    Home = 11,
    LeftArrow = 12,
    UpArrow = 13,
    RightArrow = 14,
    DownArrow = 15,
    Insert = 16,
    Delete = 17,

    // Numbers (top row)
    Alpha0 = 18,
    Alpha1 = 19,
    Alpha2 = 20,
    Alpha3 = 21,
    Alpha4 = 22,
    Alpha5 = 23,
    Alpha6 = 24,
    Alpha7 = 25,
    Alpha8 = 26,
    Alpha9 = 27,

    // Letters
    A = 28,
    B = 29,
    C = 30,
    D = 31,
    E = 32,
    F = 33,
    G = 34,
    H = 35,
    I = 36,
    J = 37,
    K = 38,
    L = 39,
    M = 40,
    N = 41,
    O = 42,
    P = 43,
    Q = 44,
    R = 45,
    S = 46,
    T = 47,
    U = 48,
    V = 49,
    W = 50,
    X = 51,
    Y = 52,
    Z = 53,

    // Numpad
    NumPad0 = 54,
    NumPad1 = 55,
    NumPad2 = 56,
    NumPad3 = 57,
    NumPad4 = 58,
    NumPad5 = 59,
    NumPad6 = 60,
    NumPad7 = 61,
    NumPad8 = 62,
    NumPad9 = 63,
    Multiply = 64,
    Add = 65,
    Subtract = 66,
    Decimal = 67,
    Divide = 68,

    // Function keys
    F1 = 69,
    F2 = 70,
    F3 = 71,
    F4 = 72,
    F5 = 73,
    F6 = 74,
    F7 = 75,
    F8 = 76,
    F9 = 77,
    F10 = 78,
    F11 = 79,
    F12 = 80,

    // Locks / modifiers
    NumLock = 81,
    ScrollLock = 82,
    LeftShift = 83,
    RightShift = 84,
    LeftCtrl = 85,
    RightCtrl = 86,
    LeftAlt = 87,
    RightAlt = 88,
    LeftMeta = 89,  // Win / Cmd
    RightMeta = 90,

    // Punctuation / symbols
    Semicolon = 91,      // ;
    Equal = 92,          // =
    Comma = 93,          // ,
    Minus = 94,          // -
    Period = 95,         // .
    Slash = 96,          // /
    Grave = 97,          // `
    LeftBracket = 98,    // [
    Backslash = 99,      //
    RightBracket = 100,  // ]
    Apostrophe = 101,    // '

    // Reserved for future expansion
    MaxValue = 0xFFFF
};

enum class MouseButton : uint8_t {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
};

/**
 * Base class for platform specific Input implementation.
 */
class PlatformInputManager {
   public:
    PlatformInputManager();
    ~PlatformInputManager();
    virtual void Init();
    virtual void PreUpdate();
    virtual void Update(double deltaTime);
    virtual void PostUpdate();
    virtual void End();
};

class InputManagerMac;

/**
 * Input manager.
 */
class InputManager {
   public:
#ifdef ME_MAC
    friend class InputManagerMac;
#endif
#ifdef ME_WIN
    friend class InputManagerWin;
#endif

    InputManager();
    ~InputManager();

    // window is the app's window; the normalized and world-space mouse queries read its size.
    void Init(const ME::WindowState* inWindow);
    void PreUpdate();
    void Update(double deltaTime);
    void PostUpdate();
    void End();

    // Check if a key is down.
    static bool GetKeyDown(ME::Input::KeyCode keyCode);

    // Check if a key is up.
    static bool GetKeyUp(ME::Input::KeyCode keyCode);

    // Check if key was released this frame.
    static bool GetKeyReleased(ME::Input::KeyCode keyCode);

    // Check if key was pressed this frame.
    static bool GetKeyPressed(ME::Input::KeyCode keyCode);

    /**
     * Get mouse position, native to the platform: top-left is (0,0), Y increases downward.
     * Matches UI space (see ui_rect.h) directly — no conversion needed for UI hit-testing.
     */
    static ME::Vec2i GetMousePos() {
        return mousePos;
    }

    /**
     * Get normalized mouse position (0.0 to 1.0), same top-left/Y-down origin as GetMousePos().
     */
    ME::Vec2 GetMousePosNorm() const {
        if (window == nullptr || window->width == 0 || window->height == 0) {
            return ME::Vec2::Zero;
        }
        return ME::Vec2(static_cast<float>(mousePos.x) / window->width,
                        static_cast<float>(mousePos.y) / window->height);
    }

    /**
     * Mouse position in world space (bottom-left origin, Y-up) — the engine's general
     * world-space convention, for gameplay math (e.g. aim-direction) rather than UI hit-testing.
     */
    ME::Vec2i GetMouseWorldPos() const {
        const int height = window != nullptr ? window->height : 0;
        return ME::Vec2i{mousePos.x, height - mousePos.y};
    }

    // Normalized (0.0 to 1.0) version of GetMouseWorldPos().
    ME::Vec2 GetMouseWorldPosNorm() const {
        ME::Vec2 norm = GetMousePosNorm();
        return ME::Vec2(norm.x, 1.0f - norm.y);
    }

    // Check if a mouse button is down.
    static bool GetMouseButtonDown(MouseButton button);

    // Check if a mouse button is up.
    static bool GetMouseButtonUp(MouseButton button);

    // Check if a mouse button was released this frame.
    static bool GetMouseButtonReleased(MouseButton button);

    // Check if a mouse button was pressed this frame.
    static bool GetMouseButtonPressed(MouseButton button);

    PlatformInputManager* GetPlatformInputManager();

   private:
    static const size_t kMouseButtonCount = 5;

    // Shared down/up transition logic for both platform backends. Mouse messages carry no
    // repeat bits (unlike WM_KEYDOWN), so state is derived from the previous frame instead.
    static void SetMouseButtonState(MouseButton button, bool isDown);

    // Advance DownWasUp -> DownWasDown and UpWasDown -> UpWasUp for all mouse buttons, same as
    // each platform backend already does for GlobalKeyState in its own PostUpdate().
    static void AdvanceMouseButtonStates();

    // Global key state map.
    static std::unordered_map<ME::Input::KeyCode, ME::Input::KeyState> GlobalKeyState;

    // Mouse button state, indexed by MouseButton. Reuses KeyState since the same
    // down/up/held/released transitions apply.
    static ME::Input::KeyState GlobalMouseButtonState[kMouseButtonCount];

    // Mouse position.
    inline static ME::Vec2i mousePos = ME::Vec2i::Zero;

    // Platform specific input manager.
    PlatformInputManager* platformInputManager;

    // The app's window, for the size-relative mouse queries.
    const ME::WindowState* window = nullptr;
};

}  // namespace ME::Input
