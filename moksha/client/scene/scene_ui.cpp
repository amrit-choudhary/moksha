#include "scene_ui.h"

#include <cstddef>

#include "moksha/client/utils/json_utils.h"
#include "moksha/shared/misc/engine_limits.h"

ME::SceneUI::SceneUI() {}

ME::SceneUI::~SceneUI() {
    delete[] spriteTexturePaths.data;
    delete[] textureAtlasProperties.data;

    // uiSpriteTransforms/uiSpriteRenderers/textTransforms/textRenderers all hold values copied
    // in via AddUISprite/AddUIText/RebuildUISprites/RebuildUIText (from the dynamic UIElement
    // path, Panel/Label/Image own the source Transform/SpriteRenderer/TextRenderer) — SceneUI
    // owns these copies, so just freeing the arrays is enough. UIElements themselves are owned
    // by whoever created them — only the array container is freed for those.
    delete[] uiSpriteTransforms.data;
    delete[] uiSpriteRenderers.data;
    delete[] uiSpriteInstanceData.data;

    delete[] textTransforms.data;
    delete[] textRenderers.data;
    delete[] textInstanceData.data;

    delete[] uiElements.data;
}

void ME::SceneUI::Update() {
    UpdateUISpriteRenderers();
    UpdateTextRenderers();
}

void ME::SceneUI::Init() {
    CreateResources();
    BuildUISprites();
    BuildTextRenderers();
    BuildUIElements();
}

void ME::SceneUI::CreateResources() {
    spriteTexturePaths.data = new const char*[Constants::MaxSpriteTextureCount];
    spriteTexturePaths.count = 0;
    textureAtlasProperties.data = new ME::TextureAtlasProperties[Constants::MaxTextureAtlasPropertiesCount];
    textureAtlasProperties.count = 0;

    uiSpriteTransforms.data = new ME::Transform[Constants::MaxUISpriteTransformCount];
    uiSpriteTransforms.count = 0;
    uiSpriteRenderers.data = new ME::SpriteRenderer[Constants::MaxUISpriteRendererCount];
    uiSpriteRenderers.count = 0;
    uiSpriteInstanceData.data = new ME::UISpriteRendererInstanceData[Constants::MaxUISpriteInstanceDataCount];
    uiSpriteInstanceData.count = 0;
    textTransforms.data = new ME::Transform[Constants::MaxTextTransformsCount];
    textTransforms.count = 0;
    textRenderers.data = new ME::TextRenderer[Constants::MaxTextRendererCount];
    textRenderers.count = 0;
    textInstanceData.data = new ME::TextRendererInstanceData[Constants::MaxTextInstanceDataCount];
    textInstanceData.count = 0;

    uiElements.data = new ME::UIElement*[Constants::MaxUIElementCount];
    uiElements.count = 0;
}

void ME::SceneUI::BuildUISprites() {}

void ME::SceneUI::BuildTextRenderers() {}

void ME::SceneUI::BuildUIElements() {}

void ME::SceneUI::AddUISprite(ME::Vec3 position, ME::Vec3 scale, const ME::SpriteRenderer& spriteRenderer) {
    uiSpriteTransforms[uiSpriteTransforms.count].SetPosition(position);
    uiSpriteTransforms[uiSpriteTransforms.count].SetScale(scale);
    ++uiSpriteTransforms.count;

    uiSpriteRenderers[uiSpriteRenderers.count] = spriteRenderer;
    ++uiSpriteRenderers.count;
}

void ME::SceneUI::AddUIText(ME::Vec3 position, ME::Vec3 scale, const ME::TextRenderer& textRenderer) {
    textTransforms[textTransforms.count].SetPosition(position);
    textTransforms[textTransforms.count].SetScale(scale);
    ++textTransforms.count;
    textRenderers[textRenderers.count] = textRenderer;
    ++textRenderers.count;
    textInstanceData.count += textRenderer.GetCount();
}

bool ME::SceneUI::AddUIElement(ME::UIElement* element) {
    if (uiElements.count >= Constants::MaxUIElementCount) {
        return false;
    }
    uiElements[uiElements.count] = element;
    ++uiElements.count;
    return true;
}

void ME::SceneUI::RemoveUIElement(ME::UIElement* element) {
    for (uint32_t i = 0; i < uiElements.count; ++i) {
        if (uiElements[i] == element) {
            uiElements[i] = uiElements[uiElements.count - 1];
            --uiElements.count;
            return;
        }
    }
}

ME::UIElement** ME::SceneUI::GetUIElements() const {
    return uiElements.data;
}

uint32_t ME::SceneUI::GetUIElementCount() const {
    return static_cast<uint32_t>(uiElements.count);
}

