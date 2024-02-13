#include <latren/ui/component/uicomponent.h>
#include <latren/entity/entity.h>

using namespace UI;

UIComponent::UIComponent(Canvas* c, int p) :
    canvas(c),
    priority_(p)
{ }

UIComponent::~UIComponent() {
    if (isAddedToCanvas_ && canvas != nullptr)
        canvas->RemoveUIComponent(*this);
    isAddedToCanvas_ = false;
}

void UIComponent::AddToCanvas() {
    if (canvas == nullptr)
        return;
    canvas->AddUIComponent({ pool, parent }, priority_);
    isAddedToCanvas_ = true;
}

void UIComponent::AddToCanvas(Canvas* canvas) {
    canvas = canvas;
    AddToCanvas();
}

UITransform UIComponent::GetTransform() const {
    if (transformFrom == UITransformFrom::ENTITY_TRANSFORM_2D) {
        UITransform trans;
        trans.pos = glm::vec2(parent.GetTransform().position.x, parent.GetTransform().position.y);
        // janky ass way to determine the size
        trans.size = parent.GetTransform().size.z;
        return trans;
    }
    return transform;
}

void UIComponent::SetTransform(const UITransform& trans) {
    transform = trans;
    if (transformFrom == UITransformFrom::ENTITY_TRANSFORM_2D) {
        parent.GetTransform().position.x = transform.pos.x;
        parent.GetTransform().position.y = transform.pos.y;
        parent.GetTransform().size.z = transform.size;
    }
}

Rect UIComponent::GetBounds() const {
    if (!isAddedToCanvas_)
        return GetLocalBounds();
    Rect bounds = GetLocalBounds();
    glm::vec2 canvasOffset = canvas->GetOffset();
    bounds.left += canvasOffset.x;
    bounds.right += canvasOffset.x;
    bounds.top += canvasOffset.y;
    bounds.bottom += canvasOffset.y;
    if (!canvas->bgOverflow) {
        float bottom = canvas->bgVerticalAnchor == CanvasBackgroundVerticalAnchor::OVER ? 0 : -canvas->bgSize.y;
        bounds.top = std::min(bounds.top, canvasOffset.y + bottom + canvas->bgSize.y);
        bounds.bottom = std::max(bounds.bottom, canvasOffset.y + bottom);

        bounds.left = std::min(bounds.left, canvasOffset.x);
        bounds.right = std::max(bounds.right, canvasOffset.x + canvas->bgSize.x);
    }
    return bounds;
}