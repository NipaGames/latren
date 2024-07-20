#pragma once

#include <latren/latren.h>

template <typename CounterT = std::size_t>
class IDFactory {
protected:
    CounterT counter_ = 0;
public:
    IDFactory() = default;
    IDFactory(CounterT c) : counter_(c) { }
    CounterT NextID() {
        return counter_++;
    }
};

typedef std::size_t EventID;
typedef std::size_t EntityID;