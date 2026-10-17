#pragma once
#ifdef ME_WIN
/**
 * Input manager for Windows
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include "input_manager.h"

namespace ME::Input {

/**
 * Represents a key event from Windows.
 * This is used to buffer input events for processing.
 */
class WinKeyEvent {
   public:
    WinKeyEvent() {}
    WinKeyEvent(UINT msg, WPARAM wParam, LPARAM lParam);
    UINT msg = 0;
    WPARAM wParam = 0;
    LPARAM lParam = 0;
};

/**
 * Buffer to store Windows key events.
 */
class WinKeyEventBuffer {
   public:
    WinKeyEventBuffer();

    ~WinKeyEventBuffer();

    // Add elements to the buffer.
    void Push(UINT msg, WPARAM wParam, LPARAM lParam);

    // Reset the buffer for reuse after swapping read and write buffers. This just resets the count.
    void Reset();

    WinKeyEvent* buffer;
    size_t count = 0;

   private:
    static const size_t MAX_KEY_EVENTS = 256;
};

class InputManagerWin : public ME::Input::PlatformInputManager {
   public:
    InputManagerWin();
    ~InputManagerWin();
    void Init() override;
    void PreUpdate() override;
    void Update(double deltaTime) override;
    void PostUpdate() override;
    void End() override;

    // Handle Windows-specific input messages.
    void HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);

   private:
    // Creating double buffers for key events to avoid race conditions and simultaneous read/write.
    WinKeyEventBuffer* bufferFirst = nullptr;
    WinKeyEventBuffer* bufferSecond = nullptr;
    WinKeyEventBuffer* readBuffer = nullptr;
    WinKeyEventBuffer* writeBuffer = nullptr;
    bool bReadingFirstBuffer = true;
};

}  // namespace ME::Input

#endif  // ME_WIN
