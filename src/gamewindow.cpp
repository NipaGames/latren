#include <latren/gamewindow.h>
#include <latren/ui/text.h>
#include <latren/input.h>
#include <latren/systems.h>
#include <latren/defines/win32.h>
#include <latren/game.h>

#include <glm/glm.hpp>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <locale>
#include <codecvt>

GameWindow::GameWindow(const std::string& title, int w, int h, bool useVsync) :
    title_(title),
    baseWndSize_(glm::ivec2(w, h)),
    useVsync_(useVsync)
{ }

bool GameWindow::Create(Renderer& renderer) {
    if(!glfwInit()) {
        spdlog::critical("GLFW init failed.");
        return false;
    }
    else {
        spdlog::info("GLFW init successful.");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    window_ = glfwCreateWindow(baseWndSize_.x, baseWndSize_.y, title_.c_str(), NULL, NULL);
    wndWidth_ = baseWndSize_.x;
    wndHeight_ = baseWndSize_.y;
    if(!window_) {
        spdlog::critical("GLFW window init failed. Make sure your GPU is OpenGL 3.3 compatible.");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSetWindowSizeLimits(window_, 400, 225, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glewExperimental = true;
    if(glewInit() != GLEW_OK) {
        spdlog::critical("GLEW init failed. {}", glGetError());
        return false;
    }
    else {
        spdlog::info("GLEW init successful.");
    }

    spdlog::info("OpenGL version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    renderer.skyboxColor = glm::vec3(0.0f);
    renderer.SetViewport(this);
    if(!renderer.Init())
        return false;
    if(!UI::Text::Init())
        return false;

    glfwSwapInterval(useVsync_ ? 1 : 0);
    glfwMakeContextCurrent(nullptr);

    return true;
}

void GameWindow::Show(const glm::ivec2& res) {
    glfwSetWindowSize(window_, res.x, res.y);
    glfwShowWindow(window_);
    wndWidth_ = res.x;
    wndHeight_ = res.y;
}

/*void GameWindow::ResetCursorPos() {
    glfwSetCursorPos(window_, wndWidth_ / 2, wndHeight_ / 2);
}*/

void GameWindow::RequestFullscreenResolution(const glm::ivec2& res) {
    // bool changeResolution = (res.x != fullscreenResolutionWidth_ || res.y != fullscreenResolutionHeight_);
    fullscreenResolutionWidth_ = res.x;
    fullscreenResolutionHeight_ = res.y;
    /*if (changeResolution)
        inputSystem.setFullscreenPending = true;*/
}

void GameWindow::RequestCursorChange(int cursor) {
    inputSystem.cursorType = cursor;
    inputSystem.cursorChangePending = true;
}

void GameWindow::GLFWFramebufferSizeCallback(int width, int height) {
    wndWidth_ = width;
    wndHeight_ = height;
    inputSystem.windowSizeChangePending = true;
}

void GameWindow::GLFWMouseCallback(double x, double y) {
    inputSystem.mousePosX = x;
    inputSystem.mousePosY = y;
    inputSystem.mouseMovePending = true;
}

void GameWindow::InputSpecialKey(Input::SpecialKey event) {
    inputSystem.keyboardEventType = Input::KeyboardEventType::NONE; // just to be safe with the threading
    inputSystem.keyboardEvent = { event, -1 };
    inputSystem.keyboardEventType = Input::KeyboardEventType::TEXT_INPUT_SPECIAL;
}

void GameWindow::GLFWKeyCallback(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_BACKSPACE:
                InputSpecialKey(Input::SpecialKey::BACKSPACE);
                break;
            case GLFW_KEY_ENTER:
                if (glfwGetKey(window_, GLFW_KEY_LEFT_ALT))
                    break;
                InputSpecialKey(Input::SpecialKey::ENTER);
                break;
            default:
                break;
        }
    }
    if (action == GLFW_PRESS) {
        inputSystem.keyboardListener.Press(key);
        if (key == GLFW_KEY_F11 || (key == GLFW_KEY_ENTER && glfwGetKey(window_, GLFW_KEY_LEFT_ALT)))
            inputSystem.updateFullscreen = true;
    }
    else if (action == GLFW_RELEASE) {
        inputSystem.keyboardListener.Release(key);
    }
}

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> UTF_CONVERTER;
void GameWindow::GLFWCharCallback(unsigned int codepoint) {
    std::string utf8String = UTF_CONVERTER.to_bytes(codepoint);
    char charCode = utf8String.length() == 1 ? utf8String.at(0) : -1;
    if (charCode == -1)
        return;
    inputSystem.keyboardEventType = Input::KeyboardEventType::NONE; // just to be safe with the threading
    inputSystem.keyboardEvent = { Input::SpecialKey::NONE, charCode };
    inputSystem.keyboardEventType = Input::KeyboardEventType::TEXT_INPUT_ASCII_CHAR;
}

void GameWindow::GLFWMouseButtonCallback(int mouseButton, int action, int mods) {
    if (action == GLFW_PRESS) {
        inputSystem.mouseButtonListener.Press(mouseButton);
    }
    else if (action == GLFW_RELEASE) {
        inputSystem.mouseButtonListener.Release(mouseButton);
    }
}

void GameWindow::GLFWMouseScrollCallback(double offsetX, double offsetY) {
    if (offsetY == 0.0f)
        return;
    inputSystem.mouseScrollAmount = (float) offsetY;
    inputSystem.mouseScrollPending = true;
}

void GameWindow::SetupInputSystem() {
    inputSystem.isMouseLocked = true;
    double mouseX, mouseY;
    glfwGetCursorPos(window_, &mouseX, &mouseY);
    inputSystem.mousePosX = mouseX;
    inputSystem.mousePosY = mouseY;
    inputSystem.mouseMovePending = true;

    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* w, int width, int height) {
        static_cast<GameWindow*>(glfwGetWindowUserPointer(w))->GLFWFramebufferSizeCallback(width, height);
    });
    glfwSetKeyCallback(window_, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        static_cast<GameWindow*>(glfwGetWindowUserPointer(w))->GLFWKeyCallback(key, scancode, action, mods);
    });
    glfwSetCharCallback(window_, [](GLFWwindow* w, unsigned int codepoint) {
        static_cast<GameWindow*>(glfwGetWindowUserPointer(w))->GLFWCharCallback(codepoint);
    });
    glfwSetMouseButtonCallback(window_, [](GLFWwindow* w, int mouseButton, int action, int mods) {
        static_cast<GameWindow*>(glfwGetWindowUserPointer(w))->GLFWMouseButtonCallback(mouseButton, action, mods);
    });
    glfwSetCursorPosCallback(window_, [](GLFWwindow* w, double xPos, double yPos) {
        static_cast<GameWindow*>(glfwGetWindowUserPointer(w))->GLFWMouseCallback(xPos, yPos);
    });
    glfwSetScrollCallback(window_, [](GLFWwindow* w, double offsetX, double offsetY) {
        static_cast<GameWindow*>(glfwGetWindowUserPointer(w))->GLFWMouseScrollCallback(offsetX, offsetY);
    });
}

