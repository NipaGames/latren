#include <latren/ui/component/containercomponent.h>

using namespace UI;

void ContainerComponent::Render(const glm::mat4& proj) {
    Canvas::Draw();
    UIComponent::Render(proj);
}

void ContainerComponent::UIUpdate() {
    Canvas::Update();
    UIComponent::UIUpdate();
}

void ContainerComponent::UpdateWindowSize() {
    UpdateComponentsOnWindowSize(aspectRatioModifier_);
}

glm::vec2 ContainerComponent::GetOffset() const {
    return parent_->GetOffset() + GetTransform().pos + offset_;
}

Rect ContainerComponent::GetLocalBounds() const {
    glm::vec2 pos = GetTransform().pos + offset_;
    Rect bounds;
    bounds.left = pos.x;
    bounds.right = pos.x + GetBackgroundSize().x;
    bounds.bottom = pos.y - GetBackgroundSize().y;
    bounds.top = pos.y;
    return bounds;
}