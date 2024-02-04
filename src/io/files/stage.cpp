#include <latren/io/files/stage.h>
#include <latren/io/fs.h>
#include <latren/io/paths.h>
#include <latren/entity/transform.h>

using namespace Serializer;
using json = nlohmann::json;

bool Serializer::DeserializeComponentFromJSON(IComponent* c, const nlohmann::json& json, const std::string& entityId) {
    if (c == nullptr)
        return false;
    VariableMap copyableData;
    for (const auto&[k, v] : json.items()) {
        if (Serializer::SetJSONComponentValue(c, k, v, entityId))
            copyableData.insert({ k, c->data.vars.at(k) });
        else
            return false;
    }
    c->data.vars = copyableData;
    return true;
}

std::list<Entity> StageSerializer::ParseEntities(const json& entities, int* invalidEntities) {
    std::list<Entity> parsedEntities;
    for (const auto&[ek, ev] : entities.items()) {
        if (ev.is_object()) {
            Entity entity("");
            entity.RemoveComponent<Transform>();
            entity.transform = nullptr;
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
                    const std::vector<IComponent*>& blueprintComponents = blueprints_->GetItem(blueprint);
                    for (const IComponent* c : blueprintComponents) {
                        IComponent* entityComponent = entity.GetComponent(c->typeHash);
                        if (entityComponent == nullptr)
                            entityComponent = entity.AddComponent(c->typeHash);
                        for (const auto& [varName, varValue] : c->data.vars) {
                            varValue->CloneValuesTo(entityComponent->data.GetComponentDataValue(varName));
                        }
                    }
                }
                else {
                    spdlog::warn("Cannot find blueprint '{}'!", blueprint);
                }
            }
            for (const auto&[ck, cv] : ev.items()) {
                if (!cv.is_object())
                    continue;
                IComponent* c = entity.GetComponent(ck);
                
                if (c == nullptr) {
                    c = entity.AddComponent(ck);
                }
                
                bool success = DeserializeComponentFromJSON(c, cv, entity.id);
                if (!success) {
                    spdlog::warn("Failed deserializing component '{}'!", ck);
                    // yeah yeah the whole entity isn't necessarily invalid but this will do now
                    if (invalidEntities != nullptr)
                        (*invalidEntities)++;
                }
            }
            parsedEntities.push_back(entity);
        }
        else if (ev.is_array()) {
            std::list<Entity> recursiveEntities = ParseEntities(ev, invalidEntities);
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