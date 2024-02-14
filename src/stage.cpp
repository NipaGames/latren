#include <latren/stage.h>
#include <latren/game.h>
#include <latren/entity/entitymanager.h>
#include <latren/io/files/stage.h>
#include <latren/io/resourcemanager.h>

#include <fstream>
#include <spdlog/spdlog.h>

std::optional<Stage> Resources::StageManager::LoadResource(const std::fs::path& path) {
    Serializer::StageSerializer serializer;
    serializer.UseBlueprints(blueprints_);
    serializer.DeserializeFile(path.generic_string());
    if (!serializer.Success())
        return std::nullopt;
    Stage s = serializer.GetStage();
    if (s.id.empty())
        return std::nullopt;
    SetItemID(s.id);
    return s;
}

void CloneComponentValuesFromData(IComponent& c, const TypedComponentData& data) {
    for (const auto& [k, v] : data.vars) {
        v->CloneValuesTo(c.data.vars[k]);
    }
}

bool Resources::StageManager::LoadStage(const std::string& id) {
    if (items_.empty())
        return false;
    if (items_.find(id) == items_.end())
        return false;
    Stage& s = items_.at(id);
    s.instantiatedEntities.clear();

    std::vector<GeneralComponentReference> newComponents;
    EntityManager& entityManager = Game::GetGameInstanceBase()->GetEntityManager();
    for (const DeserializedEntity& e : s.entities) {
        Entity entity;
        if (entityManager.HasNamedEntity(e.id)) {
            entity = entityManager.GetNamedEntity(e.id);
        }
        else {
            entity = entityManager.CreateEntity(e.id);
            s.instantiatedEntities.insert(entity);
        }
        for (const auto& data : e.components) {
            if (!entity.HasComponent(data.type)) {
                newComponents.push_back(entity.AddComponent(data.type));
            }
            IComponent& c = entity.GetComponent(data.type);
            CloneComponentValuesFromData(c, data);
        }
    }
    loadedStages_.insert(loadedStages_.begin(), s.id);
    spdlog::info("Loaded stage '" + id + "' (" + std::to_string(s.entities.size()) + " entities modified)");
    for (GeneralComponentReference& c : newComponents) {
        c->IStart();
    }
    int i = 0;
    Game::GetGameInstanceBase()->GetEntityManager().GetComponentMemory().ForEachComponent<Transform>([&](Transform& c) {
        i++;
    });
    std::cout << i << std::endl;
    Game::GetGameInstanceBase()->GetRenderer().UpdateLighting();
    Game::GetGameInstanceBase()->GetRenderer().UpdateFrustum();
    return true;
}

bool Resources::StageManager::UnloadStage(const std::string& id) {
    auto idIt = std::find(loadedStages_.begin(), loadedStages_.end(), id);
    if (idIt == loadedStages_.end())
        return false;
    const Stage& s = items_.at(id);
    for (EntityIndex entity : s.instantiatedEntities) {
        Game::GetGameInstanceBase()->GetEntityManager().DestroyEntity(entity);
    }
    loadedStages_.erase(idIt);
    Game::GetGameInstanceBase()->GetRenderer().UpdateLighting();
    Game::GetGameInstanceBase()->GetRenderer().UpdateFrustum();
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

void Resources::StageManager::UseBlueprints(Serializer::BlueprintSerializer* blueprints) {
    blueprints_ = blueprints;
}