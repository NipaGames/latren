#include <latren/gamewindow.h>
#include <latren/ui/text.h>
#include <latren/input.h>
#include <latren/game.h>
#include <latren/defines/win32.h>

#include <glm/glm.hpp>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <locale>
#include <codecvt>


namespace Input {
    void KeyDown(int key) {
        std::lock_guard<std::mutex> lock(KEYS_MUTEX_);
        KEYS_[key] = true;
        KEYS_PRESSED_BEFORE_POLL_[key] = true;
    }

    void KeyUp(int key) {
        std::lock_guard<std::mutex> lock(KEYS_MUTEX_);
        KEYS_[key] = false;
        KEYS_PRESSED_[key] = false;
    }

    bool IsKeyDown(int key) {
        std::lock_guard<std::mutex> lock(KEYS_MUTEX_);
        if (KEYS_.find(key) == KEYS_.end())
            return false;
        return KEYS_[key];
    }

    bool IsKeyPressedDown(int key) {
        std::lock_guard<std::mutex> lock(KEYS_MUTEX_);
        if (KEYS_PRESSED_.find(key) == KEYS_PRESSED_.end())
            return false;
        bool val = KEYS_PRESSED_[key];
        return val;
    }


    void ClearKeysPressedDown() {
        std::lock_guard<std::mutex> lock(KEYS_MUTEX_);
        KEYS_PRESSED_.clear();
    }

    void PollKeysPressedDown() {
        std::lock_guard<std::mutex> lock(KEYS_MUTEX_);
        KEYS_PRESSED_.clear();
        KEYS_PRESSED_ = KEYS_PRESSED_BEFORE_POLL_;
        KEYS_PRESSED_BEFORE_POLL_.clear();
    }



    void MouseButtonDown(int mouseButton) {
        std::lock_guard<std::mutex> lock(MOUSE_BUTTON_MUTEX_);
        MOUSE_BUTTONS_[mouseButton] = true;
        MOUSE_BUTTONS_PRESSED_BEFORE_POLL_[mouseButton] = true;
    }

    void MouseButtonUp(int mouseButton) {
        std::lock_guard<std::mutex> lock(MOUSE_BUTTON_MUTEX_);
        MOUSE_BUTTONS_[mouseButton] = false;
        MOUSE_BUTTONS_PRESSED_BEFORE_POLL_[mouseButton] = false;
    }

    bool IsMouseButtonDown(int mouseButton) {
        std::lock_guard<std::mutex> lock(MOUSE_BUTTON_MUTEX_);
        if (MOUSE_BUTTONS_.find(mouseButton) == MOUSE_BUTTONS_.end())
            return false;
        return MOUSE_BUTTONS_[mouseButton];
    }

    bool IsMouseButtonPressedDown(int mouseButton) {
        std::lock_guard<std::mutex> lock(MOUSE_BUTTON_MUTEX_);
        if (MOUSE_BUTTONS_PRESSED_.find(mouseButton) == MOUSE_BUTTONS_PRESSED_.end())
            return false;
        bool val = MOUSE_BUTTONS_PRESSED_[mouseButton];
        return val;
    }

    void ClearMouseButtonsPressedDown() {
        std::lock_guard<std::mutex> lock(MOUSE_BUTTON_MUTEX_);
        MOUSE_BUTTONS_PRESSED_.clear();
    }

    void PollMouseButtonsPressedDown() {
        std::lock_guard<std::mutex> lock(MOUSE_BUTTON_MUTEX_);
        MOUSE_BUTTONS_PRESSED_.clear();
        MOUSE_BUTTONS_PRESSED_ = MOUSE_BUTTONS_PRESSED_BEFORE_POLL_;
        MOUSE_BUTTONS_PRESSED_BEFORE_POLL_.clear();
    }
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Input::WINDOW_SIZE_CHANGE_PENDING_ = true;
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos) {
    Input::MOUSE_MOVE_PENDING_ = true;
}

