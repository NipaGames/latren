#include <latren/io/files/blueprints.h>
#include <latren/io/files/stage.h>
#include <latren/ec/component.h>
#include <latren/ec/serialization.h>
#include <spdlog/spdlog.h>

using namespace Serializer;

bool BlueprintSerializer::ParseJSON() {
    if (!data_.is_object()) {
        spdlog::error("Blueprints must be an object!");
        return false;
    }
    for (const auto& [id, componentsJson] : data_.items()) {
        std::vector<TypedComponentData> components;
        if (!componentsJson.is_object()) {
            spdlog::error("A blueprint must be an object!");
            continue;
        }
        for (const auto& [ck, cv] : componentsJson.items()) {
            if (!cv.is_object())
                continue;
            
            SerializableFieldValueMap map;
            const ComponentTypeData& typeData = ComponentSerialization::GetComponentType(ck);
            for (const auto& [name, field] : typeData.serializableFields) {
                map.insert({ name, { nullptr, field.type, field.containerType } });
            }
            TypedComponentData data {
                typeData.type,
                map
            };

            bool success = DeserializeComponentDataFromJSON(data.fields, cv);
            if (!success) {
                spdlog::warn("Failed deserializing component '{}' for a blueprint!", ck);
                continue;
            }
            components.push_back(data);
        }
        items_.insert({ id, components });
    }
    return true;
}