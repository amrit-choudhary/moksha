#include "ui_system.h"

#include "button.h"
#include "moksha/client/input/input_manager.h"
#include "moksha/client/misc/global_vars.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/client/rendering/shared/text_renderer.h"
#include "moksha/client/scene/scene_ui.h"
#include "moksha/shared/math/transform.h"
#include "moksha/shared/misc/engine_limits.h"
#include "ui_element.h"

ME::UISystem::UISystem() {}

ME::UISystem::~UISystem() {}

void ME::UISystem::Init() {
    layoutEngine.Init();

    spriteTransformScratch = new Transform*[Constants::MaxUIElementCount];
    spriteRendererScratch = new SpriteRenderer*[Constants::MaxUIElementCount];
    textTransformScratch = new Transform*[Constants::MaxUIElementCount];
    textRendererScratch = new TextRenderer*[Constants::MaxUIElementCount];
}

void ME::UISystem::Update(double deltaTime) {
    if (uiScene == nullptr) {
        return;
    }

    layoutEngine.RecalculateLayouts(uiScene->GetUIElements(), uiScene->GetUIElementCount());
    UpdateButtonInteractions();
    SyncToScene();
}

void ME::UISystem::End() {
    layoutEngine.End();

    delete[] spriteTransformScratch;
    delete[] spriteRendererScratch;
    delete[] textTransformScratch;
    delete[] textRendererScratch;
}

void ME::UISystem::SetUIScene(SceneUI* newUiScene) {
    uiScene = newUiScene;
}

bool ME::UISystem::AddElement(UIElement* element) {
    if (uiScene == nullptr) {
        return false;
    }
    element->Init();
    return uiScene->AddUIElement(element);
}

void ME::UISystem::RemoveElement(UIElement* element) {
    if (uiScene == nullptr) {
        return;
    }
    uiScene->RemoveUIElement(element);
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

void ME::UISystem::UpdateButtonInteractions() {
    ME::Vec2i rawMousePos = ME::Input::InputManager::GetMousePos();
    Vec2 mousePos{static_cast<float>(rawMousePos.x), static_cast<float>(rawMousePos.y)};
    bool mouseDown = ME::Input::InputManager::GetMouseButtonPressed(ME::Input::MouseButton::Left);

    UIElement** elements = uiScene->GetUIElements();
    uint32_t elementCount = uiScene->GetUIElementCount();
    for (uint32_t i = 0; i < elementCount; ++i) {
        UIElement* element = elements[i];
        if (element->GetType() != UIElementType::Button) {
            continue;
        }
        Button* button = static_cast<Button*>(element);
        // Hidden buttons (or ones under a hidden parent) take no input.
        if (IsEffectivelyVisible(button)) {
            button->UpdateInteraction(mousePos, mouseDown);
        } else {
            button->ResetInteraction();
        }
    }
}

void ME::UISystem::SyncToScene() {
    uint32_t spriteCount = 0;
    uint32_t textCount = 0;

    float windowWidth = static_cast<float>(GlobalVars::GetWindowWidth());
    float windowHeight = static_cast<float>(GlobalVars::GetWindowHeight());

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
