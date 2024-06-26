#pragma once

#include <latren/latren.h>

template <typename CounterT = size_t>
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

typedef size_t EventID;
typedef size_t EntityID;