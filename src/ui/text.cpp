#include <latren/ui/text.h>
#include <latren/graphics/shape.h>
#include <latren/io/resourcemanager.h>

#include <spdlog/spdlog.h>
#include <unordered_map>
#include <iostream>

using namespace UI::Text;

FT_Library FREETYPE_LIBRARY;

bool UI::Text::Init() {
    if (FT_Init_FreeType(&FREETYPE_LIBRARY)) {
        spdlog::error("FreeType init failed!");
        return false;
    }
    spdlog::info("FreeType init successful.");
    return true;
}

Character EMPTY_CHAR = {
    NULL,
    { 0, 0 },
    { 0, 0 },
    50 << 6
};
WCHAR MISSING_CHAR = 0x25a1;

const Character& Font::GetChar(WCHAR c) const {
    if (charMap.count(c) != 0) {
        return charMap.at(c);
    }
    if (charMap.count(MISSING_CHAR) != 0) {
        return charMap.at(MISSING_CHAR);
    }
    return EMPTY_CHAR;
}

template <typename T>
glm::ivec2 GetRowVerticalPadding(const Font& font, const T& text) {
    int max = 0;
    int min = 0;
    auto it = text.begin();
    while (it != text.end()) {
        const Character& c = font.GetChar(*it);
        int cMax = c.bearing.y;
        int cMin = c.bearing.y - c.size.y;
        if (cMax > max) max = cMax;
        if (cMin < min) min = cMin;
        ++it;
    }
    return glm::ivec2(-min, max);
}

bool RenderGlyphs(Font& font) {
    FT_Face& face = font.fontFace;
    font.fontHeight = face->height >> 6;
    bool success = true;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    std::vector<WCHAR> chars;
    FT_UInt i;
    FT_ULong c = FT_Get_First_Char(face, &i);
    FT_ULong wcharMax = std::numeric_limits<WCHAR>::max();
    while (i != 0) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0) {
            success = false;
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        font.charMap.insert(std::pair<WCHAR, Character>((WCHAR) c, character));
        chars.push_back((WCHAR) c);
        c = FT_Get_Next_Char(face, c, &i);
        if (c > wcharMax)
            break;
    }
    // this is fucking genius
    font.padding = GetRowVerticalPadding(font, chars);
    glBindTexture(GL_TEXTURE_2D, 0);
    return success;
}

std::optional<Font> Resources::FontManager::LoadResource(const std::fs::path& path) {
    std::string pathStr = path.string();
    Font font;
    const auto& additional = GetAdditionalData();
    glm::ivec2 fontSize = fontSize_;
    if (!additional.empty()) {
        fontSize = { 0, std::get<int>(GetAdditionalData().at(0)) };
    }
    if (FT_New_Face(FREETYPE_LIBRARY, pathStr.c_str(), 0, &font.fontFace))
        return std::nullopt;
    FT_Set_Pixel_Sizes(font.fontFace, fontSize.x, fontSize.y);
    if (!RenderGlyphs(font))
        spdlog::warn("Some glyphs not loaded!", pathStr);
    font.size = fontSize;
    return std::optional<Font>(font);
}

void Resources::FontManager::SetFontSize(const glm::ivec2& size) {
    fontSize_ = size;
}

void Resources::FontManager::SetFontSize(int size) {
    SetFontSize(glm::ivec2(fontSize_.x, size));
}

