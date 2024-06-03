#pragma once

#include "../serializer.h"
#include <latren/graphics/material.h>

namespace Serializer {
    class LATREN_API MaterialSerializer : public JSONFileSerializer, public SerializerItemInterface<std::shared_ptr<Material>> {
    using JSONFileSerializer::JSONFileSerializer;
    protected:
        bool ParseJSON() override;
    };
};
