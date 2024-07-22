#pragma once

#include <latren/defines/opengl.h>
#include <latren/latren.h>

class LATREN_API Viewport {
public:
    virtual glm::ivec2 GetSize() const = 0;
};