#pragma once

#include "../serializer.h"
#include <latren/graphics/material.h>

namespace Serializer {
    class MaterialSerializer : public JSONFileSerializer, public SerializerItemInterface<std::shared_ptr<Material>> {
    using JSONFileSerializer::JSONFileSerializer;
    protected:
        LATREN_API bool ParseJSON() override;
    };
};
