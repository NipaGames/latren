#pragma once

#include "../serializer.h"
#include <latren/graphics/object.h>

namespace Serializer {
    class LATREN_API ObjectSerializer : public JSONFileSerializer, public SerializerItemInterface<Object> {
    protected:
        bool ParseJSON() override;
    };
};
