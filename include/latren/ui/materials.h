#pragma once

#include <latren/graphics/material.h>

namespace UI {
    inline const std::shared_ptr<Material> SOLID_UI_SHAPE_MATERIAL = std::make_shared<Material>(Shaders::ShaderID::UI_SHAPE,
        Material::Uniform("color", glm::vec4(1.0f))
    );
    inline const std::shared_ptr<Material> DEFAULE_CARET_MATERIAL = std::make_shared<Material>(Shaders::ShaderID::UI_BLINK,
        Material::Uniform("color", glm::vec4(1.0f)),
        Material::Uniform("blinkInterval", 1.0f)
    );
}