#pragma once

#include <unordered_map>

#include "resourcepath.h"
#include "resourcetype.h"

namespace Paths {
    inline const std::unordered_map<std::string, ResourcePath> DEFAULT_PATH_VARS = {
        { "gamedir", "${cwd}" }, // cwd is internally declared as std::filesystem::current_path()
        { "res", "${gamedir}/res" },
        { "core_res", "${res}/.latren" },
        { "usr", "${gamedir}/usr" },

        // resource dirs
        { "shaders", "${res}/shaders" },
        { "textures", "${res}/textures" },
        { "fonts", "${res}/fonts" },
        { "stages", "${res}/stages" },
        { "models", "${res}/models" },
        { "audio", "${res}/audio" },
        { "data", "${res}/data" },
        { "data_bin", "${data}" },
        { "data_text", "${data}" },
        { "data_json", "${data}" },
        { "data_cfg", "${data}" },

        // core resource dirs
        { "core_shaders", "${core_res}/shaders" },
        { "core_textures", "${core_res}/textures" },
        { "core_fonts", "${core_res}/fonts" },
        { "core_stages", "${core_res}/stages" },
        { "core_models", "${core_res}/models" },
        { "core_audio", "${core_res}/audio" },
        { "core_data", "${core_res}/data" },
        { "core_data_bin", "${core_data}" },
        { "core_data_text", "${core_data}" },
        { "core_data_json", "${core_data}" },
        { "core_data_cfg", "${core_data}" },

        // resource configs
        { "imports.cfg", "${res}/imports.cfg" },
        { "materials.json", "${res}/materials.json" },
        { "objects.json", "${res}/objects.json" },
        { "blueprints.json", "${res}/blueprints.json" },

        // user
        { "video.cfg", "${usr}/video.cfg" },
        { "savedata", "${usr}/savedata" }
    };

    const ResourcePath& GetGlobalPathVar(const std::string&);
    void SetGlobalPathVar(const std::string&, const ResourcePath&);

    inline const std::unordered_map<Resources::ResourceType, ResourcePath> RESOURCE_DIRS = {
        { Resources::ResourceType::SHADER, "${shaders}" },
        { Resources::ResourceType::TEXTURE, "${textures}" },
        { Resources::ResourceType::FONT, "${fonts}" },
        { Resources::ResourceType::STAGE, "${stages}" },
        { Resources::ResourceType::MODEL, "${models}" },
        { Resources::ResourceType::AUDIO, "${audio}" },
        { Resources::ResourceType::DATA, "${data}" },

        { Resources::ResourceType::TEXT, "${data_text}" },
        { Resources::ResourceType::BINARY, "${data_bin}" },
        { Resources::ResourceType::JSON, "${data_json}" },
        { Resources::ResourceType::CFG, "${data_cfg}" }
    };
};