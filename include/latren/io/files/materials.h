#pragma once

#include "../serializationinterface.h"
#include <latren/graphics/material.h>

namespace Serialization {
    class LATREN_API MaterialSerializer : public JSONFileSerializer, public SerializerItemInterface<std::shared_ptr<Material>> {
    using JSONFileSerializer::JSONFileSerializer;
    protected:
        bool ParseJSON() override;
    };
};
