#include <latren/ui/component/interactable.h>
#include <latren/game.h>
#include <latren/input.h>
#include <latren/ui/canvas.h>

using namespace UI;

void Interactable::Update(UIComponent* c) {
    const UI::Rect& rect = c->GetBounds();
    InteractionState& intr = c->interaction_;
    const glm::vec2& mPos = c->GetCanvas()->GetMousePosition();
    intr.isHoveredOver = (mPos.x > rect.left && mPos.x < rect.right && mPos.y > rect.bottom && mPos.y < rect.top);
    if (intr.isHoveredOver) {
        if (!intr.prevHovered) {
            c->eventHandler.Dispatch("mouseEnter");
        }
        intr.prevHovered = true;
        if (Input::IsMouseButtonPressedDown(GLFW_MOUSE_BUTTON_1)) {
            c->eventHandler.Dispatch("click");
        }
    }
    else {
        if (intr.prevHovered) {
            c->eventHandler.Dispatch("mouseLeave");
        }
        intr.prevHovered = false;
    }
}