void GameWindow::UseVsync(bool enabled) {
    useVsync_ = enabled;
    inputSystem.vsyncPollRateChangePending = true;
}

glm::ivec2 GameWindow::GetSize() const {
    return glm::ivec2(wndWidth_, wndHeight_);
}

glm::ivec2 GameWindow::GetVideoModeSize() const {
    return glm::ivec2(videoModeWidth_, videoModeHeight_);
}

void GameWindow::RequestMouseLock(bool lock) {
    lockMouse_ = lock;
    inputSystem.isMouseLocked = lock;
    inputSystem.cursorModeChangePending = true;
    if (lock) {
        inputSystem.windowFocusPending = true;
    }
}

bool GameWindow::IsMouseLocked() {
    return lockMouse_ && inputSystem.isMouseLocked;
}
const glm::vec2& GameWindow::GetMousePosition() {
    return currentMousePos_;
}
const glm::vec2& GameWindow::GetRelativeMousePosition() {
    return relativeMousePos_;
}

void GameWindow::Update() {
    if (inputSystem.vsyncPollRateChangePending) {
        inputSystem.vsyncPollRateChangePending = false;
        glfwSwapInterval(useVsync_ ? 1 : 0);
    }

    if (inputSystem.mouseMovePending) {
        inputSystem.mouseMovePending = false;
        currentMousePos_ = { inputSystem.mousePosX, inputSystem.mousePosY };
        glm::ivec2 wndSize = GetSize();
        relativeMousePos_ = currentMousePos_ / (glm::vec2) wndSize * glm::vec2(1280, 720);
        relativeMousePos_.y = 720.0f - relativeMousePos_.y;

        if (inputSystem.firstMouseInteraction) {
            prevCursorPos_ = currentMousePos_;
            inputSystem.firstMouseInteraction = false;
        }

        glm::vec2 delta = glm::vec2(currentMousePos_.x - prevCursorPos_.x, prevCursorPos_.y - currentMousePos_.y);
        prevCursorPos_ = currentMousePos_;

        eventHandler.Dispatch<const glm::ivec2&>(WindowEventType::MOUSE_MOVE, delta);
    }

    if (inputSystem.mouseScrollPending) {
        inputSystem.mouseScrollPending = false;
        float amt = inputSystem.mouseScrollAmount;
        eventHandler.Dispatch(WindowEventType::MOUSE_SCROLL, amt);
    }

    if (inputSystem.keyboardEventType != Input::KeyboardEventType::NONE) {
        Input::KeyboardEventType keyboardEventType = inputSystem.keyboardEventType;
        Input::KeyboardEvent keyboardEvent = inputSystem.keyboardEvent;
        keyboardEventHandler.Dispatch(keyboardEventType, keyboardEvent);
        inputSystem.keyboardEventType = Input::KeyboardEventType::NONE;
    }

    if (inputSystem.keyboardListener.IsPressedDown(GLFW_KEY_ESCAPE) && lockMouse_) {
        inputSystem.isMouseLocked = false;
        inputSystem.cursorModeChangePending = true;
    }
    
    if (inputSystem.mouseButtonListener.IsPressedDown(GLFW_MOUSE_BUTTON_LEFT) && !inputSystem.isMouseLocked && lockMouse_) {
        inputSystem.mouseButtonListener.Poll();
        inputSystem.mouseButtonListener.UpdateStates();
        inputSystem.isMouseLocked = true;
        inputSystem.cursorModeChangePending = true;
    }
}

