#include <latren/game.h>
#include <latren/defines/opengl.h>
#include <latren/input.h>
#include <latren/physics/physics.h>
#include <latren/io/files/cfg.h>
#include <latren/io/paths.h>

#include <latren/debugmacros.h>
#include <latren/ec/serialization.h>

bool Game::InitWindow() {
    window_ = GameWindow("", LATREN_BASE_WND_WIDTH, LATREN_BASE_WND_HEIGHT);
    if (!window_.Create(renderer_)) {
        return false;
    }
    return true;
}

void Game::Run() {
    running_ = true;
    deltaTime_ = GetTime();

    // Run rendering, updates etc. in a separate thread
    // This way polling doesn't interrupt the whole program
    std::thread gameThread(&Game::GameThread, this);
    window_.SetupInputSystem();
    while (!wndInit_);
    window_.Show(wndInitProperties_.size);
    wndInitReady_ = true;
    while (running_) {
        window_.WindowThreadUpdate();
    }
    gameThread.join();
}

void DumpComponentData(const std::vector<ComponentTypeData>& types, const char* path) {
    std::ofstream file(path);
    for (const ComponentTypeData& type : types) {
        std::vector<std::tuple<std::string, int, std::type_index>> fields;
        fields.reserve(type.serializableFields.size());
        for (const auto& [fName, f] : type.serializableFields) {
            fields.push_back({ fName, (int) f.offset, f.type });
        }
        std::sort(fields.begin(), fields.end(), [](const auto& lhs, const auto& rhs) {
            return std::get<1>(lhs) < std::get<1>(rhs);
        });
        file << type.name << "\n";
        for (const auto& f : fields) {
            file << std::get<1>(f) << " " << std::get<0>(f) << " <" << std::get<2>(f).name() << ">\n";
        }
        file << "\n";
    }
}

void Game::GameThreadInit() {
    srand(static_cast<unsigned int>(time(0)));
    RegisterComponents();
    #ifdef LATREN_DUMP_COMPONENT_DATA
    DumpComponentData(ComponentSerialization::GetComponentTypes(), LATREN_DUMP_COMPONENT_DATA);
    #endif
    RegisterDeserializers();
    if (!audioPlayer_.Init())
        spdlog::error("Audio disabled!");
    resources_.LoadConfigs();
    window_.UseVsync(resources_.videoSettings.useVsync);
    window_.RequestFullscreenResolution(resources_.videoSettings.fullscreenResolution);
    if (resources_.videoSettings.fullscreen)
        window_.inputSystem.updateFullscreen = true;
    ShowAndWaitForWindow(resources_.videoSettings.resolution);
    glfwMakeContextCurrent(window_.GetWindow());
    Serialization::CFGSerializer importsSerializer = Serialization::CFGSerializer(Resources::ImportsFileTemplate());
    importsSerializer.DeserializeFile("${imports.cfg}"_resp);
    entityManager_.Setup();
    PreLoad();
    resources_.GetShaderManager()->LoadStandardShaders();
    renderer_.UpdateVideoSettings(resources_.videoSettings);
    resources_.LoadImports(importsSerializer.GetData());
}

void Game::ShowAndWaitForWindow(const glm::ivec2& res) {
    wndInitProperties_.size = res;
    wndInit_ = true;
    // wait until the window is shown in the main thread
    while (!wndInitReady_);
}

void Game::StartEntities() {
    entityManager_.StartAll();
    renderer_.Start();
}

void Game::GameThreadStart() {
    physics_.Init();
    Start();
    StartEntities();
    prevUpdate_ = GetTime();
    prevFixedUpdate_ = prevUpdate_;
}

void Game::GameThreadCleanUp() {
    CleanUp();
    physics_.Destroy();
    window_.eventHandler.ClearEvents();
    window_.keyboardEventHandler.ClearEvents();
    resources_.GetStageManager()->UnloadAllStages();
    renderer_.CleanUp();
    entityManager_.ClearEverything();
    audioPlayer_.DeleteAllSources();
}

void Game::GameThreadDestroy() { }

void Game::GameThreadPrepareUpdate() {
    window_.inputSystem.keyboardListener.UpdateStates();
    window_.inputSystem.mouseButtonListener.UpdateStates();

    if (glfwWindowShouldClose(window_.GetWindow())) {
        running_ = false;
        return;
    }
    
    double currentTime = GetTime();
    if (limitFps_ > 0 && currentTime - prevUpdate_ < 1.0 / limitFps_)
        return;
    // don't skip too big intervals (>.5s)
    deltaTime_ = std::min(currentTime - prevUpdate_, .5);
    if (freezeDeltaTime_) {
        deltaTime_ = 0;
        freezeDeltaTime_ = false;
    }
    prevUpdate_ = currentTime;
    glm::ivec2 videoModeSize = window_.GetVideoModeSize();
    if (window_.inputSystem.setFullscreenPending) {
        glm::ivec2 videoModeSize = window_.GetVideoModeSize();
        window_.inputSystem.setFullscreenPending = false;
        window_.inputSystem.windowSizeChangePending = false;
        renderer_.UpdateCameraProjection(videoModeSize.x, videoModeSize.y);
        window_.eventHandler.Dispatch<const glm::ivec2&>(WindowEventType::WINDOW_RESIZE, videoModeSize);
    }
    if (window_.inputSystem.windowSizeChangePending) {
        window_.inputSystem.windowSizeChangePending = false;
        glm::ivec2 wndSize = window_.GetSize();
        if (wndSize.x > 0 && wndSize.y > 0)
            renderer_.UpdateCameraProjection(wndSize.x, wndSize.y);
        window_.eventHandler.Dispatch<const glm::ivec2&>(WindowEventType::WINDOW_RESIZE, wndSize);
    }
    
    if (physics_.GetDynamicsWorld() != nullptr)
        physics_.Update(deltaTime_);
    
    isFixedUpdate_ = currentTime - prevFixedUpdate_ > 1.0 / fixedUpdateRate_;
    if (isFixedUpdate_) {
        window_.inputSystem.keyboardListener.Poll();
        window_.inputSystem.mouseButtonListener.Poll();
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
        entityManager_.FixedUpdateAll();
    }
    Update();
    entityManager_.UpdateAll();
    
    Camera& cam = renderer_.GetCamera();
    cam.viewMatrix = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
    cam.UpdateFrustum();
    
    renderer_.Render();
    glfwSwapBuffers(window_.GetWindow());

    audioPlayer_.UseCameraTransform(cam);
}

void Game::GameThread() {
    GameThreadInit();
    GameThreadStart();
    while (running_) {
        GameThreadUpdate();
    }
    GameThreadCleanUp();
    GameThreadDestroy();
}

void Game::Quit() {
    running_ = false;
}

double Game::GetTime() const {
    return glfwGetTime();
}

double Game::GetDeltaTime() const {
    return deltaTime_;
}

double Game::GetFixedDeltaTime() const {
    return 1.0 / fixedUpdateRate_;
}

void Game::FreezeDeltaTime() {
    freezeDeltaTime_ = true;
}

bool Game::IsCurrentlyFixedUpdate() const {
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

ModularResourceManager& Game::GetResources() {
    return resources_;
}

AudioPlayer& Game::GetAudioPlayer() {
    return audioPlayer_;
}

PhysicsWorld& Game::GetPhysics() {
    return physics_;
}

void Game::RegisterComponents() {
    ComponentSerialization::RegisterCoreComponents();
}
void Game::RegisterDeserializers() {
    ComponentSerialization::RegisterCoreDeserializers();
}