#pragma once

#include "../serializationinterface.h"

namespace Serialization {
    class LATREN_API BlueprintSerializer :
        public JSONFileSerializer,
        public SerializerItemInterface<std::vector<TypedComponentData>>,
        public Serialization::JSONComponentDeserializer
    {
    using JSONFileSerializer::JSONFileSerializer;
    protected:
        bool ParseJSON() override;
    };
};
