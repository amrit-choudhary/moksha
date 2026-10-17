#pragma once

/**
 * On-screen debug system: draws on top of everything via its own SceneUI + UISystemDebug,
 * completely separate from whatever SceneUI/UISystem a Game owns. Owned by ClientApp (see
 * app/client_app.h), not by Game. Two kinds of on-screen text:
 *  - Slots: persistent, index-addressed rows (ScreenPrintSlot/ClearSlot/ClearAllSlot).
 *  - Transient log lines: time-limited rows (ScreenPrint/ClearAllTransient), FIFO-evicted
 *    (oldest first) when the pool is full.
 */

#include <cstdint>

#include "moksha/client/rendering/shared/color.h"
#include "scene_ui_debug.h"
#include "ui_system_debug.h"

namespace ME {

class WindowState;

class DebugSystem {
   public:
    DebugSystem();
    ~DebugSystem();

    // window is the app's window; the overlay is laid out against it.
    void Init(const WindowState* window);
    void Update(double deltaTime);
    void End();

    ME::SceneUI* GetUIScene();

    // Public API.
    static void SetInstance(DebugSystem* debugSystemInstance);
    static void ScreenPrintSlot(uint8_t slotIndex, const char* message, const Color& color = ME::Color::White());
    static void ClearSlot(uint8_t slotIndex);
    static void ClearAllSlot();
    static void ScreenPrint(const char* message, float lifetime, const Color& color = ME::Color::White());
    static void ClearAllTransient();
    static void ClearAll();

   private:
    // Keeps it's own copy of scene and ui system because it has to run independently of the game's scene and ui system.
    // The game may not even have a scene/ui system at all, but the debug system should still work.
    ME::SceneUIDebug sceneUIDebug;
    ME::UISystemDebug uiSystem;

    // Per-instance implementation, called by Update() and by the static forwarders above.
    void ScreenPrintSlotImpl(uint8_t slotIndex, const char* message, const Color& color);
    void ClearSlotImpl(uint8_t slotIndex);
    void ClearAllSlotImpl();
    void ScreenPrintImpl(const char* message, float lifetime, const Color& color);
    void ClearAllTransientImpl();
    void ClearAllImpl();
    void SyncLogLineLabels();

    static DebugSystem* instance;
};

}  // namespace ME
