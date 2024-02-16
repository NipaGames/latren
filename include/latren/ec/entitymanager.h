#pragma once

#include "memmgr.h"
#include <latren/util/idfactory.h>
#include <unordered_set>

class Entity;
struct GlobalEntityData {
    std::string name;
    std::unordered_set<ComponentType> components;
};

class EntityManager : public IDFactory<EntityIndex> {
friend class Game;
private:
    ComponentMemoryManager componentMemoryManager_;
    // i'm not actually sure if the names of destroyed components should be dynamically removed
    // at the moment they're not
    std::unordered_map<std::string, EntityIndex> entityNames_;
    // these however are dynamically managed (faster entityindex lookup)
    std::unordered_map<EntityIndex, GlobalEntityData> entityData_;
public:
    LATREN_API void Setup();
    LATREN_API void StartAll();
    LATREN_API void UpdateAll();
    LATREN_API void FixedUpdateAll();
    LATREN_API Entity GetNamedEntity(const std::string&);
    LATREN_API bool HasNamedEntity(const std::string&);
    LATREN_API const GlobalEntityData& GetEntityData(EntityIndex);
    LATREN_API ComponentMemoryManager& GetComponentMemory();
    LATREN_API Entity CreateEntity(const std::string& = "");
    LATREN_API GeneralComponentReference AddComponent(EntityIndex, ComponentType);
    LATREN_API IComponent& GetComponent(EntityIndex, ComponentType);
    LATREN_API IComponentMemoryPool& GetComponentPool(ComponentType);
    template <typename C>
    ComponentMemoryPool<C>& GetComponentPool() {
        return GetComponentMemory().GetPool<C>();
    }
    template <typename C>
    C& GetComponent(EntityIndex entity) {
        return GetComponentPool<C>().GetComponent(entity);
    }
    LATREN_API void DestroyComponent(EntityIndex, ComponentType);
    LATREN_API void DestroyEntity(EntityIndex);
    LATREN_API void ClearEverything();
    LATREN_API size_t GetTotalPoolBytes();
};