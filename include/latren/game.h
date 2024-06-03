#pragma once

#include <memory>

#include "gamewindow.h"
#include "graphics/renderer.h"
#include "ec/entitymanager.h"
#include "io/resourcemanager.h"
#include "audio/audioplayer.h"
#include "physics/physics.h"

class LATREN_API Game {
protected:
    EntityManager entityManager_;
    GameWindow window_;
    Renderer renderer_;
    ResourceManager resources_;
    AudioPlayer audioPlayer_;
    PhysicsWorld physics_;

    std::atomic_bool running_ = false;
    struct {
        glm::ivec2 size;
    } wndInitProperties_;
    std::atomic_bool wndInit_ = false;
    std::atomic_bool wndInitReady_ = false;
    
    double prevUpdate_;
    double prevFixedUpdate_;
    // time in seconds since the last frame
    double deltaTime_;
    bool isFixedUpdate_ = false;
    // 0 for unlimited
    int limitFps_ = 0;
    // fixed updates per second
    int fixedUpdateRate_ = 60;
    Threads::Atomic<bool> freezeDeltaTime_ = false;
    // request to show window in the window thread and wait
    virtual void ShowAndWaitForWindow(const glm::ivec2&);
public:
    virtual bool InitWindow();
    virtual void Run();
    virtual void GameThread();
    
    void GameThreadInit();
    void GameThreadStart();
    void GameThreadPrepareUpdate();
    void GameThreadUpdate();
    // remove all physics and rendering entities etc.
    void GameThreadCleanUp();
    // destroy all the core systems (whoa that sounds dramatic), audio player etc.
    void GameThreadDestroy();
    void StartEntities();
    void Quit();
    // Called before the window is shown
    virtual void PreLoad() { }
    // Called before the first update
    virtual void Start() { }
    // Called every frame
    virtual void Update() { }
    virtual void FixedUpdate() { }
    virtual void CleanUp() { }

    virtual double GetTime() const;
    virtual double GetDeltaTime() const;
    virtual double GetFixedDeltaTime() const;
    virtual void FreezeDeltaTime();
    virtual bool IsCurrentlyFixedUpdate() const;
    virtual EntityManager& GetEntityManager();
    virtual GameWindow& GetGameWindow();
    virtual Renderer& GetRenderer();
    virtual ResourceManager& GetResources();
    virtual AudioPlayer& GetAudioPlayer();
    virtual PhysicsWorld& GetPhysics();

    virtual void RegisterComponents();
    virtual void RegisterDeserializers();
};