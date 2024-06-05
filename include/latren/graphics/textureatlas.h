#pragma once

#include "texture.h"
#include "sprite.h"

#include <vector>

namespace Texture {
struct TextureAtlas {
    struct AtlasSpriteData {
        int id;
        glm::ivec2 offset;
    };
    uint8_t* buffer;
    int w, h;
    std::vector<AtlasSpriteData> spriteData;
};

TextureAtlas CreateAtlas(const std::vector<Sprite>&, int channels, int padding = 0);
};