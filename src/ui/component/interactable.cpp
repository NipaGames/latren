#include <latren/ui/component/interactable.h>
#include <latren/systems.h>
#include <latren/input.h>
#include <latren/ui/canvas.h>

using namespace UI;

void Interactable::Update(UIComponent* c) {
    const UI::Rect& rect = c->GetBounds();
    InteractionState& intr = c->interaction_;
    const glm::vec2& mPos = c->GetCanvas()->GetMousePosition();
    intr.isHoveredOver = (mPos.x > rect.left && mPos.x < rect.right && mPos.y > rect.bottom && mPos.y < rect.top);
    if (Systems::GetInputSystem().mouseButtonListener.IsReleased(GLFW_MOUSE_BUTTON_1)) {
        if (intr.isHoveredOver && isSelected_) {
            c->eventHandler.Dispatch("mouseUp");
        }
        isSelected_ = false;
    }
    if (intr.isHoveredOver) {
        if (!intr.prevHovered) {
            c->eventHandler.Dispatch("mouseEnter");
        }
        intr.prevHovered = true;
        if (Systems::GetInputSystem().mouseButtonListener.IsPressedDown(GLFW_MOUSE_BUTTON_1)) {
            c->eventHandler.Dispatch("mouseDown");
            isSelected_ = true;
        }
    }
    else {
        if (intr.prevHovered) {
            c->eventHandler.Dispatch("mouseLeave");
        }
        intr.prevHovered = false;
    }
}