#pragma once

#include "../serializer.h"

namespace Serializer {
    class BlueprintSerializer : public JSONFileSerializer, public SerializerItemInterface<std::vector<IComponent*>> {
    using JSONFileSerializer::JSONFileSerializer;
    protected:
        LATREN_API bool ParseJSON() override;
    public:
        LATREN_API ~BlueprintSerializer();
    };
};
