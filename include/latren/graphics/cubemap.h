#pragma once
#include <latren/latren.h>

#include <string>
#include <vector>
#include "texture.h"

namespace Cubemap {
    LATREN_API Texture::TextureID LoadTextureFromDir(const std::string&, bool = false);
    LATREN_API Texture::TextureID LoadTextureFromFaces(const std::string&, const char* const[6], bool = false);
};