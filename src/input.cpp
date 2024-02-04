#include <latren/input.h>

using namespace Input;

std::mutex Input::KEYS_MUTEX_;
std::unordered_map<int, bool> Input::KEYS_;
std::unordered_map<int, bool> Input::KEYS_PRESSED_BEFORE_POLL_;
std::unordered_map<int, bool> Input::KEYS_PRESSED_;

std::mutex Input::MOUSE_BUTTON_MUTEX_;
std::unordered_map<int, bool> Input::MOUSE_BUTTONS_;
std::unordered_map<int, bool> Input::MOUSE_BUTTONS_PRESSED_BEFORE_POLL_;
std::unordered_map<int, bool> Input::MOUSE_BUTTONS_PRESSED_;

std::atomic_bool Input::WINDOW_SIZE_CHANGE_PENDING_(false);
std::atomic_bool Input::SET_FULLSCREEN_PENDING_(false);

std::atomic_bool Input::MOUSE_MOVE_PENDING_(false);
std::atomic_bool Input::FIRST_MOUSE_(true);
std::atomic_bool Input::MOUSE_SCROLL_PENDING_(false);
std::atomic<float> Input::MOUSE_SCROLL_AMOUNT_(0.0f);

std::atomic_bool Input::IS_MOUSE_LOCKED_(false);
std::atomic_bool Input::CURSOR_MODE_CHANGE_PENDING_(false);
std::atomic_bool Input::VSYNC_POLL_RATE_CHANGE_PENDING_(false);
std::atomic_bool Input::WINDOW_FOCUS_PENDING_(false);
std::atomic_bool Input::CONSOLE_FOCUS_PENDING_(false);

std::atomic<KeyboardEvent> Input::KEYBOARD_EVENT_;
std::atomic<KeyboardEventType> Input::KEYBOARD_EVENT_TYPE_(KeyboardEventType::NONE);

bool Input::UPDATE_FULLSCREEN_ = false;
