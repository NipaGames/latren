#pragma once

#include <latren/latren.h>
#include <mutex>
#include <unordered_map>

namespace Input {
    enum class SpecialKey {
        NONE,
        BACKSPACE,
        ENTER
    };
    enum class KeyboardEventType {
        NONE,
        TEXT_INPUT_ASCII_CHAR,
        TEXT_INPUT_SPECIAL,
    };
    struct KeyboardEvent {
        SpecialKey special;
        char character;
    };

    extern std::mutex KEYS_MUTEX_;
    extern std::unordered_map<int, bool> KEYS_;
    extern std::unordered_map<int, bool> KEYS_PRESSED_BEFORE_POLL_;
    extern std::unordered_map<int, bool> KEYS_PRESSED_;

    extern std::mutex MOUSE_BUTTON_MUTEX_;
    extern std::unordered_map<int, bool> MOUSE_BUTTONS_;
    extern std::unordered_map<int, bool> MOUSE_BUTTONS_PRESSED_BEFORE_POLL_;
    extern std::unordered_map<int, bool> MOUSE_BUTTONS_PRESSED_;

    extern std::atomic_bool WINDOW_SIZE_CHANGE_PENDING_;
    extern std::atomic_bool SET_FULLSCREEN_PENDING_;

    extern std::atomic_bool MOUSE_MOVE_PENDING_;
    extern std::atomic_bool FIRST_MOUSE_;
    extern std::atomic_bool MOUSE_SCROLL_PENDING_;
    extern std::atomic<float> MOUSE_SCROLL_AMOUNT_;
    
    extern std::atomic_bool IS_MOUSE_LOCKED_;
    extern std::atomic_bool CURSOR_MODE_CHANGE_PENDING_;
    extern std::atomic_bool VSYNC_POLL_RATE_CHANGE_PENDING_;
    extern std::atomic_bool WINDOW_FOCUS_PENDING_;
    extern std::atomic_bool CONSOLE_FOCUS_PENDING_;

    extern std::atomic<KeyboardEvent> KEYBOARD_EVENT_;
    extern std::atomic<KeyboardEventType> KEYBOARD_EVENT_TYPE_;

    extern bool UPDATE_FULLSCREEN_;

    LATREN_API void KeyDown(int);
    LATREN_API void KeyUp(int);
    LATREN_API bool IsKeyDown(int);
    LATREN_API bool IsKeyPressedDown(int);
    LATREN_API void ClearKeysPressedDown();
    LATREN_API void PollKeysPressedDown();

    LATREN_API void MouseButtonDown(int);
    LATREN_API void MouseButtonUp(int);
    LATREN_API bool IsMouseButtonDown(int);
    LATREN_API bool IsMouseButtonPressedDown(int);
    LATREN_API void ClearMouseButtonsPressedDown();
    LATREN_API void PollMouseButtonsPressedDown();
}