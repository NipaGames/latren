#pragma once

#include <atomic>

namespace Threads {
    // basically just a wrapper for std::atomic with copy and assignment
    template <typename T>
    class Atomic {
    private:
        std::atomic<T> val_;
    public:
        Atomic() = default;
        Atomic(T v) : val_(v) { }
        Atomic(const Atomic& c) : val_(c.Get()) { }
        Atomic& operator=(T c) {
            val_ = c;
            return *this;
        }
        Atomic& operator=(const Atomic& c) {
            val_ = c.Get();
            return *this;
        }
        T Get() const { return val_.load(); }
        operator T() const { return Get(); }
    };
};