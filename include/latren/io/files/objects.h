#pragma once

#include "../serializer.h"
#include <latren/graphics/object.h>

namespace Serializer {
    class ObjectSerializer : public JSONFileSerializer, public SerializerItemInterface<Object> {
    protected:
        LATREN_API bool ParseJSON() override;
    };
};
