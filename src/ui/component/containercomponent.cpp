#include <latren/ui/component/containercomponent.h>

using namespace UI;

void ContainerComponent::Render(const glm::mat4& proj) {
    Canvas::Draw();
}

void ContainerComponent::Update() {
    for (auto& [p, layer] : components_) {
        layer.ForEachOwned([](UI::UIComponent& c) {
            c.Update();
        });
    }
}

void ContainerComponent::UpdateWindowSize() {
    UpdateComponentsOnWindowSize(aspectRatioModifier_);
}

glm::vec2 ContainerComponent::GetOffset() const {
    return GetTransform().pos + offset;
}

Rect ContainerComponent::GetLocalBounds() const {
    glm::vec2 pos = GetTransform().pos + offset;
    Rect bounds;
    bounds.left = pos.x;
    bounds.right = pos.x + bgSize.x;
    bounds.bottom = pos.y - bgSize.y;
    bounds.top = pos.y;
    return bounds;
}