#pragma once

#include "../serializer.h"

namespace Serializer {
    class BlueprintSerializer : public JSONFileSerializer, public SerializerItemInterface<std::vector<TypedComponentData>> {
    using JSONFileSerializer::JSONFileSerializer;
    protected:
        LATREN_API bool ParseJSON() override;
    };
};
