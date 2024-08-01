#pragma once

#include <cstdint>

namespace RenderPass {
    enum Enum : uint8_t {
        // until i fix transparency all transparent objects should be rendered late
        NORMAL,
        LATE,
        AFTER_POST_PROCESSING,
        // doesn't render in the renderer pipeline
        CUSTOM
    };
    inline constexpr uint8_t TOTAL_RENDER_PASSES = 4;
};