void InputSpecialKey(Input::SpecialKey event) {
    Input::KEYBOARD_EVENT_TYPE_ = Input::KeyboardEventType::NONE; // just to be safe with the threading
    Input::KEYBOARD_EVENT_ = { event, -1 };
    Input::KEYBOARD_EVENT_TYPE_ = Input::KeyboardEventType::TEXT_INPUT_SPECIAL;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_BACKSPACE:
                InputSpecialKey(Input::SpecialKey::BACKSPACE);
                break;
            case GLFW_KEY_ENTER:
                if (glfwGetKey(window, GLFW_KEY_LEFT_ALT))
                    break;
                InputSpecialKey(Input::SpecialKey::ENTER);
                break;
            default:
                break;
        }
    }
    if (action == GLFW_PRESS) {
        Input::KeyDown(key);
        if (key == GLFW_KEY_F11 || (key == GLFW_KEY_ENTER && glfwGetKey(window, GLFW_KEY_LEFT_ALT)))
            Input::UPDATE_FULLSCREEN_ = true;
    }
    else if (action == GLFW_RELEASE) {
        Input::KeyUp(key);
    }
}

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> UTF_CONVERTER;
void CharCallback(GLFWwindow* window, unsigned int codepoint) {
    std::string utf8String = UTF_CONVERTER.to_bytes(codepoint);
    char charCode = utf8String.length() == 1 ? utf8String.at(0) : -1;
    if (charCode == -1)
        return;
    Input::KEYBOARD_EVENT_TYPE_ = Input::KeyboardEventType::NONE; // just to be safe with the threading
    Input::KEYBOARD_EVENT_ = { Input::SpecialKey::NONE, charCode };
    Input::KEYBOARD_EVENT_TYPE_ = Input::KeyboardEventType::TEXT_INPUT_ASCII_CHAR;
}

void MouseButtonCallback(GLFWwindow* window, int mouseButton, int action, int mods) {
    if (action == GLFW_PRESS) {
        Input::MouseButtonDown(mouseButton);
    }
    else if (action == GLFW_RELEASE) {
        Input::MouseButtonUp(mouseButton);
    }
}

void MouseScrollCallback(GLFWwindow* window, double offsetX, double offsetY) {
    if (offsetY == 0.0f)
        return;
    Input::MOUSE_SCROLL_AMOUNT_ = (float) offsetY;
    Input::MOUSE_SCROLL_PENDING_ = true;
}

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
    renderer.SetWindow(window_);
    if(!renderer.Init())
        return false;
    if(!UI::Text::Init())
        return false;

    glfwSwapInterval(useVsync_ ? 1 : 0);
    glfwMakeContextCurrent(nullptr);

    return true;
}

void GameWindow::Update() {
    if (Input::VSYNC_POLL_RATE_CHANGE_PENDING_) {
        glfwSwapInterval(useVsync_ ? 1 : 0);
        Input::VSYNC_POLL_RATE_CHANGE_PENDING_ = false;
    }

    if (Input::MOUSE_MOVE_PENDING_) {
        Input::MOUSE_MOVE_PENDING_ = false;
        double xPos, yPos;
        glfwGetCursorPos(window_, &xPos, &yPos);

        currentMousePos_ = { xPos, yPos };
        glm::ivec2 wndSize;
        glfwGetWindowSize(window_, &wndSize.x, &wndSize.y);
        relativeMousePos_ = currentMousePos_ / (glm::vec2) wndSize * glm::vec2(1280, 720);
        relativeMousePos_.y = 720.0f - relativeMousePos_.y;

        if (Input::FIRST_MOUSE_) {
            prevCursorPos_ = { xPos, yPos };
            Input::FIRST_MOUSE_ = false;
        }

        glm::ivec2 delta = glm::ivec2(xPos - prevCursorPos_.x, prevCursorPos_.y - yPos);
        prevCursorPos_ = { xPos, yPos };

        eventHandler.Dispatch<const glm::ivec2&>(WindowEventType::MOUSE_MOVE, delta);
    }

    if (Input::MOUSE_SCROLL_PENDING_) {
        Input::MOUSE_SCROLL_PENDING_ = false;
        float amt = Input::MOUSE_SCROLL_AMOUNT_;
        eventHandler.Dispatch(WindowEventType::MOUSE_SCROLL, amt);
    }

    if (Input::KEYBOARD_EVENT_TYPE_ != Input::KeyboardEventType::NONE) {
        Input::KeyboardEventType keyboardEventType = Input::KEYBOARD_EVENT_TYPE_;
        Input::KeyboardEvent keyboardEvent = Input::KEYBOARD_EVENT_;
        keyboardEventHandler.Dispatch(keyboardEventType, keyboardEvent);
        Input::KEYBOARD_EVENT_TYPE_ = Input::KeyboardEventType::NONE;
    }

    if (Input::WINDOW_FOCUS_PENDING_) {
        Input::WINDOW_FOCUS_PENDING_ = false;
        glfwShowWindow(window_);
        glfwFocusWindow(window_);
    }

    if (Input::IsKeyPressedDown(GLFW_KEY_ESCAPE) && lockMouse_) {
        Input::IS_MOUSE_LOCKED_ = false;
        Input::CURSOR_MODE_CHANGE_PENDING_ = true;
    }
    
    if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !Input::IS_MOUSE_LOCKED_ && lockMouse_) {
        Input::PollMouseButtonsPressedDown();
        Input::ClearMouseButtonsPressedDown();
        Input::IS_MOUSE_LOCKED_ = true;
        Input::CURSOR_MODE_CHANGE_PENDING_ = true;
    }
}

