#include <latren/stage.h>
#include <latren/systems.h>
#include <latren/ec/entitymanager.h>
#include <latren/ec/serialization.h>
#include <latren/io/files/stage.h>
#include <latren/io/resourcemanager.h>
#include <latren/graphics/renderer.h>

#include <fstream>
#include <spdlog/spdlog.h>

std::optional<Stage> Resources::StageManager::LoadResource(const ResourcePath& path) {
    stageSerializer_->DeserializeFile(path.GetParsedPathStr());
    if (!stageSerializer_->Success())
        return std::nullopt;
    Stage s = stageSerializer_->GetStage();
    if (s.id.empty())
        return std::nullopt;
    SetItemID(s.id);
    return s;
}

bool Resources::StageManager::LoadStage(const std::string& id) {
    if (items_.empty())
        return false;
    if (items_.find(id) == items_.end())
        return false;
    Stage& s = items_.at(id);
    s.instantiatedEntities.clear();

    std::vector<GeneralComponentReference> newComponents;
    EntityManager& entityManager = Systems::GetEntityManager();
    for (const DeserializedEntity& e : s.entities) {
        Entity entity;
        if (entityManager.HasNamedEntity(e.id)) {
            entity = entityManager.GetNamedEntity(e.id);
        }
        else {
            entity = entityManager.CreateEntity(e.id);
            s.instantiatedEntities.insert(entity);
        }
        for (const auto& c : e.components) {
            if (!entity.HasComponent(c.type)) {
                newComponents.push_back(entity.AddComponent(c.type));
            }
            IComponent& instance = entity.GetComponent(c.type);
            const SerializableFieldMap& componentFields = ComponentSerialization::GetComponentType(instance.GetType()).serializableFields;
            for (const auto& [name, field] : c.fields) {
                if (field.value == nullptr || componentFields.count(name) == 0)
                    continue;
                field.value->CopyValueTo((char*) &instance + componentFields.at(name).offset);
            }
        }
    }
    loadedStages_.insert(loadedStages_.begin(), s.id);
    spdlog::info("Loaded stage '" + id + "' (" + std::to_string(s.entities.size()) + " entities modified)");
    for (GeneralComponentReference& c : newComponents) {
        c->IStart();
    }
    Systems::GetRenderer().UpdateLighting();
    Systems::GetRenderer().UpdateFrustum();
    return true;
}

bool Resources::StageManager::UnloadStage(const std::string& id) {
    auto idIt = std::find(loadedStages_.begin(), loadedStages_.end(), id);
    if (idIt == loadedStages_.end())
        return false;
    const Stage& s = items_.at(id);
    for (EntityIndex entity : s.instantiatedEntities) {
        Systems::GetEntityManager().DestroyEntity(entity);
    }
    loadedStages_.erase(idIt);
    Systems::GetRenderer().UpdateLighting();
    Systems::GetRenderer().UpdateFrustum();
    return true;
}

void Resources::StageManager::UnloadAllStages() {
    for (int i = 0; i < loadedStages_.size(); i++) {
        UnloadStage(loadedStages_.at(i--));
    }
}

const std::vector<std::string>& Resources::StageManager::GetLoadedStages() {
    return loadedStages_;
}

void Resources::StageManager::UseBlueprints(Serialization::BlueprintSerializer* blueprints) {
    stageSerializer_->UseBlueprints(blueprints);
}