#include "scene_ui_debug.h"

#include "moksha/client/utils/json_utils.h"
#include "moksha/shared/math/vec2.h"

namespace {
constexpr float kRowStartX = 20.0f;
constexpr float kRowStartY = 10.0f;
constexpr float kRowHeight = 18.0f;
constexpr uint16_t kFontHeight = 16;
constexpr uint16_t kFontWidth = 16;
constexpr int16_t kLetterSpacing = 0;
constexpr int16_t kLineGap = 2;
constexpr uint16_t kCharsPerLine = 120;
}  // namespace

ME::SceneUIDebug::SceneUIDebug() {}

ME::SceneUIDebug::~SceneUIDebug() {
    delete headerLabel;
    for (size_t i = 0; i < ME::Constants::MaxDebugSlotCount; ++i) {
        delete slotLabels[i];
    }
    for (size_t i = 0; i < ME::Constants::MaxDebugLogLineCount; ++i) {
        delete logLineLabels[i];
    }
}

void ME::SceneUIDebug::CreateResources() {
    SceneUI::CreateResources();

    spriteTexturePaths[0] = "textures/ui/ui_atlas.dds";
    spriteTexturePaths[1] = "textures/font/ascii_ibm_transparent.dds";
    spriteTexturePaths.count = 2;

    ME::JsonUtils::LoadTextureAtlasProps("texture_data/atlas_ui.json", textureAtlasProperties[0]);
    ME::JsonUtils::LoadTextureAtlasProps("texture_data/font_atlas_01.json", textureAtlasProperties[1]);
    textureAtlasProperties.count = 2;
}

void ME::SceneUIDebug::BuildUIElements() {
    SceneUI::BuildUIElements();

    uint32_t row = 0;

    headerLabel = new ME::Label("DEBUG LOGS", 0, 1, 0, ME::Color::White(), kFontHeight, kFontWidth, kLetterSpacing,
                                kLineGap, kCharsPerLine, ME::TextAlignment::Left);
    headerLabel->SetAnchor(ME::UIAnchor::TopLeft);
    headerLabel->SetPivot(ME::UIPivot::TopLeft);
    headerLabel->SetOffset(ME::Vec2{kRowStartX, kRowStartY + static_cast<float>(row) * kRowHeight});
    headerLabel->SetSize(ME::Vec2{0.0f, kRowHeight});
    headerLabel->SetColor(ME::Color::Yellow());
    headerLabel->Init();
    AddUIElement(headerLabel);
    ++row;

    for (size_t i = 0; i < ME::Constants::MaxDebugSlotCount; ++i) {
        ME::Label* slotLabel = new ME::Label("", 0, 1, 0, ME::Color::White(), kFontHeight, kFontWidth, kLetterSpacing,
                                             kLineGap, kCharsPerLine, ME::TextAlignment::Left);
        slotLabel->SetAnchor(ME::UIAnchor::TopLeft);
        slotLabel->SetPivot(ME::UIPivot::TopLeft);
        slotLabel->SetOffset(ME::Vec2{kRowStartX, kRowStartY + static_cast<float>(row) * kRowHeight});
        slotLabel->SetSize(ME::Vec2{0.0f, kRowHeight});
        slotLabel->SetVisible(false);
        slotLabel->Init();
        AddUIElement(slotLabel);
        slotLabels[i] = slotLabel;
        ++row;
    }

    for (size_t i = 0; i < ME::Constants::MaxDebugLogLineCount; ++i) {
        ME::Label* logLineLabel = new ME::Label("", 0, 1, 0, ME::Color::White(), kFontHeight, kFontWidth,
                                                kLetterSpacing, kLineGap, kCharsPerLine, ME::TextAlignment::Left);
        logLineLabel->SetAnchor(ME::UIAnchor::TopLeft);
        logLineLabel->SetPivot(ME::UIPivot::TopLeft);
        logLineLabel->SetOffset(ME::Vec2{kRowStartX, kRowStartY + static_cast<float>(row) * kRowHeight});
        logLineLabel->SetSize(ME::Vec2{0.0f, kRowHeight});
        logLineLabel->SetVisible(false);
        logLineLabel->Init();
        AddUIElement(logLineLabel);
        logLineLabels[i] = logLineLabel;
        ++row;
    }
}

ME::Label* ME::SceneUIDebug::GetSlotLabel(uint8_t slotIndex) const {
    if (slotIndex >= ME::Constants::MaxDebugSlotCount) {
        return nullptr;
    }
    return slotLabels[slotIndex];
}

ME::Label* ME::SceneUIDebug::GetLogLineLabel(uint8_t logLineIndex) const {
    if (logLineIndex >= ME::Constants::MaxDebugLogLineCount) {
        return nullptr;
    }
    return logLineLabels[logLineIndex];
}
