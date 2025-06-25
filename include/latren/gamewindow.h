#pragma once

#include <latren/defines/opengl.h>
#include <spdlog/spdlog.h>
#include <iostream>
#include <cstdint>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>

#include "event/eventhandler.h"
#include "graphics/renderer.h"
#include "graphics/viewport.h"
#include "threads/atomic.h"
#include "input.h"

enum class WindowEventType {
    MOUSE_MOVE,
    MOUSE_SCROLL,
    WINDOW_RESIZE
};

typedef std::function<void(const glm::ivec2&)> MouseMoveEvent, WindowResizeEvent;
typedef std::function<void(float)> MouseScrollEvent;

namespace Input {
    struct KeyboardEvent;
    enum class KeyboardEventType;
};

class  GameWindow : public Viewport {
private:
    GLFWwindow* window_ = nullptr;
    std::string title_;
    glm::vec2 prevCursorPos_ = glm::vec2(0.0f);
    glm::vec2 currentMousePos_ = glm::vec2(0.0f);
    glm::vec2 relativeMousePos_ = glm::vec2(0.0f);
    bool isFullscreen_ = false;
    bool lockMouse_ = true;
    glm::ivec2 prevWndPos_;
    glm::ivec2 prevWndSize_;
    glm::ivec2 baseWndSize_;
    Threads::Atomic<int> fullscreenResolutionWidth_ = -1;
    Threads::Atomic<int> fullscreenResolutionHeight_ = -1;
    Threads::Atomic<int> wndWidth_, wndHeight_;
    Threads::Atomic<int> videoModeWidth_, videoModeHeight_;
    bool useVsync_ = true;

    void GLFWFramebufferSizeCallback(int, int);
    void GLFWMouseCallback(double, double);
    void GLFWKeyCallback(int, int, int, int);
    void GLFWCharCallback(unsigned int);
    void GLFWMouseButtonCallback(int, int, int);
    void GLFWMouseScrollCallback(double, double);
    void InputSpecialKey(Input::SpecialKey);
public:
    VariantEventHandler<WindowEventType,
        void(float),
        void(const glm::ivec2&)> eventHandler;
    EventHandler<Input::KeyboardEventType, Input::KeyboardEvent> keyboardEventHandler;
    InputSystem inputSystem;

    GameWindow() = default;
    GameWindow(const std::string&, int, int, bool = true);

    bool Create(Renderer&);
    // called in the game thread
    void Update();
    void Show(const glm::ivec2&);
    // called in, well, the window thread (the main thread)
    void WindowThreadUpdate();
    void SetupInputSystem();
    bool IsUsingVsync() { return useVsync_; }
    void UseVsync(bool);
    glm::ivec2 GetSize() const override;
    glm::ivec2 GetVideoModeSize() const;
    bool IsMouseLocked();
    const glm::vec2& GetMousePosition();
    // returns mouse position in window bounds in range (0, 0), (720, 1280)
    const glm::vec2& GetRelativeMousePosition();
    GLFWwindow* const GetWindow() { return window_; }

    void RequestFullscreenResolution(const glm::ivec2&);
    void RequestWindowSize(const glm::ivec2&);
    void RequestCursorChange(int);
    void RequestMouseLock(bool);
    // void ResetCursorPos();
};