#include <latren/entity/entitymanager.h>
#include <latren/entity/transform.h>

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
    e.AddComponent<Transform>();
    if (!name.empty()) {
        entityNames_[name] = e;
    }
    entityData_[e] = {
        name
    };
    return e;
}

Entity EntityManager::GetNamedEntity(const std::string& name) {
    return Entity(this, entityNames_.at(name));
}

const GlobalEntityData& EntityManager::GetEntityData(EntityIndex entity) {
    return entityData_.at(entity);
}

GeneralComponentReference EntityManager::AddComponent(EntityIndex entity, std::type_index type) {
    return componentMemoryManager_.AllocNewComponent(entity, type);
}

void EntityManager::DeleteComponent(EntityIndex entity, std::type_index type) {
    componentMemoryManager_.DeleteComponent(entity, type);
    entityData_.erase(entity);
}

void EntityManager::ClearEverything() {
    componentMemoryManager_.ForEachPool([](IComponentMemoryPool& pool) {
        pool.ClearAllComponents();
    });
    entityNames_.clear();
}

size_t EntityManager::GetTotalPoolBytes() {
    size_t total = 0;
    componentMemoryManager_.ForEachPool([&total](IComponentMemoryPool& pool) {
        total += pool.GetTotalBytes();
    });
    return total;
}