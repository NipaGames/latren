#pragma once

#include <cstdint>

namespace Resources {
    enum class ResourceType : uint32_t  {
        TEXTURE = 1 << 0,
        SHADER  = 1 << 1,
        FONT    = 1 << 2,
        MODEL   = 1 << 3,
        STAGE   = 1 << 4,
        AUDIO   = 1 << 5,
        DATA    = 1 << 6,
        TEXT    = 1 << 7,
        BINARY  = 1 << 8,
        JSON    = 1 << 9,
        CFG     = 1 << 10,
        
        ALL = TEXTURE | SHADER | FONT | MODEL | STAGE | AUDIO | DATA | TEXT | BINARY | JSON | CFG
    };

    constexpr uint32_t operator|(const enum ResourceType l, const enum ResourceType r) {
        return static_cast<uint32_t>(l) | static_cast<uint32_t>(r);
    }
    constexpr uint32_t operator&(const enum ResourceType l, const enum ResourceType r) {
        return static_cast<uint32_t>(l) & static_cast<uint32_t>(r);
    }
};