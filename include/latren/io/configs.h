#pragma once

#include "serializablestruct.h"

#include <latren/defines/opengl.h>
#include <latren/defines/wndmetrics.h>

namespace Config {
    struct VideoSettings : SerializableStruct {        
        DATA_FIELD(float, gamma, 1.0f);
        DATA_FIELD(float, contrast, 1.0f);
        DATA_FIELD(float, brightness, 1.0f);
        DATA_FIELD(float, saturation, 1.0f);
        META_NEWLINE();
        DATA_FIELD(float, fov, 60.0f);
        META_NEWLINE();
        DATA_FIELD(bool, useVsync, true);
        DATA_FIELD(bool, fullscreen, false);
        META_NEWLINE();
        DATA_FIELD(glm::ivec2, resolution, glm::ivec2(LATREN_BASE_WND_WIDTH, LATREN_BASE_WND_HEIGHT));
        META_COMMENT("-1 -1 for auto");
        DATA_FIELD(glm::ivec2, fullscreenResolution, glm::ivec2(-1));
    };
};