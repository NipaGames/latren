#pragma once

#include <cstdint>

namespace Resources {
    enum class ResourceType : uint32_t  {
        NONE        = 0,
        TEXTURE     = 1U << 0,
        SHADER      = 1U << 1,
        FONT        = 1U << 2,
        MODEL       = 1U << 3,
        STAGE       = 1U << 4,
        AUDIO       = 1U << 5,
        DATA        = 1U << 6,
        TEXT        = 1U << 7,
        BINARY      = 1U << 8,
        JSON        = 1U << 9,
        CFG         = 1U << 10,

        MATERIAL    = 1U << 31,
        OBJECT      = 1U << 30,
        BLUEPRINT   = 1U << 29,
        
        ALL = TEXTURE | SHADER | FONT | MODEL | STAGE | AUDIO | DATA | TEXT | BINARY | JSON | CFG | MATERIAL | OBJECT | BLUEPRINT
    };

    constexpr ResourceType operator~(ResourceType t) {
        return static_cast<ResourceType>(~static_cast<uint32_t>(t));
    }
    constexpr ResourceType operator|(ResourceType l, ResourceType r) {
        return static_cast<ResourceType>(static_cast<uint32_t>(l) | static_cast<uint32_t>(r));
    }
    constexpr ResourceType operator&(ResourceType l, ResourceType r) {
        return static_cast<ResourceType>(static_cast<uint32_t>(l) & static_cast<uint32_t>(r));
    }
    constexpr ResourceType operator^(ResourceType l, ResourceType r) {
        return static_cast<ResourceType>(static_cast<uint32_t>(l) ^ static_cast<uint32_t>(r));
    }
    constexpr ResourceType& operator|=(ResourceType& l, ResourceType r) {
        l = l | r;
        return l;
    }
    constexpr ResourceType& operator&=(ResourceType& l, ResourceType r) {
        l = l & r;
        return l;
    }
    constexpr ResourceType& operator^=(ResourceType& l, ResourceType r) {
        l = l ^ r;
        return l;
    }
    constexpr bool operator==(ResourceType t, uint32_t cmp) {
        return static_cast<uint32_t>(t) == cmp;
    }
    constexpr bool operator!=(ResourceType t, uint32_t cmp) {
        return !(t == cmp);
    }
};