#include <latren/ui/component/containercomponent.h>

using namespace UI;

void ContainerComponent::Render(const glm::mat4&) {
    Canvas::Draw();
}

void ContainerComponent::UIUpdate() {
    Canvas::Update();
}

void ContainerComponent::UpdateWindowSize() {
    UpdateComponentsOnWindowSize(aspectRatioModifier_);
}

glm::vec2 ContainerComponent::GetOffset() const {
    return parent_->GetOffset() + GetTransform().pos + offset;
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