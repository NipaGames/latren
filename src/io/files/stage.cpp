#include <latren/io/files/stage.h>
#include <latren/io/fs.h>
#include <latren/io/paths.h>
#include <latren/ec/transform.h>
#include <latren/ec/serialization.h>
#include <spdlog/spdlog.h>

using namespace Serializer;
using json = nlohmann::json;

bool Serializer::DeserializeComponentDataFromJSON(SerializableFieldValueMap& fields, const nlohmann::json& json, const std::string& entityId) {
    for (const auto& [k, v] : json.items()) {
        if (!ParseJSONComponentData(fields, k, v, entityId))
            return false;
    }
    return true;
}

std::vector<DeserializedEntity> StageSerializer::ParseEntities(const json& entities, int* invalidEntities) {
    std::vector<DeserializedEntity> parsedEntities;
    for (const auto& [ek, ev] : entities.items()) {
        if (ev.is_object()) {
            DeserializedEntity entity;
            if (ev.contains("id") && ev.at("id").is_string())
                entity.id = ev.at("id");
            std::string blueprint;
            if (ev.contains("blueprint") && ev.at("blueprint").is_string())
                blueprint = ev.at("blueprint");
            if (!blueprint.empty() && blueprints_ != nullptr) {
                if (blueprints_->HasItem(blueprint)) {
                    entity.components = blueprints_->GetItem(blueprint);
                }
                else {
                    spdlog::warn("Cannot find blueprint '{}'!", blueprint);
                }
            }
            for (const auto& [ck, cv] : ev.items()) {
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

                bool success = DeserializeComponentDataFromJSON(data.fields, cv, entity.id);
                if (!success) {
                    spdlog::warn("Failed deserializing component '{}'!", ck);
                    // yeah yeah the whole entity isn't necessarily invalid but this will do now
                    if (invalidEntities != nullptr)
                        (*invalidEntities)++;
                }
                entity.components.push_back(data);
            }
            parsedEntities.push_back(entity);
        }
        else if (ev.is_array()) {
            std::vector<DeserializedEntity> recursiveEntities = ParseEntities(ev, invalidEntities);
            parsedEntities.insert(parsedEntities.end(), recursiveEntities.begin(), recursiveEntities.end());
        }
        else if (invalidEntities != nullptr) {
            (*invalidEntities)++;
        }
    }
    return parsedEntities;
}

bool StageSerializer::ParseJSON() {
    std::string relativePath = std::fs::proximate(path_, ResourcePath(Paths::RESOURCE_DIRS.at(Resources::ResourceType::STAGE)).GetParsedPath()).string();
    if (!data_.contains("id") || !data_.at("id").is_string()) {
        spdlog::error("[" + relativePath + "] Missing 'id'!");
        return false;
    }
    stage_.id = data_.at("id");
    if (data_.contains("data") && data_.at("data").is_object())
        stage_.data = data_.at("data");
    int invalidEntities = 0;
    if (data_.contains("entities")) {
        const json& entities = data_.at("entities");
        if (entities.is_array()) {
            stage_.entities = ParseEntities(entities, &invalidEntities);
        }
        else
            spdlog::warn("[" + relativePath + "] 'entities' must be an array!");
    }
    if (invalidEntities > 0) {
        spdlog::warn("[" + relativePath + "] " + std::to_string(invalidEntities) + " invalid entities!");
    }
    return true;
}

Stage& StageSerializer::GetStage() {
    return stage_;
}