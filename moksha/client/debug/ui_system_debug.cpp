#include "ui_system_debug.h"

#include "moksha/client/app/app_config.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/client/rendering/shared/text_renderer.h"
#include "moksha/client/scene/scene_ui.h"
#include "moksha/client/ui/ui_element.h"
#include "moksha/shared/math/transform.h"
#include "moksha/shared/misc/engine_limits.h"

ME::UISystemDebug::UISystemDebug() {}

ME::UISystemDebug::~UISystemDebug() {}

void ME::UISystemDebug::Init(const WindowState* inWindow) {
    window = inWindow;
    layoutEngine.Init(window);

    spriteTransformScratch = new Transform*[Constants::MaxUIElementCount];
    spriteRendererScratch = new SpriteRenderer*[Constants::MaxUIElementCount];
    textTransformScratch = new Transform*[Constants::MaxUIElementCount];
    textRendererScratch = new TextRenderer*[Constants::MaxUIElementCount];
}

void ME::UISystemDebug::Update(double deltaTime) {
    if (uiScene == nullptr) {
        return;
    }

    layoutEngine.RecalculateLayouts(uiScene->GetUIElements(), uiScene->GetUIElementCount());
    SyncToScene();
}

void ME::UISystemDebug::End() {
    layoutEngine.End();

    delete[] spriteTransformScratch;
    delete[] spriteRendererScratch;
    delete[] textTransformScratch;
    delete[] textRendererScratch;
}

void ME::UISystemDebug::SetUIScene(SceneUI* newUiScene) {
    uiScene = newUiScene;
}

namespace {

bool IsEffectivelyVisible(ME::UIElement* element) {
    while (element != nullptr) {
        if (!element->IsVisible()) {
            return false;
        }
        element = element->GetParent();
    }
    return true;
}

}  // namespace

void ME::UISystemDebug::SyncToScene() {
    uint32_t spriteCount = 0;
    uint32_t textCount = 0;

    float windowWidth = window != nullptr ? static_cast<float>(window->width) : 0.0f;
    float windowHeight = window != nullptr ? static_cast<float>(window->height) : 0.0f;

    UIElement** elements = uiScene->GetUIElements();
    uint32_t elementCount = uiScene->GetUIElementCount();

    for (uint32_t i = 0; i < elementCount; ++i) {
        UIElement* element = elements[i];
        if (!IsEffectivelyVisible(element)) {
            continue;
        }

        Vec2 absolutePosition = element->GetRect()->GetPosition();
        Vec2 absoluteSize = element->GetRect()->GetSize();

        float centerX = (absolutePosition.x + absoluteSize.x / 2.0f) - windowWidth / 2.0f;
        float centerY = windowHeight / 2.0f - (absolutePosition.y + absoluteSize.y / 2.0f);

        SpriteRenderer* spriteRenderer = element->GetSpriteRenderer();
        if (spriteRenderer != nullptr) {
            Transform* transform = element->GetTransform();
            transform->SetPosition(centerX, centerY, 0.0f);
            transform->SetScale(absoluteSize.x, absoluteSize.y, 1.0f);
            spriteRenderer->bDirty = true;

            spriteTransformScratch[spriteCount] = transform;
            spriteRendererScratch[spriteCount] = spriteRenderer;
            ++spriteCount;
        }

        TextRenderer* textRenderer = element->GetTextRenderer();
        if (textRenderer != nullptr) {
            Transform* transform = element->GetTransform();
            transform->SetPosition(centerX, centerY, 0.0f);
            transform->SetScale(absoluteSize.x, absoluteSize.y, 1.0f);
            textRenderer->bDirty = true;

            textTransformScratch[textCount] = transform;
            textRendererScratch[textCount] = textRenderer;
            ++textCount;
        }
    }

    uiScene->RebuildUISprites(spriteTransformScratch, spriteRendererScratch, spriteCount);
    uiScene->RebuildUIText(textTransformScratch, textRendererScratch, textCount);
}
