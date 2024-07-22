#include <latren/systems.h>
#include <latren/game.h>

using namespace Systems;

std::unique_ptr<Game> GLOBAL_GAME_INSTANCE_;

std::function<EntityManager&()> GLOBAL_ENTITY_MANAGER_GETTER_;
std::function<GameWindow&()> GLOBAL_GAME_WINDOW_GETTER_;
std::function<InputSystem&()> GLOBAL_INPUT_SYSTEM_GETTER_;
std::function<Renderer&()> GLOBAL_RENDERER_GETTER_;
std::function<ResourceManager&()> GLOBAL_RESOURCES_GETTER_;
std::function<AudioPlayer&()> GLOBAL_AUDIO_PLAYER_GETTER_; 
std::function<PhysicsWorld&()> GLOBAL_PHYSICS_GETTER_;
std::function<double()> GLOBAL_TIME_GETTER_; 
std::function<double()> GLOBAL_DELTA_TIME_GETTER_;

void Systems::UseGameInstance(std::unique_ptr<Game>&& instance) {
    GLOBAL_GAME_INSTANCE_ = std::move(instance);
    SetEntityManagerGetter([]() -> EntityManager& { return GetGame().GetEntityManager(); });
    SetGameWindowGetter([]() -> GameWindow& { return GetGame().GetGameWindow(); });
    SetInputSystemGetter([]() -> InputSystem& { return GetGame().GetGameWindow().inputSystem; });
    SetRendererGetter([]() -> Renderer& { return GetGame().GetRenderer(); });
    SetResourcesGetter([]() -> ResourceManager& { return GetGame().GetResources(); });
    SetAudioPlayerGetter([]() -> AudioPlayer& { return GetGame().GetAudioPlayer(); });
    SetPhysicsGetter([]() -> PhysicsWorld& { return GetGame().GetPhysics(); });
    SetTimeGetter([] { return GetGame().GetTime(); });
    SetDeltaTimeGetter([] { return GetGame().GetDeltaTime(); });
}

Game& Systems::GetGame() {
    return *GLOBAL_GAME_INSTANCE_;
}
EntityManager& Systems::GetEntityManager() {
    if (!GLOBAL_ENTITY_MANAGER_GETTER_) {
        spdlog::error("Getter not defined for Systems::GetEntityManager!");
        throw;
    }
    return GLOBAL_ENTITY_MANAGER_GETTER_();
}
GameWindow& Systems::GetGameWindow() {
    if (!GLOBAL_GAME_WINDOW_GETTER_) {
        spdlog::error("Getter not defined for Systems::GetGameWindow!");
        throw;
    }
    return GLOBAL_GAME_WINDOW_GETTER_();
}
InputSystem& Systems::GetInputSystem() {
    if (!GLOBAL_INPUT_SYSTEM_GETTER_) {
        spdlog::error("Getter not defined for Systems::GetInputSystem!");
        throw;
    }
    return GLOBAL_INPUT_SYSTEM_GETTER_();
}
Renderer& Systems::GetRenderer() {
    if (!GLOBAL_RENDERER_GETTER_) {
        spdlog::error("Getter not defined for Systems::GetRenderer!");
        throw;
    }
    return GLOBAL_RENDERER_GETTER_();
}
ResourceManager& Systems::GetResources() {
    if (!GLOBAL_RESOURCES_GETTER_) {
        spdlog::error("Getter not defined for Systems::GetResources!");
        throw;
    }
    return GLOBAL_RESOURCES_GETTER_();
}
AudioPlayer& Systems::GetAudioPlayer() {
    if (!GLOBAL_AUDIO_PLAYER_GETTER_) {
        spdlog::error("Getter not defined for Systems::GetAudioPlayer!");
        throw;
    }
    return GLOBAL_AUDIO_PLAYER_GETTER_();
}
PhysicsWorld& Systems::GetPhysics() {
    if (!GLOBAL_PHYSICS_GETTER_) {
        spdlog::error("Getter not defined for Systems::GetPhysics!");
        throw;
    }
    return GLOBAL_PHYSICS_GETTER_();
}

double Systems::GetTime() {
    if (!GLOBAL_TIME_GETTER_) {
        spdlog::error("Getter not defined for Systems::GetTime!");
        throw;
    }
    return GLOBAL_TIME_GETTER_();
}
double Systems::GetDeltaTime() {
    if (!GLOBAL_DELTA_TIME_GETTER_) {
        spdlog::error("Getter not defined for Systems::GetDeltaTime!");
        throw;
    }
    return GLOBAL_DELTA_TIME_GETTER_();
}

void Systems::SetEntityManagerGetter(const std::function<EntityManager&()>& getter) {
    GLOBAL_ENTITY_MANAGER_GETTER_ = getter;
}
void Systems::SetGameWindowGetter(const std::function<GameWindow&()>& getter) {
    GLOBAL_GAME_WINDOW_GETTER_ = getter;
}
void Systems::SetInputSystemGetter(const std::function<InputSystem&()>& getter) {
    GLOBAL_INPUT_SYSTEM_GETTER_ = getter;
}
void Systems::SetRendererGetter(const std::function<Renderer&()>& getter) {
    GLOBAL_RENDERER_GETTER_ = getter;
}
void Systems::SetResourcesGetter(const std::function<ResourceManager&()>& getter) {
    GLOBAL_RESOURCES_GETTER_ = getter;
}
void Systems::SetAudioPlayerGetter(const std::function<AudioPlayer&()>& getter) {
    GLOBAL_AUDIO_PLAYER_GETTER_ = getter;
}
void Systems::SetPhysicsGetter(const std::function<PhysicsWorld&()>& getter) {
    GLOBAL_PHYSICS_GETTER_ = getter;
}
void Systems::SetTimeGetter(const std::function<double()>& getter) {
    GLOBAL_TIME_GETTER_ = getter;
}
void Systems::SetDeltaTimeGetter(const std::function<double()>& getter) {
    GLOBAL_DELTA_TIME_GETTER_ = getter;
}