#pragma once

#include <memory>
#include <functional>
#include <latren/latren.h>

class Game;
class EntityManager;
class EntityManager;
class GameWindow;
class InputSystem;
class Renderer;
class IResourceManager;
class AudioPlayer;
class PhysicsWorld;

// Systems - the crazy useful singleton dictionary
namespace Systems {
    LATREN_API void UseGameInstance(std::unique_ptr<Game>&&);
    template <typename T>
    void CreateGameInstance() {
        UseGameInstance(std::make_unique<T>());
    }
    LATREN_API Game& GetGame();
    template <typename T>
    T& GetGame() {
        return static_cast<T&>(GetGame());
    }
    LATREN_API EntityManager& GetEntityManager();
    LATREN_API GameWindow& GetGameWindow();
    LATREN_API InputSystem& GetInputSystem();
    LATREN_API Renderer& GetRenderer();
    LATREN_API IResourceManager& GetResources();
    LATREN_API AudioPlayer& GetAudioPlayer();
    LATREN_API PhysicsWorld& GetPhysics();
    LATREN_API double GetTime();
    LATREN_API double GetDeltaTime();

    LATREN_API void SetEntityManagerGetter(const std::function<EntityManager&()>&);
    LATREN_API void SetGameWindowGetter(const std::function<GameWindow&()>&);
    LATREN_API void SetInputSystemGetter(const std::function<InputSystem&()>&);
    LATREN_API void SetRendererGetter(const std::function<Renderer&()>&);
    LATREN_API void SetResourcesGetter(const std::function<IResourceManager&()>&);
    LATREN_API void SetAudioPlayerGetter(const std::function<AudioPlayer&()>&);
    LATREN_API void SetPhysicsGetter(const std::function<PhysicsWorld&()>&);
    LATREN_API void SetTimeGetter(const std::function<double()>&);
    LATREN_API void SetDeltaTimeGetter(const std::function<double()>&);
};