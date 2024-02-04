#include <latren/ui/component/listcomponent.h>
#include <latren/ui/component/textcomponent.h>
#include <latren/game.h>

using namespace UI;

ListComponent::~ListComponent() {
    if (scrollEvent_ != -1)
        Game::GetGameInstanceBase()->GetGameWindow().eventHandler.Unsubscribe(WindowEventType::MOUSE_SCROLL, scrollEvent_);
}

void ListComponent::SetScrollPos(float pos) {
    scrollPos_ = pos;
    float rootPos = GetOffset().y;
    for (int i = 0; i < children_.size(); i++) {
        UIComponent* c = children_.at(i);
        c->transform.pos.y = i * -itemSpacing + pos;
    }
}

void ListComponent::Start() {
    scrollEvent_ = Game::GetGameInstanceBase()->GetGameWindow().eventHandler.Subscribe(WindowEventType::MOUSE_SCROLL, [this](float delta) {
        if (!isActive)
            return;
        // nothing to scroll; the rendered area is smaller than the viewport
        if (GetChildCount() * itemSpacing < bgSize.y)
            return;
        float pos = scrollPos_ - delta * scrollSensitivity;
        pos = std::max(pos, 0.0f);
        pos = std::min(pos, GetChildCount() * itemSpacing - bgSize.y);
        SetScrollPos(pos);
    });
}

void ListComponent::AddListItem(UI::UIComponent* c) {
    c->transform.pos.y = GetChildCount() * -itemSpacing + scrollPos_;
    AddChild(c);
}

void TextListComponent::AddListItem(const std::string& str) {
    TextComponent* textItem = new TextComponent(this);
    textItem->transformFrom = UITransformFrom::UI_TRANSFORM;
    textItem->font = "FONT_FIRACODE";
    textItem->transform.size = .5f;
    textItem->verticalAlignment = VerticalAlignment::TOP;
    switch (itemAlignment) {
        case HorizontalAlignment::LEFT:
            textItem->transform.pos.x = 0.0f;
            break;
        case HorizontalAlignment::RIGHT:
            textItem->transform.pos.x = bgSize.x;
            break;
        case HorizontalAlignment::CENTER:
            textItem->transform.pos.x = bgSize.x / 2.0f;
            break;
    }
    textItem->horizontalAlignment = itemAlignment;
    textItem->SetText(str);
    ListComponent::AddListItem(textItem);
}