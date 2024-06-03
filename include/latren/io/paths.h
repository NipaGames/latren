#pragma once

#include <unordered_map>

#include "fs.h"
#include "resourcetype.h"

namespace Paths {
    inline const std::fs::path RESOURCES_DIR = "../../res";

    inline const std::unordered_map<Resources::ResourceType, std::fs::path> RESOURCE_DIRS = {
        { Resources::ResourceType::SHADER, RESOURCES_DIR / "shaders" },
        { Resources::ResourceType::TEXTURE, RESOURCES_DIR / "textures" },
        { Resources::ResourceType::FONT, RESOURCES_DIR / "fonts" },
        { Resources::ResourceType::STAGE, RESOURCES_DIR / "stages" },
        { Resources::ResourceType::MODEL, RESOURCES_DIR / "models" },
        { Resources::ResourceType::AUDIO, RESOURCES_DIR / "audio" },
        { Resources::ResourceType::DATA, RESOURCES_DIR / "data" },

        { Resources::ResourceType::TEXT, RESOURCES_DIR / "data" },
        { Resources::ResourceType::BINARY, RESOURCES_DIR / "data" },
        { Resources::ResourceType::JSON, RESOURCES_DIR / "data" },
        { Resources::ResourceType::CFG, RESOURCES_DIR / "data" }
    };

    inline const std::fs::path IMPORTS_PATH = RESOURCES_DIR / "imports.cfg";
    inline const std::fs::path MATERIALS_PATH = RESOURCES_DIR / "materials.json";
    inline const std::fs::path OBJECTS_PATH = RESOURCES_DIR / "objects.json";
    inline const std::fs::path BLUEPRINTS_PATH = RESOURCES_DIR / "blueprints.json";

    inline const std::fs::path LATREN_CORE_RESOURCES_DIR = RESOURCES_DIR / ".latren";
    inline const std::fs::path LATREN_CORE_SHADER_DIR = LATREN_CORE_RESOURCES_DIR / "shaders";

    inline const std::fs::path USER_DIR = "../../user";
    inline const std::fs::path VIDEO_SETTINGS_PATH = USER_DIR / "video.cfg";
    inline const std::fs::path SAVEDATA_DIR = USER_DIR / "savedata";

    inline std::string Path(const std::fs::path& p, const std::string& subPath) {
        return std::fs::path(p / subPath).string();
    }
    inline std::string Path(const std::string& p, const std::string& subPath) {
        return Path(std::fs::path(p), subPath);
    }
};