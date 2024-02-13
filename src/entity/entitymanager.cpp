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

Entity EntityManager::CreateEntity() {
    Entity e = Entity(this, NextID());
    e.AddComponent<Transform>();
    return e;
}

GeneralComponentReference EntityManager::AddComponent(EntityIndex entity, std::type_index type) {
    return componentMemoryManager_.AllocNewComponent(entity, type);
}

void EntityManager::DeleteComponent(EntityIndex entity, std::type_index type) {
    componentMemoryManager_.DeleteComponent(entity, type);
}

void EntityManager::ClearEverything() {
    componentMemoryManager_.ForEachPool([](IComponentMemoryPool& pool) {
        pool.ClearAllComponents();
    });
}

size_t EntityManager::GetTotalPoolBytes() {
    size_t total = 0;
    componentMemoryManager_.ForEachPool([&total](IComponentMemoryPool& pool) {
        total += pool.GetTotalBytes();
    });
    return total;
}