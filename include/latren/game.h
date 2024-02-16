#pragma once

#include <memory>

#include "gamewindow.h"
#include "graphics/renderer.h"
#include "ec/entitymanager.h"
#include "io/resourcemanager.h"

class Game {
protected:
    EntityManager entityManager_;

    GameWindow window_;
    Renderer renderer_;
    ResourceManager resources_;

    bool running_ = false;
    double prevUpdate_;
    double prevFixedUpdate_;
    // time in seconds since the last frame
    double deltaTime_;
    bool isFixedUpdate_ = false;
    // 0 for unlimited
    int limitFps_ = 0;
    // fixed updates per second
    int fixedUpdateRate_ = 60;
    bool freezeDeltaTime_ = false;
public:
    LATREN_API virtual bool InitWindow();
    LATREN_API virtual void Run();
    LATREN_API virtual void GameThread();
    
    LATREN_API void GameThreadInit();
    LATREN_API void GameThreadStart();
    LATREN_API void GameThreadPrepareUpdate();
    LATREN_API void GameThreadUpdate();
    LATREN_API void GameThreadCleanUp();
    LATREN_API void StartEntities();
    LATREN_API void Quit();
    // Called before the window is shown
    virtual void PreLoad() { }
    // Called before the first update
    virtual void Start() { }
    // Called every frame
    virtual void Update() { }
    virtual void FixedUpdate() { }
    virtual void CleanUp() { }

    LATREN_API virtual double GetDeltaTime();
    LATREN_API virtual double GetFixedDeltaTime();
    LATREN_API virtual void FreezeDeltaTime();
    LATREN_API virtual bool IsCurrentlyFixedUpdate();
    LATREN_API virtual EntityManager& GetEntityManager();
    LATREN_API virtual GameWindow& GetGameWindow();
    LATREN_API virtual Renderer& GetRenderer();
    LATREN_API virtual ResourceManager& GetResources();

    LATREN_API static const std::unique_ptr<Game>& GetGameInstanceBase();
    template <typename T>
    static T* CastGameInstance() {
        return static_cast<T*>(Game::GetGameInstanceBase().get());
    }
    LATREN_API static void SetGameInstance(std::unique_ptr<Game>&&);
    template <typename T>
    static void CreateGameInstance() {
        SetGameInstance(std::make_unique<T>());
    }
};

extern std::unique_ptr<Game> GLOBAL_GAME_INSTANCE_;