void UI::Text::RenderText(const Font& font, const std::string& text, glm::vec2 pos, float size, float modifier, HorizontalAlignment alignment, float lineSpacing) {    
    glActiveTexture(GL_TEXTURE0);
    Shapes::RECTANGLE_VEC4.Bind();
    std::vector<int> lineWidths = GetLineWidths(font, text);
    int textWidth = *std::max_element(lineWidths.begin(), lineWidths.end());
    int line = 0;
    glm::vec2 startPos = pos;
    for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
        const Character& c = font.GetChar(*it);
        float fontModifier = ((float) font.size.y / BASE_FONT_SIZE);
        if (*it == '\n') {
            pos.x = startPos.x;
            pos.y -= (font.fontHeight * fontModifier * size + lineSpacing);
            ++line;
            continue;
        }

        glm::vec2 actualPos;
        actualPos.y = pos.y - (c.size.y - c.bearing.y) * size;
        switch (alignment) {
            case HorizontalAlignment::LEFT:
                actualPos.x = pos.x + c.bearing.x * size * fontModifier;
                break;
            case HorizontalAlignment::RIGHT:
                actualPos.x = pos.x + (c.bearing.x + textWidth - lineWidths.at(line)) * size * fontModifier;
                break;
            case HorizontalAlignment::CENTER:
                actualPos.x = pos.x + (c.bearing.x * size + (textWidth - lineWidths.at(line)) / 2.0f) * size * fontModifier;
                break;
        }
        float w = c.size.x * size * modifier;
        float h = c.size.y * size;

        float vertices[6][4] = {
            { actualPos.x,     actualPos.y + h,   0.0f, 0.0f },
            { actualPos.x,     actualPos.y,       0.0f, 1.0f },
            { actualPos.x + w, actualPos.y,       1.0f, 1.0f },

            { actualPos.x,     actualPos.y + h,   0.0f, 0.0f },
            { actualPos.x + w, actualPos.y,       1.0f, 1.0f },
            { actualPos.x + w, actualPos.y + h,   1.0f, 0.0f }
        };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

        glBindTexture(GL_TEXTURE_2D, c.texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // some bitshift magic from learnopengl.com
        // just multiplies by 64 since for some reason freetype uses 1/64 pixel as a unit
        pos.x += (c.advance >> 6) * size * modifier;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int UI::Text::GetLineWidth(const Font& font, const std::string& text) {
    int width = 0;
    for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
        width += (font.GetChar(*it).advance >> 6);
    }
    return (int) (width * ((float) BASE_FONT_SIZE / font.size.y));
}


std::vector<int> UI::Text::GetLineWidths(const Font& font, const std::string& text) {
    std::vector<int> widths;
    if (std::count(text.begin(), text.end(), '\n') > 0) {
        std::stringstream ss(text);
        std::string ln;
        while (std::getline(ss, ln, '\n')) {
            widths.push_back(GetLineWidth(font, ln));
        }
    }
    else {
        widths.push_back(GetLineWidth(font, text));
    }
    return widths;
}


int UI::Text::GetTextWidth(const Font& font, const std::string& text) {
    std::vector<int> widths = GetLineWidths(font, text);
    return *std::max_element(widths.begin(), widths.end());
}

glm::ivec2 UI::Text::GetVerticalPadding(const Font& font, const std::string& text) {
    if (std::count(text.begin(), text.end(), '\n') > 0) {
        size_t first = text.find_first_of('\n');
        size_t last = text.find_last_of('\n');
        glm::ivec2 firstRowPadding = GetRowVerticalPadding(font, text.substr(0, first));
        glm::ivec2 lastRowPadding = GetRowVerticalPadding(font, text.substr(last + 1));
        return glm::ivec2(lastRowPadding[0], firstRowPadding[1]);
    }
    return GetRowVerticalPadding(font, text);
}

int UI::Text::GetTextHeight(const Font& font, const std::string& text, int lineSpacing) {
    int h = 0;
    int additionalRows = (int) std::count(text.begin(), text.end(), '\n');
    glm::ivec2 padding;
    if (additionalRows > 0) {
        h += additionalRows * (font.fontHeight + lineSpacing);
        std::string lastRow = text.substr(text.find_last_of('\n'));
        padding = GetRowVerticalPadding(font, lastRow);
        if (padding[1] + padding[0] == 0)
            padding[1] = font.padding[1];
    }
    else {
        padding = GetVerticalPadding(font, text);
    }
    h += (int) ((float) (padding[1] + padding[0]) * ((float) BASE_FONT_SIZE / font.size.y));
    return h;
}