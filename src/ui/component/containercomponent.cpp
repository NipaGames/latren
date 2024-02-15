#include <latren/ui/component/containercomponent.h>

using namespace UI;

void ContainerComponent::ClearChildren() {
    children_.clear();
}

void ContainerComponent::DeleteChildAtIndex(size_t i) {
    children_.erase(children_.begin() + i);
}

ContainerComponent::~ContainerComponent() {
    ClearChildren();
}

void ContainerComponent::Render(const glm::mat4& proj) {
    Canvas::Draw();
}

void ContainerComponent::Update() {
    for (auto& [p, components] : components_) {
        for (ComponentReference<UI::UIComponent>& c : components) {
            c->Update();
        }
    }
}

void ContainerComponent::UpdateWindowSize() {
    UpdateComponentsOnWindowSize(aspectRatioModifier_);
}

glm::vec2 ContainerComponent::GetOffset() const {
    return GetTransform().pos + offset;
}

size_t ContainerComponent::GetChildCount() {
    return children_.size();
}

void ContainerComponent::AddChild(const std::shared_ptr<UIComponent>& c) {
    children_.push_back(c);
    c->Start();
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