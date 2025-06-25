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
     void UseGameInstance(std::unique_ptr<Game>&&);
    template <typename T>
    void CreateGameInstance() {
        UseGameInstance(std::make_unique<T>());
    }
     Game& GetGame();
    template <typename T>
    T& GetGame() {
        return static_cast<T&>(GetGame());
    }
     EntityManager& GetEntityManager();
     GameWindow& GetGameWindow();
     InputSystem& GetInputSystem();
     Renderer& GetRenderer();
     IResourceManager& GetResources();
     AudioPlayer& GetAudioPlayer();
     PhysicsWorld& GetPhysics();
     double GetTime();
     double GetDeltaTime();

     void SetEntityManagerGetter(const std::function<EntityManager&()>&);
     void SetGameWindowGetter(const std::function<GameWindow&()>&);
     void SetInputSystemGetter(const std::function<InputSystem&()>&);
     void SetRendererGetter(const std::function<Renderer&()>&);
     void SetResourcesGetter(const std::function<IResourceManager&()>&);
     void SetAudioPlayerGetter(const std::function<AudioPlayer&()>&);
     void SetPhysicsGetter(const std::function<PhysicsWorld&()>&);
     void SetTimeGetter(const std::function<double()>&);
     void SetDeltaTimeGetter(const std::function<double()>&);
};