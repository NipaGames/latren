#pragma once

#include "memmgr.h"
#include <latren/util/idfactory.h>
#include <unordered_set>

class Entity;
struct GlobalEntityData {
    std::string name;
    std::unordered_set<ComponentType> components;
};

class LATREN_API EntityManager : public IDFactory<EntityIndex> {
friend class Game;
private:
    ComponentMemoryManager componentMemoryManager_;
    // i'm not actually sure if the names of destroyed components should be dynamically removed
    // at the moment they're not
    std::unordered_map<std::string, EntityIndex> entityNames_;
    // these however are dynamically managed (faster entityindex lookup)
    std::unordered_map<EntityIndex, GlobalEntityData> entityData_;
public:
    void Setup();
    void StartAll();
    void UpdateAll();
    void FixedUpdateAll();
    Entity GetNamedEntity(const std::string&);
    bool HasNamedEntity(const std::string&);
    const GlobalEntityData& GetEntityData(EntityIndex);
    ComponentMemoryManager& GetComponentMemory();
    Entity CreateEntity(const std::string& = "");
    GeneralComponentReference AddComponent(EntityIndex, ComponentType);
    IComponent& GetComponent(EntityIndex, ComponentType);
    IComponentMemoryPool& GetComponentPool(ComponentType);
    template <typename C>
    ComponentMemoryPool<C>& GetComponentPool() {
        return GetComponentMemory().GetPool<C>();
    }
    template <typename C>
    C& GetComponent(EntityIndex entity) {
        return GetComponentPool<C>().GetComponent(entity);
    }
    void DestroyComponent(EntityIndex, ComponentType);
    void DestroyEntity(EntityIndex);
    void ClearEverything();
    std::size_t GetTotalPoolBytes();
};