#include <latren/ui/component/uicomponent.h>
#include <latren/ui/canvas.h>
#include <latren/ec/entity.h>

using namespace UI;

void Rect::operator+=(const glm::vec2& offset) {
    left += offset.x;
    right += offset.x;
    top += offset.y;
    bottom += offset.y;
}

Rect Rect::operator+(const glm::vec2& offset) const {
    Rect r = *this;
    r += offset;
    return r;
}

void Rect::operator-=(const glm::vec2& offset) {
    left -= offset.x;
    right -= offset.x;
    top -= offset.y;
    bottom -= offset.y;
}

Rect Rect::operator-(const glm::vec2& offset) const {
    Rect r = *this;
    r -= offset;
    return r;
}

Rect::operator glm::vec2() const {
    return { left, bottom };
}

UITransform UIComponent::GetTransform() const {
    if (transformFrom == UITransformFrom::ENTITY_TRANSFORM_2D) {
        UITransform trans;
        trans.pos = glm::vec2(parent.GetTransform().position->x, parent.GetTransform().position->y);
        // janky ass way to determine the size
        trans.size = parent.GetTransform().size->z;
        return trans;
    }
    return transform;
}

void UIComponent::SetTransform(const UITransform& trans) {
    transform = trans;
    if (transformFrom == UITransformFrom::ENTITY_TRANSFORM_2D) {
        parent.GetTransform().position->x = transform.pos.x;
        parent.GetTransform().position->y = transform.pos.y;
        parent.GetTransform().size->z = transform.size;
    }
}

void UIComponent::Start() {
    for (const auto& e : subcomponents_) {
        e->Start(this);
    }
}
void UIComponent::Render(const glm::mat4& proj) {
    for (const auto& e : subcomponents_) {
        e->Render(this, proj);
    }
}
void UIComponent::UIUpdate() {
    for (const auto& e : subcomponents_) {
        e->Update(this);
    }
}

Rect UIComponent::GetLocalBounds() const {
    return localBounds_;
}

void UIComponent::SetLocalBounds(const Rect& bounds) {
    localBounds_ = bounds;
}

Rect UIComponent::GetBounds() const {
    Rect bounds = GetLocalBounds();
    if (parent_ == nullptr)
        return bounds;
    glm::vec2 parentOffset = parent_->GetOffset();
    bounds += parentOffset;
    if (!parent_->bgOverflow) {
        float bottom = parent_->bgVerticalAnchor == CanvasBackgroundVerticalAnchor::OVER ? 0 : -parent_->GetBackgroundSize().y;
        bounds.top = std::min(bounds.top, parentOffset.y + bottom + parent_->GetBackgroundSize().y);
        bounds.bottom = std::max(bounds.bottom, parentOffset.y + bottom);

        bounds.left = std::min(bounds.left, parentOffset.x);
        bounds.right = std::max(bounds.right, parentOffset.x + parent_->GetBackgroundSize().x);
    }
    return bounds;
}

const InteractionState& UIComponent::GetInteractionState() const {
    return interaction_;
}

Canvas* UIComponent::GetCanvas() const {
    return parent_;
}