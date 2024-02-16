#include <latren/io/files/stage.h>
#include <latren/io/fs.h>
#include <latren/io/paths.h>
#include <latren/ec/transform.h>

using namespace Serializer;
using json = nlohmann::json;

bool Serializer::DeserializeComponentDataFromJSON(ComponentData& data, const nlohmann::json& json, const std::string& entityId) {
    VariableMap copyableData;
    for (const auto& [k, v] : json.items()) {
        if (Serializer::ParseJSONComponentData(data, k, v, entityId))
            copyableData.insert({ k, data.vars.at(k) });
        else
            return false;
    }
    data.vars = copyableData;
    return true;
}

std::vector<DeserializedEntity> StageSerializer::ParseEntities(const json& entities, int* invalidEntities) {
    std::vector<DeserializedEntity> parsedEntities;
    for (const auto& [ek, ev] : entities.items()) {
        if (ev.is_object()) {
            DeserializedEntity entity;
            if (ev.contains("id")) {
                if (ev["id"].is_string())
                    entity.id = ev["id"];
            }
            std::string blueprint;
            if (ev.contains("blueprint")) {
                if (ev["blueprint"].is_string())
                    blueprint = ev["blueprint"];
            }
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
                TypedComponentData data = IComponent::CreateComponentData(IComponent::GetComponentType(ck)->type);
                bool success = DeserializeComponentDataFromJSON(data, cv, entity.id);
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
    std::string relativePath = std::fs::proximate(path_, Paths::STAGES_DIR).string();
    if (!data_.contains("id") || !data_["id"].is_string()) {
        spdlog::error("[" + relativePath + "] Missing 'id'!");
        return false;
    }
    stage_.id = data_["id"];
    if (data_.contains("data") && data_["data"].is_object())
        stage_.data = data_["data"];
    int invalidEntities = 0;
    if (data_.contains("entities")) {
        json& entities = data_["entities"];
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