void GameWindow::ResetCursorPos() {
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    glfwSetCursorPos(window_, width / 2, height / 2);
}

void GameWindow::SetupInputSystem() {
    Input::IS_MOUSE_LOCKED_ = true;
    Input::MOUSE_MOVE_PENDING_ = true;

    glfwSetFramebufferSizeCallback(window_, FramebufferSizeCallback);
    glfwSetKeyCallback(window_, KeyCallback);
    glfwSetCharCallback(window_, CharCallback);
    glfwSetMouseButtonCallback(window_, MouseButtonCallback);
    glfwSetCursorPosCallback(window_, MouseCallback);
    glfwSetScrollCallback(window_, MouseScrollCallback);
}

void GameWindow::UseVsync(bool enabled) {
    useVsync_ = enabled;
    Input::VSYNC_POLL_RATE_CHANGE_PENDING_ = true;
}

void GameWindow::LockMouse(bool lock) {
    lockMouse_ = lock;
    Input::IS_MOUSE_LOCKED_ = lock;
    Input::CURSOR_MODE_CHANGE_PENDING_ = true;
    if (lock) {
        Input::WINDOW_FOCUS_PENDING_ = true;
    }
}

bool GameWindow::IsMouseLocked() {
    return lockMouse_ && Input::IS_MOUSE_LOCKED_;
}

const glm::vec2& GameWindow::GetMousePosition() {
    return currentMousePos_;
}
const glm::vec2& GameWindow::GetRelativeMousePosition() {
    return relativeMousePos_;
}

void GameWindow::UpdateInputSystem() {
    glfwWaitEvents();
    if (Input::UPDATE_FULLSCREEN_) {
        Input::UPDATE_FULLSCREEN_ = false;
        Game::GetGameInstanceBase()->FreezeDeltaTime();
        isFullscreen_ = !isFullscreen_;
        glm::ivec2 windowSize;
        if (isFullscreen_) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            int fullscreenWidth = Game::GetGameInstanceBase()->GetResources().videoSettings.fullscreenResolution.x;
            int fullscreenHeight = Game::GetGameInstanceBase()->GetResources().videoSettings.fullscreenResolution.y;
            if (fullscreenWidth <= 0)
                fullscreenWidth = mode->width;
            if (fullscreenHeight <= 0)
                fullscreenHeight = mode->height;
            glfwGetWindowPos(window_, &prevWndPos_.x, &prevWndPos_.y);
            glfwGetWindowSize(window_, &prevWndSize_.x, &prevWndSize_.y);
            if (useVsync_)
                glfwSetWindowMonitor(window_, monitor, 0, 0, fullscreenWidth, fullscreenHeight, mode->refreshRate);
            else
                glfwSetWindowMonitor(window_, monitor, 0, 0, fullscreenWidth, fullscreenHeight, GLFW_DONT_CARE);
            Input::VSYNC_POLL_RATE_CHANGE_PENDING_ = true;
            Input::SET_FULLSCREEN_PENDING_ = true;
        }
        else {
            glfwSetWindowMonitor(window_, nullptr,  prevWndPos_.x, prevWndPos_.y, prevWndSize_.x, prevWndSize_.y, 0);
            glfwGetWindowSize(window_, &windowSize.x, &windowSize.y);
            Game::GetGameInstanceBase()->GetRenderer().UpdateCameraProjection(windowSize.x, windowSize.y);
        }
    }
    if (Input::CURSOR_MODE_CHANGE_PENDING_) {
        Input::CURSOR_MODE_CHANGE_PENDING_ = false;
        glfwSetInputMode(window_, GLFW_CURSOR, Input::IS_MOUSE_LOCKED_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (Input::IS_MOUSE_LOCKED_) {
            Input::FIRST_MOUSE_ = true;
        }
    }
    if (Input::CONSOLE_FOCUS_PENDING_) {
        Input::CONSOLE_FOCUS_PENDING_ = false;
        #ifdef _WIN32
        HWND console = GetConsoleWindow();
        SetForegroundWindow(console);
        SetFocus(console);
        #endif
    }
}