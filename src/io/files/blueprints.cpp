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
            
            TypedComponentData data = ComponentSerialization::CreateComponentData(ComponentSerialization::GetComponentType(ck)->type);
            bool success = DeserializeComponentDataFromJSON(data, cv);
            if (!success) {
                spdlog::warn("Failed deserializing component '{}' for a blueprint!", ck);
            }
            components.push_back(data);
        }
        items_.insert({ id, components });
    }
    return true;
}