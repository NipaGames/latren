#pragma once

#include <latren/latren.h>
#include "threads/atomic.h"
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

    enum class KeyState {
        UP,
        DOWN,
        PRESSED_DOWN,
        RELEASED
    };

    class LATREN_API KeyInputListener {
    private:
        std::mutex mutex_;
        std::unordered_map<int, KeyState> keysBeforePoll_;
        std::unordered_map<int, KeyState> keys_;
    public:
        KeyInputListener() = default;
        KeyInputListener(const KeyInputListener&);

        void Poll();
        void UpdateStates();
        void Press(int);
        void Release(int);

        KeyState GetState(int);
        bool IsDown(int);
        bool IsPressedDown(int);
        bool IsReleased(int);

        KeyInputListener& operator=(const KeyInputListener&);
    };

    // interactions from the window thread (received by the game thread)
    enum class ReceiveInteraction : uint8_t {
        VSYNC_POLL_RATE_CHANGING,
        WINDOW_SIZE_CHANGE,
        SET_FULLSCREEN,
        MOUSE_MOVE
    };

    // interactions sent to the window thread (sent by the game thread)
    enum class SendInteraction : uint8_t {
        UPDATE_FULLSCREEN,
        CURSOR_MODE_CHANGE,
        FOCUS_CONSOLE,
        FOCUS_WINDOW
    };
}

// basically a layer to connect the input and the game thread
class InputSystem {
public:
    Input::KeyInputListener keyboardListener;
    Input::KeyInputListener mouseButtonListener;

    Threads::Atomic<bool> windowSizeChangePending = false;
    Threads::Atomic<bool> setFullscreenPending = false;
    bool updateFullscreen = false;

    Threads::Atomic<bool> mouseMovePending = false;
    Threads::Atomic<double> mousePosX;
    Threads::Atomic<double> mousePosY;
    Threads::Atomic<bool> firstMouseInteraction = false;
    Threads::Atomic<bool> mouseScrollPending = false;
    Threads::Atomic<float> mouseScrollAmount;
    
    Threads::Atomic<bool> isMouseLocked = false;
    Threads::Atomic<bool> cursorModeChangePending = false;
    Threads::Atomic<bool> cursorChangePending = false;
    Threads::Atomic<int> cursorType;
    Threads::Atomic<bool> vsyncPollRateChangePending = false;
    Threads::Atomic<bool> windowFocusPending = false;
    Threads::Atomic<bool> consoleFocusPending = false;

    Threads::Atomic<Input::KeyboardEvent> keyboardEvent;
    Threads::Atomic<Input::KeyboardEventType> keyboardEventType = Input::KeyboardEventType::NONE;
};
