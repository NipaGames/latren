#include <latren/ui/text.h>
#include <latren/graphics/shape.h>
#include <latren/graphics/textureatlas.h>
#include <latren/io/resourcemanager.h>
#include <latren/debugmacros.h>

#include <stb/stb_image_write.h>

#include <spdlog/spdlog.h>
#include <unordered_map>
#include <iostream>
#include <sstream>

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
    GL_NONE,
    { 0, 0 },
    { 0, 0 },
    50 << 6
};
WCHAR_T MISSING_CHAR = 0x25a1;

const Character& Font::GetChar(WCHAR_T c) const {
    if (charMap.find(c) != charMap.end()) {
        return charMap.at(c);
    }
    if (charMap.find(MISSING_CHAR) != charMap.end()) {
        return charMap.at(MISSING_CHAR);
    }
    return EMPTY_CHAR;
}

const float Font::GetSizeModifier() const {
    return (float) BASE_FONT_SIZE / size.y;
}

template <typename T>
BaseLine GetRowBaseLine(const Font& font, const T& text, bool applyModifier = true) {
    BaseLine bl = { 0, 0 };
    for (auto it = text.begin(); it != text.end(); it++) {
        const Character& c = font.GetChar(*it);
        bl.fromGlyphBottom = std::max(c.size.y - c.bearing.y, bl.fromGlyphBottom);
        bl.fromGlyphTop = std::max(c.bearing.y, bl.fromGlyphTop);
    }
    if (applyModifier) {
        float m = font.GetSizeModifier();
        bl.fromGlyphBottom = (int) std::ceil(bl.fromGlyphBottom * m);
        bl.fromGlyphTop = (int) std::ceil(bl.fromGlyphTop * m);
    }
    return bl;
}

// you should also free everything created with this wicked function
// apparently i haven't bothered. oh well, too lazy to write the frees now
// fonts are loaded only once anyway on start and freed automatically on program halt so it isn't that critical
GLuint CreateOpenGLFontTexture(const uint8_t* buffer, int w, int h) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        w,
        h,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        buffer
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return texture;
}

std::optional<Font> Resources::FontManager::LoadResource(const ResourcePath& path) {
    std::string pathStr = path.GetParsedPathStr();
    Font font;
    const auto& additional = GetAdditionalData();
    glm::ivec2 fontSize = fontSize_;
    if (!additional.empty()) {
        fontSize = { 0, std::get<int>(GetAdditionalData().at(0)) };
    }
    bool createAtlas = true;

    if (FT_New_Face(FREETYPE_LIBRARY, pathStr.c_str(), 0, &font.fontFace))
        return std::nullopt;
    FT_Face& face = font.fontFace;
    FT_Set_Pixel_Sizes(face, fontSize.x, fontSize.y);
    
    bool allGlyphsLoaded = true;
    FT_UInt i;
    FT_ULong c = FT_Get_First_Char(face, &i);
    FT_ULong wcharMax = std::numeric_limits<WCHAR_T>::max();
    
    std::vector<WCHAR_T> chars;
    std::vector<Texture::Sprite> atlasSprites;
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    while (i != 0) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0) {
            allGlyphsLoaded = false;
            continue;
        }
        const FT_GlyphSlot& glyph = face->glyph;
        
        Character character;
        character.size = glm::ivec2(glyph->bitmap.width, glyph->bitmap.rows);
        character.bearing = glm::ivec2(glyph->bitmap_left, glyph->bitmap_top);
        // some bitshift magic from learnopengl.com
        // just multiplies by 64 since for some reason freetype uses 1/64 pixel as a unit
        character.advance = glyph->advance.x >> 6;

        if (!createAtlas)
            character.texture = CreateOpenGLFontTexture(glyph->bitmap.buffer, glyph->bitmap.width, glyph->bitmap.rows);

        font.charMap.insert(std::pair<WCHAR_T, Character>((WCHAR_T) c, character));
        chars.push_back((WCHAR_T) c);

        if (createAtlas) {
            Texture::Sprite s;
            s.w = character.size.x;
            s.h = character.size.y;
            s.buffer = new uint8_t[s.w * s.h];
            std::copy(glyph->bitmap.buffer, glyph->bitmap.buffer + s.w * s.h, s.buffer);
            atlasSprites.push_back(s);
        }

        c = FT_Get_Next_Char(face, c, &i);
        if (c > wcharMax)
            break;
    }
    
    if (!allGlyphsLoaded)
        spdlog::warn("Some glyphs not loaded!", pathStr);
    
    if (createAtlas) {
        Texture::TextureAtlas atlas = Texture::CreateAtlas(atlasSprites, 1, 1);
        GLuint atlasTexture = CreateOpenGLFontTexture(atlas.buffer, atlas.w, atlas.h);
        font.atlasTexture = atlasTexture;
        font.atlasSize = glm::ivec2(atlas.w, atlas.h);

        spdlog::info("Grouped {} characters into a {}x{} atlas texture", atlas.spriteData.size(), atlas.w, atlas.h);

        #ifdef LATREN_DUMP_FONT_ATLAS_PNGS
        stbi_write_png(std::string(std::string(face->family_name) + ".png").c_str(), atlas.w, atlas.h, 1, atlas.buffer, atlas.w);
        #endif

        for (const auto& sprite : atlas.spriteData) {
            Character& c = font.charMap[chars.at(sprite.id)];
            c.texture = atlasTexture;
            c.atlasOffset = sprite.offset;
        }
        
        // yeah yeah these are redundant but since we're handing raw buffers i'll stick to raw heap arrays
        for (Texture::Sprite& s : atlasSprites) {
            delete[] s.buffer;
        }
        delete[] atlas.buffer;
    }
    else {
        font.atlasTexture = TEXTURE_NONE;
    }

    // this is fucking genius
    font.baseLine = GetRowBaseLine(font, chars, false);
    font.fontHeight = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
    font.size = fontSize;

    return std::optional<Font>(font);
}

