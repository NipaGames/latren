#include <latren/input.h>

using namespace Input;

void KeyInputListener::Poll() {
    std::lock_guard<std::mutex> lock(mutex_);
    keys_ = keysBeforePoll_;
}

void KeyInputListener::UpdateStates() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [k, v] : keys_) {
        if (v == KeyState::PRESSED_DOWN) {
            keysBeforePoll_[k] = KeyState::DOWN;
            keys_[k] = KeyState::DOWN;
        }
        else if (v == KeyState::RELEASED) {
            keysBeforePoll_[k] = KeyState::UP;
            keys_[k] = KeyState::UP;
        }
    }
}

void KeyInputListener::Press(int key) {
    std::lock_guard<std::mutex> lock(mutex_);
    keysBeforePoll_[key] = KeyState::PRESSED_DOWN;
}

void KeyInputListener::Release(int key) {
    std::lock_guard<std::mutex> lock(mutex_);
    keysBeforePoll_[key] = KeyState::RELEASED;
}

KeyState KeyInputListener::GetState(int key) {
    if (keys_.find(key) == keys_.end())
        return KeyState::UP;
    return keys_.at(key);
}

bool KeyInputListener::IsDown(int key) {
    KeyState state = GetState(key);
    return state == KeyState::DOWN || state == KeyState::PRESSED_DOWN;
}

bool KeyInputListener::IsPressedDown(int key) {
    return GetState(key) == KeyState::PRESSED_DOWN;
}

bool KeyInputListener::IsReleased(int key) {
    return GetState(key) == KeyState::RELEASED;
}

KeyInputListener::KeyInputListener(const KeyInputListener& from) :
    keysBeforePoll_(from.keysBeforePoll_),
    keys_(from.keysBeforePoll_)
{ }

KeyInputListener& KeyInputListener::operator=(const KeyInputListener& from) {
    std::lock_guard<std::mutex> lock(mutex_);
    keysBeforePoll_ = from.keysBeforePoll_;
    keys_ = from.keys_;
    return *this;
}