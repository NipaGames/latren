#include <latren/io/files/stage.h>
#include <latren/io/fs.h>
#include <latren/io/paths.h>
#include <latren/ec/transform.h>
#include <latren/ec/serialization.h>
#include <spdlog/spdlog.h>

using namespace Serialization;
using json = nlohmann::json;

bool StageSerializer::ParseEntityBlueprints(DeserializedEntity& entity, const std::string& blueprint) {
    if (blueprint.empty() || blueprints_ == nullptr)
        return false;
    if (!blueprints_->HasItem(blueprint)) {
        spdlog::warn("Cannot find blueprint '{}'!", blueprint);
        return false;
    }
    entity.components = blueprints_->GetItem(blueprint);
    return false;
}

std::vector<DeserializedEntity> StageSerializer::ParseEntities(const json& entities, int* invalidEntities) {
    std::vector<DeserializedEntity> parsedEntities;
    for (const auto& [ek, ev] : entities.items()) {
        if (ev.is_object()) {
            DeserializedEntity entity;
            if (ev.contains("id") && ev.at("id").is_string())
                entity.id = ev.at("id");
            if (ev.contains("blueprint") && ev.at("blueprint").is_string())
                ParseEntityBlueprints(entity, ev.at("blueprint"));
            if (!DeserializeComponents(entity.components, ev, entity.id))
                (*invalidEntities)++;
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
    stage_ = Stage();
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