#include <latren/game.h>
#include <latren/defines/opengl.h>
#include <latren/input.h>
#include <latren/physics/physics.h>
#include <latren/io/serializetypes.h>
#include <latren/io/files/cfg.h>
#include <latren/io/paths.h>

std::unique_ptr<Game> GLOBAL_GAME_INSTANCE_;

bool Game::InitWindow() {
    window_ = GameWindow("", BASE_WIDTH, BASE_HEIGHT);
    if(!window_.Create(renderer_)) {
        return false;
    }
    return true;
}

void Game::Run() {
    running_ = true;
    deltaTime_ = glfwGetTime();

    // Run rendering, updates etc. in a separate thread
    // This way polling doesn't interrupt the whole program
    std::thread gameThread(&Game::GameThread, this);
    window_.SetupInputSystem();

    while (running_) {
        window_.UpdateInputSystem();
    }
    gameThread.join();
}

void Game::GameThreadInit() {
    srand(static_cast<unsigned int>(time(0)));
    RegisterDefaultSerializers();
    glfwMakeContextCurrent(window_.GetWindow());
    resources_.LoadConfigs();
    glfwSetWindowSize(window_.GetWindow(), resources_.videoSettings.resolution.x, resources_.videoSettings.resolution.y);
    window_.UseVsync(resources_.videoSettings.useVsync);
    if (resources_.videoSettings.fullscreen)
        Input::UPDATE_FULLSCREEN_ = true;
    glfwShowWindow(window_.GetWindow());
    Serializer::CFGSerializer importsSerializer = Serializer::CFGSerializer(CFG::ImportsFile());
    importsSerializer.DeserializeFile(Paths::IMPORTS_PATH.string());
    PreLoad();
    resources_.shaderManager.LoadStandardShaders();
    renderer_.UpdateVideoSettings(resources_.videoSettings);
    resources_.LoadImports(importsSerializer.GetData());
}

void Game::StartEntities() {
    for (const auto& entity : entityManager_.entities_) {
        entity->Start();
    }
    renderer_.Start();
}

void Game::GameThreadStart() {
    Physics::Init();
    Start();
    StartEntities();
    prevUpdate_ = glfwGetTime();
    prevFixedUpdate_ = prevUpdate_;
}

void Game::GameThreadCleanUp() {
    CleanUp();
    Physics::Destroy();
    window_.eventHandler.ClearEvents();
    window_.keyboardEventHandler.ClearEvents();
    resources_.stageManager.UnloadAllStages();
    entityManager_.ClearEntities();
    renderer_.CleanUpEntities();
}

void Game::GameThreadPrepareUpdate() {
    Input::ClearKeysPressedDown();
    Input::ClearMouseButtonsPressedDown();

    if (glfwWindowShouldClose(window_.GetWindow())) {
        running_ = false;
        return;
    }
    
    double currentTime = glfwGetTime();
    if (limitFps_ > 0 && currentTime - prevUpdate_ < 1.0 / limitFps_)
        return;
    // don't skip too big intervals (>.5s)
    deltaTime_ = std::min(currentTime - prevUpdate_, .5);
    if (freezeDeltaTime_) {
        deltaTime_ = 0;
        freezeDeltaTime_ = false;
    }
    prevUpdate_ = currentTime;

    if (Input::SET_FULLSCREEN_PENDING_) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        Game::GetGameInstanceBase()->GetRenderer().UpdateCameraProjection(mode->width, mode->height);
        window_.eventHandler.Dispatch<const glm::ivec2&>(WindowEventType::WINDOW_RESIZE, glm::ivec2(mode->width, mode->height));
        Input::SET_FULLSCREEN_PENDING_ = false;
        Input::WINDOW_SIZE_CHANGE_PENDING_ = false;
    }
    if (Input::WINDOW_SIZE_CHANGE_PENDING_) {
        glm::ivec2 wndSize;
        glfwGetFramebufferSize(window_.GetWindow(), &wndSize.x, &wndSize.y);
        if (wndSize.x > 0 && wndSize.y > 0)
            renderer_.UpdateCameraProjection(wndSize.x, wndSize.y);
        window_.eventHandler.Dispatch<const glm::ivec2&>(WindowEventType::WINDOW_RESIZE, wndSize);
        Input::WINDOW_SIZE_CHANGE_PENDING_ = false;
    }
    
    if (Physics::GLOBAL_DYNAMICS_WORLD_ != nullptr)
        Physics::Update(deltaTime_);
    
    isFixedUpdate_ = currentTime - prevFixedUpdate_ > 1.0 / fixedUpdateRate_;
    if (isFixedUpdate_) {
        Input::PollKeysPressedDown();
        Input::PollMouseButtonsPressedDown();
        prevFixedUpdate_ = currentTime;
        renderer_.UpdateFrustum();
        renderer_.SortMeshesByDistance();
    }
}

void Game::GameThreadUpdate() {
    GameThreadPrepareUpdate();
    window_.Update();
    if (isFixedUpdate_) {
        FixedUpdate();
        for (const auto& entity : entityManager_.entities_) {
            entity->FixedUpdate();
        }
    }
    Update();
    for (const auto& entity : entityManager_.entities_) {
        entity->Update();
    }
    Camera& cam = renderer_.GetCamera();
    glm::vec3 direction;
    direction.x = cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
    direction.y = sin(glm::radians(cam.pitch));
    direction.z = sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
    cam.front = glm::normalize(direction);
    cam.right = glm::normalize(glm::cross(cam.front, cam.up));
    cam.UpdateFrustum();
    
    renderer_.Render();
}

void Game::GameThread() {
    GameThreadInit();
    GameThreadStart();
    while (running_) {
        GameThreadUpdate();
    }
    GameThreadCleanUp();
}

double Game::GetDeltaTime() {
    return deltaTime_;
}

double Game::GetFixedDeltaTime() {
    return 1.0 / fixedUpdateRate_;
}

void Game::FreezeDeltaTime() {
    freezeDeltaTime_ = true;
}

bool Game::IsCurrentlyFixedUpdate() {
    return isFixedUpdate_;
}

EntityManager& Game::GetEntityManager() {
    return entityManager_;
}

GameWindow& Game::GetGameWindow() {
    return window_;
}

Renderer& Game::GetRenderer() {
    return renderer_;
}

ResourceManager& Game::GetResources() {
    return resources_;
}

void Game::Quit() {
    running_ = false;
}

void Game::SetGameInstance(std::unique_ptr<Game>&& instance) {
    GLOBAL_GAME_INSTANCE_ = std::move(instance);
}

const std::unique_ptr<Game>& Game::GetGameInstanceBase() {
    return GLOBAL_GAME_INSTANCE_;
}