void ME::SceneUI::RebuildUISprites(ME::Transform** transforms, ME::SpriteRenderer** renderers, uint32_t count) {
    uint32_t boundedCount = count < Constants::MaxUISpriteRendererCount ? count : Constants::MaxUISpriteRendererCount;
    for (uint32_t i = 0; i < boundedCount; ++i) {
        uiSpriteTransforms[i] = *transforms[i];
        uiSpriteRenderers[i] = *renderers[i];
    }
    uiSpriteTransforms.count = boundedCount;
    uiSpriteRenderers.count = boundedCount;
    // One instance per sprite renderer (unlike text, which can have multiple glyph instances
    // per TextRenderer) — UpdateUISpriteRenderers() doesn't recompute this itself, unlike
    // UpdateTextRenderers()'s textInstanceData.count, so it must be set here.
    uiSpriteInstanceData.count = boundedCount;
}

void ME::SceneUI::RebuildUIText(ME::Transform** transforms, ME::TextRenderer** renderers, uint32_t count) {
    uint32_t boundedCount = count < Constants::MaxTextRendererCount ? count : Constants::MaxTextRendererCount;
    for (uint32_t i = 0; i < boundedCount; ++i) {
        textTransforms[i] = *transforms[i];
        textRenderers[i] = *renderers[i];
    }
    textTransforms.count = boundedCount;
    textRenderers.count = boundedCount;
}

void ME::SceneUI::UpdateUISpriteRenderers() {
    // Updating transforms and atlas indicesfor dirty instances.
    // In separate loops to avoid cache misses.

    for (uint32_t i = 0; i < uiSpriteRenderers.count; ++i) {
        if (!uiSpriteRenderers[i].bDirty) {
            continue;
        }
        uiSpriteInstanceData[i].modelMatrixData = uiSpriteTransforms[i].GetModelMatrix().GetDataForShader();
    }

    for (uint32_t i = 0; i < uiSpriteRenderers.count; ++i) {
        if (!uiSpriteRenderers[i].bDirty) {
            continue;
        }
        uiSpriteInstanceData[i].atlasIndex = uiSpriteRenderers[i].atlasIndex;
        uiSpriteInstanceData[i].color = uiSpriteRenderers[i].color;
        uiSpriteInstanceData[i].flags = uiSpriteRenderers[i].flags;
    }

    for (uint32_t i = 0; i < uiSpriteRenderers.count; ++i) {
        if (uiSpriteRenderers[i].bDirty) {
            uiSpriteRenderers[i].bDirty = false;
        }
    }
}

void ME::SceneUI::UpdateTextRenderers() {
    // Update transform data.
    uint32_t count = 0;
    for (uint32_t i = 0; i < textRenderers.count; ++i) {
        if (!textRenderers[i].bDirty) {
            count += textRenderers[i].GetCount();
            continue;
        }

        float totalWidth = static_cast<float>(textRenderers[i].GetRenderWidth());
        float glyphStep = static_cast<float>(textRenderers[i].width + textRenderers[i].letterSpacing);

        // textTransforms[i]'s position is always the owning box's center (see UISystem/
        // UISystemDebug's SyncToScene), and its scale.x is the box's width (same call sets
        // both) — startOffset picks where the run begins relative to that center, per
        // alignment. Left/Right are expressed in terms of boxWidth rather than totalWidth
        // specifically so they land exactly on the box edge regardless of any mismatch between
        // the box's declared size and the text's actual rendered width.
        float boxWidth = textTransforms[i].GetScale().x;
        float startOffset;
        switch (textRenderers[i].alignment) {
            case ME::TextAlignment::Left:
                startOffset = -boxWidth / 2.0f;
                break;
            case ME::TextAlignment::Right:
                startOffset = boxWidth / 2.0f - totalWidth;
                break;
            case ME::TextAlignment::Center:
            default:
                startOffset = -totalWidth / 2.0f;
                break;
        }

        for (int j = 0; j < textRenderers[i].GetCount(); ++j) {
            ME::Transform tempTransform;
            ME::Vec3 pos = textTransforms[i].GetPosition();
            pos.x += startOffset + j * glyphStep + textRenderers[i].width / 2.0f;
            tempTransform.SetPosition(pos);
            tempTransform.SetScale(textRenderers[i].width, textRenderers[i].height);
            textInstanceData[count].modelMatrixData = tempTransform.GetModelMatrix().GetDataForShader();
            ++count;
        }
    }

    // Update atlas indices and colors.
    count = 0;
    for (uint32_t i = 0; i < textRenderers.count; ++i) {
        if (!textRenderers[i].bDirty) {
            count += textRenderers[i].GetCount();
            continue;
        }

        for (int j = 0; j < textRenderers[i].GetCount(); ++j) {
            textInstanceData[count].atlasIndex = textRenderers[i].text[j];
            textInstanceData[count].color = textRenderers[i].color;
            ++count;
        }
    }

    // TODO: change only if bDirty.
    textInstanceData.count = 0;
    for (uint32_t i = 0; i < textRenderers.count; ++i) {
        textInstanceData.count += textRenderers[i].GetCount();
    }

    for (uint32_t i = 0; i < textRenderers.count; ++i) {
        if (textRenderers[i].bDirty) {
            textRenderers[i].bDirty = false;
        }
    }
}
