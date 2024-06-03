#pragma once

#include <memory>
#include <latren/latren.h>

class Game;
class EntityManager;
class EntityManager;
class GameWindow;
class InputSystem;
class Renderer;
class ResourceManager;
class AudioPlayer;
class PhysicsWorld;

// Systems - the crazy useful singleton dictionary
namespace Systems {
    LATREN_API void SetGameInstance(std::unique_ptr<Game>&&);
    template <typename T>
    void CreateGameInstance() {
        SetGameInstance(std::make_unique<T>());
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
    LATREN_API ResourceManager& GetResources();
    LATREN_API AudioPlayer& GetAudioPlayer();
    LATREN_API PhysicsWorld& GetPhysics();
    LATREN_API double GetTime();
    LATREN_API double GetDeltaTime();
};