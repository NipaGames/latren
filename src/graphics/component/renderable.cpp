#include <latren/graphics/component/renderable.h>
#include <latren/game.h>

void IRenderable::AssignToRenderer() {
    Game::GetGameInstanceBase()->GetRenderer().AddRenderable(this);
    isAssignedToRenderer_ = true;
}
void IRenderable::RemoveFromRenderer() {
    Game::GetGameInstanceBase()->GetRenderer().RemoveRenderable(this);
    isAssignedToRenderer_ = false;
}