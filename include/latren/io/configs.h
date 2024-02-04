#include "serializablestruct.h"
#include <latren/gamewindow.h>

#include <latren/defines/opengl.h>

namespace Config {
    struct VideoSettings : SerializableStruct {
        INHERIT_COPY(VideoSettings);
        
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
        DATA_FIELD(glm::ivec2, resolution, glm::ivec2(BASE_WIDTH, BASE_HEIGHT));
        META_COMMENT("-1 -1 for auto");
        DATA_FIELD(glm::ivec2, fullscreenResolution, glm::ivec2(-1));
    };
};