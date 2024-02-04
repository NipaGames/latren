#include <latren/ui/component/uicomponent.h>
#include <latren/entity/entity.h>

using namespace UI;

UIComponent::UIComponent(Canvas* c, int p) :
    canvas_(c),
    priority_(p)
{ }

UIComponent::~UIComponent() {
    if (isAddedToCanvas_ && canvas_ != nullptr)
        canvas_->RemoveUIComponent(this);
    isAddedToCanvas_ = false;
}

void UIComponent::AddToCanvas() {
    if (canvas_ == nullptr)
        return;
    canvas_->AddUIComponent(this, priority_);
    isAddedToCanvas_ = true;
}

void UIComponent::AddToCanvas(Canvas* canvas) {
    canvas_ = canvas;
    AddToCanvas();
}

UITransform UIComponent::GetTransform() const {
    if (transformFrom == UITransformFrom::ENTITY_TRANSFORM_2D) {
        UITransform trans;
        trans.pos = glm::vec2(parent->transform->position.x, parent->transform->position.y);
        // janky ass way to determine the size
        trans.size = parent->transform->size.z;
        return trans;
    }
    return transform;
}

void UIComponent::SetTransform(const UITransform& trans) {
    transform = trans;
    if (transformFrom == UITransformFrom::ENTITY_TRANSFORM_2D) {
        parent->transform->position.x = transform.pos.x;
        parent->transform->position.y = transform.pos.y;
        parent->transform->size.z = transform.size;
    }
}

Rect UIComponent::GetBounds() const {
    if (!isAddedToCanvas_)
        return GetLocalBounds();
    Rect bounds = GetLocalBounds();
    glm::vec2 canvasOffset = canvas_->GetOffset();
    bounds.left += canvasOffset.x;
    bounds.right += canvasOffset.x;
    bounds.top += canvasOffset.y;
    bounds.bottom += canvasOffset.y;
    if (!canvas_->bgOverflow) {
        float bottom = canvas_->bgVerticalAnchor == CanvasBackgroundVerticalAnchor::OVER ? 0 : -canvas_->bgSize.y;
        bounds.top = std::min(bounds.top, canvasOffset.y + bottom + canvas_->bgSize.y);
        bounds.bottom = std::max(bounds.bottom, canvasOffset.y + bottom);

        bounds.left = std::min(bounds.left, canvasOffset.x);
        bounds.right = std::max(bounds.right, canvasOffset.x + canvas_->bgSize.x);
    }
    return bounds;
}