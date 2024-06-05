#include <latren/graphics/textureatlas.h>
#include <stb/stb_rect_pack.h>

#include <algorithm>
#include <array>
#include <set>

using namespace Texture;

int FindOptimalSizeAndPack(std::vector<stbrp_rect>& rects, std::set<int> sizes = { 128, 256, 512, 1024, 2048, 4096 }) {
    int finalSize = 0;

    // since stbrp_pack_rects is inplace, i think it's faster to do this with copies than
    // first trying to pack for all and then run the packing again for the smallest fitting size (final packing called twice)
    std::vector<stbrp_rect> rectsCopy = rects;

    for (auto it = sizes.rbegin(); it != sizes.rend(); it++) {
        int s = *it;

        stbrp_context context;
        std::vector<stbrp_node> nodes(s * 2); // no clue what this is used for lol
        stbrp_init_target(&context, s, s, nodes.data(), nodes.size());
        stbrp_pack_rects(&context, rectsCopy.data(), rectsCopy.size());

        auto firstNotFitting = std::find_if(rectsCopy.begin(), rectsCopy.end(), [](const stbrp_rect& r) {
            return !r.was_packed;
        });
        if (firstNotFitting != rectsCopy.end())
            return finalSize;

        rects = rectsCopy;
        finalSize = s;
    }
    
    return finalSize;
}

TextureAtlas Texture::CreateAtlas(const std::vector<Sprite>& sprites, int channels, int padding) {
    std::vector<stbrp_rect> rects(sprites.size());
    for (int i = 0; i < rects.size(); i++) {
        stbrp_rect& r = rects[i];
        const Sprite& s = sprites.at(i);
        r.id = i;
        r.x = 0;
        r.y = 0;
        r.w = s.w + padding * 2;
        r.h = s.h + padding * 2;
        r.was_packed = false;
    }

    TextureAtlas atlas;
    int size = FindOptimalSizeAndPack(rects);
    if (size == 0) {
        return atlas;
    }

    atlas.buffer = new uint8_t[size * size * channels];
    std::fill_n(atlas.buffer, size * size * channels, 0x00);
    atlas.spriteData.reserve(rects.size());
    for (const stbrp_rect& r : rects) {
        const Sprite& sprite = sprites.at(r.id);
        atlas.spriteData.push_back({
            r.id,
            glm::ivec2(r.x + padding, r.y + padding)
        });
        int h = (r.h - padding * 2);
        for (int y = 0; y < h; y++) {
            int w = (r.w - padding * 2);
            std::copy(sprite.buffer + (y * w) * channels, sprite.buffer + (y * w + w) * channels, atlas.buffer + ((r.y + y + padding) * size + r.x + padding) * channels);
        }
    }
    atlas.w = size;
    atlas.h = size;

    return atlas;
}