void Resources::FontManager::SetFontSize(const glm::ivec2& size) {
    fontSize_ = size;
}

void Resources::FontManager::SetFontSize(int size) {
    SetFontSize(glm::ivec2(fontSize_.x, size));
}

void UI::Text::RenderText(const Font& font, const std::string& text, glm::vec2 pos, float size, float aspectRatio, HorizontalAlignment alignment, float lineSpacing) {    
    glActiveTexture(GL_TEXTURE0);
    Shapes::GetDefaultShape(Shapes::DefaultShape::RECTANGLE_VEC4).Bind();
    std::vector<int> lineWidths = GetLineWidths(font, text);
    int textWidth = *std::max_element(lineWidths.begin(), lineWidths.end());
    int line = 0;
    glm::vec2 startPos = pos;
    
    struct CharToRender {
        glm::vec2 pos;
        const Character& c;
    };
    std::vector<CharToRender> charsToRender;
    float m = font.GetSizeModifier();
    for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
        const Character& c = font.GetChar(*it);
        if (*it == '\n') {
            pos.x = startPos.x;
            pos.y -= (font.fontHeight * size + lineSpacing);
            ++line;
            continue;
        }

        glm::vec2 actualPos;
        actualPos.y = pos.y - (c.size.y - c.bearing.y) * size;
        switch (alignment) {
            case HorizontalAlignment::LEFT:
                actualPos.x = pos.x + c.bearing.x * size;
                break;
            case HorizontalAlignment::RIGHT:
                actualPos.x = pos.x + (c.bearing.x * m + textWidth - lineWidths.at(line)) / m * size * aspectRatio;
                break;
            case HorizontalAlignment::CENTER:
                actualPos.x = pos.x + (c.bearing.x * m * size + (textWidth - lineWidths.at(line)) / 2.0f) / m * size * aspectRatio;
                break;
        }

        charsToRender.push_back({ actualPos, c });
        
        pos.x += c.advance * size * aspectRatio;
    }
    bool useAtlas = (font.atlasTexture != TEXTURE_NONE);

    struct CharQuad {
        struct {
            float posX, posY, texX, texY;
        } tl0, bl0, br0, tl1, br1, tr1;
    };
    if (useAtlas) {
        std::vector<CharQuad> vertices;
        vertices.reserve(charsToRender.size());
        for (const CharToRender& c : charsToRender) {
            float inwards = 0.0f;
            float texTop = (c.c.atlasOffset.y + inwards) / (float) font.atlasSize.y;
            float texBottom = c.c.atlasOffset.y / (float) font.atlasSize.y + (c.c.size.y - inwards) / (float) font.atlasSize.y;
            float texLeft = (c.c.atlasOffset.x + inwards) / (float) font.atlasSize.x;
            float texRight = c.c.atlasOffset.x / (float) font.atlasSize.x + (c.c.size.x - inwards) / (float) font.atlasSize.x;

            float w = c.c.size.x * size * aspectRatio;
            float h = c.c.size.y * size;

            vertices.push_back({
                { c.pos.x,     c.pos.y + h,   texLeft,  texTop },
                { c.pos.x,     c.pos.y,       texLeft,  texBottom },
                { c.pos.x + w, c.pos.y,       texRight, texBottom },

                { c.pos.x,     c.pos.y + h,   texLeft,  texTop },
                { c.pos.x + w, c.pos.y,       texRight, texBottom },
                { c.pos.x + w, c.pos.y + h,   texRight, texTop }
            });
        }
        Shape s;
        s.numVertices = 6 * (int) vertices.size();
        s.numVertexAttributes = 4;
        s.stride = 4;
        s.GenerateVAO();
        s.Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(CharQuad), vertices.data()); 
        glBindTexture(GL_TEXTURE_2D, font.atlasTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6 * (int) vertices.size());
    }
    else {
        Shapes::GetDefaultShape(Shapes::DefaultShape::RECTANGLE_VEC4).Bind();
        for (const auto& c : charsToRender) {
            float w = c.c.size.x * size * aspectRatio;
            float h = c.c.size.y * size;
            CharQuad quad = {
                { c.pos.x,     c.pos.y + h,   0.0f, 0.0f },
                { c.pos.x,     c.pos.y,       0.0f, 1.0f },
                { c.pos.x + w, c.pos.y,       1.0f, 1.0f },

                { c.pos.x,     c.pos.y + h,   0.0f, 0.0f },
                { c.pos.x + w, c.pos.y,       1.0f, 1.0f },
                { c.pos.x + w, c.pos.y + h,   1.0f, 0.0f }
            };
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad), &quad); 

            glBindTexture(GL_TEXTURE_2D, c.c.texture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int UI::Text::GetLineWidth(const Font& font, const std::string& text) {
    int width = 0;
    std::string::const_iterator it = text.begin();
    if (text.length() > 1) {
        while (it != text.end() - 1) {
            width += font.GetChar(*it).advance;
            ++it;
        }
    }
    if (!text.empty()) {
        const Character& lastChar = font.GetChar(*it);
        width += lastChar.bearing.x + lastChar.size.x;
    }
    return (int) std::ceil(width * font.GetSizeModifier());
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

BaseLine UI::Text::GetBaseLine(const Font& font, const std::string& text) {
    if (std::count(text.begin(), text.end(), '\n') > 0) {
        std::size_t first = text.find('\n');
        std::size_t last = text.rfind('\n');
        BaseLine firstRowPadding = GetRowBaseLine(font, text.substr(0, first));
        BaseLine lastRowPadding = GetRowBaseLine(font, text.substr(last + 1));
        return { lastRowPadding.fromGlyphBottom, firstRowPadding.fromGlyphTop };
    }
    return GetRowBaseLine(font, text);
}

int UI::Text::GetRowHeight(const Font& font, const std::string& text) {
    BaseLine bl = GetRowBaseLine(font, text);
    return bl.fromGlyphBottom + bl.fromGlyphTop;
}

int UI::Text::GetTextHeight(const Font& font, const std::string& text, int lineSpacing) {
    int h = 0;
    int linebreaks = (int) std::count(text.begin(), text.end(), '\n');
    if (linebreaks > 0) {
        BaseLine bl = GetBaseLine(font, text);
        h += bl.fromGlyphBottom + bl.fromGlyphTop;
        // rows in between
        h += linebreaks * ((int) (font.fontHeight * font.GetSizeModifier()) + lineSpacing);
    }
    else {
        h = GetRowHeight(font, text);
    }
    return h;
}

int UI::Text::GetFixedTextHeight(const Font& font, const std::string& text, int lineSpacing) {
    int lines = (int) std::count(text.begin(), text.end(), '\n') + 1;
    return lines * ((int) (font.fontHeight * font.GetSizeModifier())) + (lines - 1) * lineSpacing;
}