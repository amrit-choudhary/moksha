#include "debug_system.h"

#include "moksha/client/utils/string_utils.h"

ME::DebugSystem* ME::DebugSystem::instance = nullptr;

ME::DebugSystem::DebugSystem() {}

ME::DebugSystem::~DebugSystem() {}

void ME::DebugSystem::Init() {
    sceneUIDebug.Init();
    uiSystem.Init();
    uiSystem.SetUIScene(&sceneUIDebug);
}

void ME::DebugSystem::Update(double deltaTime) {
    // Tick down active transient log lines, compacting expired ones out so the remaining lines
    // shift up with no gaps.
    size_t writeIndex = 0;
    for (size_t readIndex = 0; readIndex < sceneUIDebug.activeLogLineCount; ++readIndex) {
        sceneUIDebug.logLineRemaining[readIndex] -= static_cast<float>(deltaTime);
        if (sceneUIDebug.logLineRemaining[readIndex] > 0.0f) {
            if (writeIndex != readIndex) {
                ME::StringUtils::CopyBounded(sceneUIDebug.logLineText[writeIndex],
                                             ME::SceneUIDebug::kLogLineTextCapacity,
                                             sceneUIDebug.logLineText[readIndex]);
                sceneUIDebug.logLineColor[writeIndex] = sceneUIDebug.logLineColor[readIndex];
                sceneUIDebug.logLineRemaining[writeIndex] = sceneUIDebug.logLineRemaining[readIndex];
            }
            ++writeIndex;
        }
    }
    sceneUIDebug.activeLogLineCount = writeIndex;
    SyncLogLineLabels();

    uiSystem.Update(deltaTime);
}

void ME::DebugSystem::End() {
    uiSystem.End();
}

ME::SceneUI* ME::DebugSystem::GetUIScene() {
    return &sceneUIDebug;
}

void ME::DebugSystem::SetInstance(DebugSystem* debugSystemInstance) {
    instance = debugSystemInstance;
}

void ME::DebugSystem::ScreenPrintSlot(uint8_t slotIndex, const char* message, const Color& color) {
    if (instance != nullptr) {
        instance->ScreenPrintSlotImpl(slotIndex, message, color);
    }
}

void ME::DebugSystem::ClearSlot(uint8_t slotIndex) {
    if (instance != nullptr) {
        instance->ClearSlotImpl(slotIndex);
    }
}

void ME::DebugSystem::ClearAllSlot() {
    if (instance != nullptr) {
        instance->ClearAllSlotImpl();
    }
}

void ME::DebugSystem::ScreenPrint(const char* message, float lifetime, const Color& color) {
    if (instance != nullptr) {
        instance->ScreenPrintImpl(message, lifetime, color);
    }
}

void ME::DebugSystem::ClearAllTransient() {
    if (instance != nullptr) {
        instance->ClearAllTransientImpl();
    }
}

void ME::DebugSystem::ClearAll() {
    if (instance != nullptr) {
        instance->ClearAllImpl();
    }
}

void ME::DebugSystem::ScreenPrintSlotImpl(uint8_t slotIndex, const char* message, const Color& color) {
    ME::Label* label = sceneUIDebug.GetSlotLabel(slotIndex);
    if (label == nullptr) {
        return;
    }
    label->SetText(message);
    label->SetColor(color);
    label->SetVisible(true);
}

void ME::DebugSystem::ClearSlotImpl(uint8_t slotIndex) {
    ME::Label* label = sceneUIDebug.GetSlotLabel(slotIndex);
    if (label == nullptr) {
        return;
    }
    label->SetVisible(false);
}

void ME::DebugSystem::ClearAllSlotImpl() {
    for (size_t i = 0; i < ME::Constants::MaxDebugSlotCount; ++i) {
        ClearSlotImpl(static_cast<uint8_t>(i));
    }
}

void ME::DebugSystem::ScreenPrintImpl(const char* message, float lifetime, const Color& color) {
    size_t index;
    if (sceneUIDebug.activeLogLineCount >= ME::Constants::MaxDebugLogLineCount) {
        // Pool full — evict the oldest (index 0), shifting everyone else down to make room at
        // the end. Deterministic FIFO eviction, never drops the newest print.
        for (size_t i = 1; i < ME::Constants::MaxDebugLogLineCount; ++i) {
            ME::StringUtils::CopyBounded(sceneUIDebug.logLineText[i - 1], ME::SceneUIDebug::kLogLineTextCapacity,
                                         sceneUIDebug.logLineText[i]);
            sceneUIDebug.logLineColor[i - 1] = sceneUIDebug.logLineColor[i];
            sceneUIDebug.logLineRemaining[i - 1] = sceneUIDebug.logLineRemaining[i];
        }
        index = ME::Constants::MaxDebugLogLineCount - 1;
    } else {
        index = sceneUIDebug.activeLogLineCount;
        ++sceneUIDebug.activeLogLineCount;
    }

    ME::StringUtils::CopyBounded(sceneUIDebug.logLineText[index], ME::SceneUIDebug::kLogLineTextCapacity, message);
    sceneUIDebug.logLineColor[index] = color;
    sceneUIDebug.logLineRemaining[index] = lifetime;

    SyncLogLineLabels();
}

void ME::DebugSystem::ClearAllTransientImpl() {
    sceneUIDebug.activeLogLineCount = 0;
    SyncLogLineLabels();
}

void ME::DebugSystem::ClearAllImpl() {
    ClearAllSlotImpl();
    ClearAllTransientImpl();
}

void ME::DebugSystem::SyncLogLineLabels() {
    for (size_t i = 0; i < ME::Constants::MaxDebugLogLineCount; ++i) {
        ME::Label* label = sceneUIDebug.GetLogLineLabel(static_cast<uint8_t>(i));
        if (label == nullptr) {
            continue;
        }
        if (i < sceneUIDebug.activeLogLineCount) {
            label->SetText(sceneUIDebug.logLineText[i]);
            label->SetColor(sceneUIDebug.logLineColor[i]);
            label->SetVisible(true);
        } else {
            label->SetVisible(false);
        }
    }
}
