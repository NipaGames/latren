#pragma once

#include "../serializer.h"

namespace Serializer {
    class LATREN_API BlueprintSerializer : public JSONFileSerializer, public SerializerItemInterface<std::vector<TypedComponentData>> {
    using JSONFileSerializer::JSONFileSerializer;
    protected:
        bool ParseJSON() override;
    };
};
