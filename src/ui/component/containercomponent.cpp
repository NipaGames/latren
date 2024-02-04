#include <latren/ui/component/listcomponent.h>

using namespace UI;

void ContainerComponent::ClearChildren() {
    for (UIComponent* c : children_) {
        delete c;
    }
    children_.clear();
}

void ContainerComponent::DeleteChildAtIndex(size_t i) {
    delete children_.at(i);
    children_.erase(children_.begin() + i);
}

ContainerComponent::~ContainerComponent() {
    ClearChildren();
}

void ContainerComponent::Render(const glm::mat4& proj) {
    Canvas::Draw();
}

void ContainerComponent::Update() {
    for (const auto& [p, components] : components_) {
        for (UIComponent* c : components) {
            c->Update();
        }
    }
}

void ContainerComponent::UpdateWindowSize() {
    UpdateComponentsOnWindowSize(aspectRatioModifier_);
}

glm::vec2 ContainerComponent::GetOffset() const {
    return canvas_->GetOffset() + GetTransform().pos + offset;
}

size_t ContainerComponent::GetChildCount() {
    return children_.size();
}

void ContainerComponent::AddChild(UI::UIComponent* c) {
    c->AddToCanvas(this);
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