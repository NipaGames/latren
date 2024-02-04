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

inline constexpr int BASE_WIDTH = 1280;
inline constexpr int BASE_HEIGHT = 720;

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

class GameWindow {
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
    bool useVsync_ = true;
public:
    VariantEventHandler<WindowEventType,
        void(float),
        void(const glm::ivec2&)> eventHandler;
    EventHandler<Input::KeyboardEventType, Input::KeyboardEvent> keyboardEventHandler;
    GameWindow() = default;
    LATREN_API GameWindow(const std::string&, int, int, bool = true);
    
    LATREN_API bool Create(Renderer&);
    LATREN_API void Update();
    LATREN_API void SetupInputSystem();
    LATREN_API void UpdateInputSystem();
    LATREN_API void ResetCursorPos();
    bool IsUsingVsync() { return useVsync_; }
    LATREN_API void UseVsync(bool);
    LATREN_API void LockMouse(bool);
    LATREN_API bool IsMouseLocked();
    LATREN_API const glm::vec2& GetMousePosition();
    // returns mouse position in window bounds in range (0, 0), (720, 1280)
    LATREN_API const glm::vec2& GetRelativeMousePosition();
    GLFWwindow* const GetWindow() { return window_; }
};