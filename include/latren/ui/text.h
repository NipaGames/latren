#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <latren/defines/opengl.h>

#include <latren/graphics/shader.h>
#include <latren/defines/wchar.h>
#include "alignment.h"

namespace UI::Text {
    struct Character {
        GLuint texture;
        glm::ivec2 size;
        glm::ivec2 bearing;
        int advance;
        glm::ivec2 atlasOffset = glm::ivec2(0);
    };
    /**
    * ░░████░░█░ \
    * ░██░░░███░ | fromGlyphTop
    * ░██░░░░██░ |
    * ░██░░░░██░ /
    * ░░███████░ -- BaseLine
    * ░░░░░░░██░ \ 
    * ░██░░░░██░ | fromGlyphBottom
    * ░░░█████░░ /
    **/
    struct BaseLine {
        int fromGlyphBottom;
        int fromGlyphTop;
    };
    struct Font {
        FT_Face fontFace;
        int fontHeight;
        glm::ivec2 size;
        BaseLine baseLine;
        std::unordered_map<WCHAR_T, Character> charMap;
        GLuint atlasTexture;
        glm::ivec2 atlasSize;
        LATREN_API const Character& GetChar(WCHAR_T) const;
    };
    
    LATREN_API bool Init();
    // this apparently takes a shit ton of parameters, will have to make a struct or smth
    LATREN_API void RenderText(const Font&, const std::string&, glm::vec2, float, float, HorizontalAlignment = HorizontalAlignment::LEFT, float = 5);
    LATREN_API BaseLine GetBaseLine(const Font&, const std::string&);
    LATREN_API int GetLineWidth(const Font&, const std::string&);
    LATREN_API std::vector<int> GetLineWidths(const Font&, const std::string&);
    LATREN_API int GetTextWidth(const Font&, const std::string&);
    LATREN_API int GetTextHeight(const Font&, const std::string&, int = 5);
};