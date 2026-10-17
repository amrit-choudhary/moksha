#pragma once

/**
 * SceneUI for DebugSystem. Loads the UI atlas + font textures at the fixed indices the DX12
 * renderer hardcodes for UI drawing (spriteTextures[0]/textureAtlasProperties[0] for sprites,
 * [1]/[1] for text — see renderer_dx.cpp's UI Sprite/Text Drawing sections), same convention as
 * SceneUIDemo. Builds a single fixed-capacity vertical stack of Labels (no per-frame
 * allocation): a static "DEBUG LOGS" header, then Constants::MaxDebugSlotCount persistent slot
 * rows, then Constants::MaxDebugLogLineCount transient log-line rows.
 *
 * This is the sole data holder for the debug system (matching Scene/SceneUI's existing role
 * elsewhere) — DebugSystem itself stays stateless, only operating on the data owned here: the
 * Label pool below, plus the transient log lines' text/color/remaining-lifetime bookkeeping.
 */

#include <cstdint>

#include "moksha/client/rendering/shared/color.h"
#include "moksha/client/scene/scene_ui.h"
#include "moksha/client/ui/label.h"
#include "moksha/shared/misc/engine_limits.h"

namespace ME {

class SceneUIDebug : public ME::SceneUI {
   public:
    SceneUIDebug();
    virtual ~SceneUIDebug() override;

    virtual void CreateResources() override;
    virtual void BuildUIElements() override;

    // Row 0 is the static header; slots start at row 1, log lines start right after the slots.
    ME::Label* GetSlotLabel(uint8_t slotIndex) const;
    ME::Label* GetLogLineLabel(uint8_t logLineIndex) const;

    static constexpr size_t kLogLineTextCapacity = 128;

    // Transient log line bookkeeping, indices [0, activeLogLineCount). Labels themselves keep a
    // fixed row/offset forever (see slotLabels/logLineLabels below); compaction/eviction moves
    // the *content* between labels via these buffers, not the labels' layout. Public, like
    // SceneUI's own sprite/text arrays — DebugSystem operates on this data directly.
    char logLineText[Constants::MaxDebugLogLineCount][kLogLineTextCapacity] = {};
    ME::Color logLineColor[Constants::MaxDebugLogLineCount] = {};
    float logLineRemaining[Constants::MaxDebugLogLineCount] = {};
    size_t activeLogLineCount = 0;

   private:
    ME::Label* headerLabel = nullptr;
    ME::Label* slotLabels[Constants::MaxDebugSlotCount] = {};
    ME::Label* logLineLabels[Constants::MaxDebugLogLineCount] = {};
};

}  // namespace ME
