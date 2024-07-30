#pragma once

#include "../serializationinterface.h"
#include <latren/graphics/object.h>

namespace Serialization {
    class LATREN_API ObjectSerializer : public JSONFileSerializer, public SerializerItemInterface<Object> {
    protected:
        bool ParseJSON() override;
    };
};
