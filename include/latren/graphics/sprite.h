#pragma once

#include <cstdint>

namespace Texture {
struct Sprite {
    int w, h;
    uint8_t* buffer;
};
};