#pragma once

#include <latren/latren.h>

typedef size_t CommonID;

class IDFactory {
protected:
    CommonID counter_ = 0;
public:
    LATREN_API CommonID NextID();
};

typedef CommonID EventID;
typedef CommonID EntityID;