void GameWindow::WindowThreadUpdate() {
    glfwPollEvents();
    int w = wndWidth_;
    int h = wndHeight_;
    if (inputSystem.updateFullscreen) {
        inputSystem.updateFullscreen = false;
        Systems::GetGame().FreezeDeltaTime();
        isFullscreen_ = !isFullscreen_;
        if (isFullscreen_) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            int fullscreenWidth = fullscreenResolutionWidth_;
            int fullscreenHeight = fullscreenResolutionHeight_;
            if (fullscreenWidth <= 0)
                fullscreenWidth = mode->width;
            if (fullscreenHeight <= 0)
                fullscreenHeight = mode->height;
            videoModeWidth_ = fullscreenWidth;
            videoModeHeight_ = fullscreenHeight;
            glfwGetWindowPos(window_, &prevWndPos_.x, &prevWndPos_.y);
            prevWndSize_ = glm::ivec2(w, h);
            if (useVsync_)
                glfwSetWindowMonitor(window_, monitor, 0, 0, fullscreenWidth, fullscreenHeight, mode->refreshRate);
            else
                glfwSetWindowMonitor(window_, monitor, 0, 0, fullscreenWidth, fullscreenHeight, GLFW_DONT_CARE);
            inputSystem.vsyncPollRateChangePending = true;
            inputSystem.setFullscreenPending = true;
        }
        else {
            glfwSetWindowMonitor(window_, nullptr,  prevWndPos_.x, prevWndPos_.y, prevWndSize_.x, prevWndSize_.y, 0);
            inputSystem.windowSizeChangePending = true;
        }
    }
    if (inputSystem.cursorModeChangePending) {
        inputSystem.cursorModeChangePending = false;
        inputSystem.cursorModeChangePending = false;
        glfwSetInputMode(window_, GLFW_CURSOR, inputSystem.isMouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (inputSystem.isMouseLocked)
            inputSystem.firstMouseInteraction = true;
    }
    if (inputSystem.cursorChangePending) {
        inputSystem.cursorChangePending = false;
        glfwSetCursor(window_, glfwCreateStandardCursor(inputSystem.cursorType));
    }
    if (inputSystem.consoleFocusPending) {
        inputSystem.consoleFocusPending = false;
        #ifdef _WIN32
        HWND console = GetConsoleWindow();
        SetForegroundWindow(console);
        SetFocus(console);
        #endif
    }
    if (inputSystem.windowFocusPending) {
        inputSystem.windowFocusPending = false;
        glfwShowWindow(window_);
        glfwFocusWindow(window_);
    }
}