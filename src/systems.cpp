#include <latren/systems.h>
#include <latren/game.h>

std::unique_ptr<Game> GLOBAL_GAME_INSTANCE_;

using namespace Systems;
void Systems::SetGameInstance(std::unique_ptr<Game>&& instance) {
    GLOBAL_GAME_INSTANCE_ = std::move(instance);
}
Game& Systems::GetGame() {
    return *GLOBAL_GAME_INSTANCE_;
}
EntityManager& Systems::GetEntityManager() {
    return GetGame().GetEntityManager();
}
GameWindow& Systems::GetGameWindow() {
    return GetGame().GetGameWindow();
}
InputSystem& Systems::GetInputSystem() {
    return GetGameWindow().inputSystem;
}
Renderer& Systems::GetRenderer() {
    return GetGame().GetRenderer();
}
ResourceManager& Systems::GetResources() {
    return GetGame().GetResources();
}
AudioPlayer& Systems::GetAudioPlayer() {
    return GetGame().GetAudioPlayer();
}
PhysicsWorld& Systems::GetPhysics() {
    return GetGame().GetPhysics();
}

double Systems::GetTime() {
    return GetGame().GetTime();
}
double Systems::GetDeltaTime() {
    return GetGame().GetDeltaTime();
}