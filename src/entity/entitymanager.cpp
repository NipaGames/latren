#include <latren/entity/entitymanager.h>
#include <latren/entity/transform.h>
#include <latren/entity/entity.h>

void EntityManager::Setup() {
    componentMemoryManager_.MovePools(IComponent::CreateComponentMemoryPools());
}

void EntityManager::StartAll() {
    componentMemoryManager_.ForAllComponents([](IComponent& c) {
        c.IStart();
    });
}
void EntityManager::UpdateAll() {
    componentMemoryManager_.ForAllComponents([](IComponent& c) {
        c.IUpdate();
    });
}

void EntityManager::FixedUpdateAll() {
    componentMemoryManager_.ForAllComponents([](IComponent& c) {
        c.IFixedUpdate();
    });
}

ComponentMemoryManager& EntityManager::GetComponentMemory() {
    return componentMemoryManager_;
}

Entity EntityManager::CreateEntity(const std::string& name) {
    Entity e = Entity(this, NextID());
    entityData_[e] = {
        name,
        { }
    };
    if (!name.empty()) {
        entityNames_[name] = e;
    }
    e.AddComponent<Transform>();
    return e;
}

Entity EntityManager::GetNamedEntity(const std::string& name) {
    return Entity(this, entityNames_.at(name));
}

const GlobalEntityData& EntityManager::GetEntityData(EntityIndex entity) {
    return entityData_.at(entity);
}

GeneralComponentReference EntityManager::AddComponent(EntityIndex entity, ComponentType type) {
    GeneralComponentReference ref = componentMemoryManager_.AllocNewComponent(entity, type);
    entityData_.at(entity).components.insert(type);
    return ref;
}

IComponent& EntityManager::GetComponent(EntityIndex entity, ComponentType type) {
    return componentMemoryManager_.GetPool(type).GetComponentBase(entity);
}

IComponentMemoryPool& EntityManager::GetComponentPool(ComponentType type) {
    return componentMemoryManager_.GetPool(type);
}

void EntityManager::DestroyComponent(EntityIndex entity, ComponentType type) {
    componentMemoryManager_.DestroyComponent(entity, type);
    entityData_.at(entity).components.erase(type);
}

void EntityManager::DestroyEntity(EntityIndex entity) {
    auto it = entityData_.find(entity);
    if (it == entityData_.end())
        return;
    std::unordered_set<ComponentType> components = it->second.components;
    for (ComponentType type : components) {
        DestroyComponent(entity, type);
    }
    entityData_.erase(entity);
}

void EntityManager::ClearEverything() {
    componentMemoryManager_.ForEachPool([](IComponentMemoryPool& pool) {
        pool.ClearAllComponents();
    });
    entityData_.clear();
    entityNames_.clear();
}

size_t EntityManager::GetTotalPoolBytes() {
    size_t total = 0;
    componentMemoryManager_.ForEachPool([&total](IComponentMemoryPool& pool) {
        total += pool.GetTotalBytes();
    });
    return total;
}