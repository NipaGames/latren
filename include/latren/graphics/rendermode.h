#pragma once

enum BasicRenderMode : int {
    RENDER_MODE_NORMAL = 0,
    RENDER_MODE_NO_MATERIALS = 1,

    RENDER_MODE_DEBUG_NORMALS = -1,
    RENDER_MODE_DEBUG_AABBS = -2
};