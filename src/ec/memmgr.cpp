#include <latren/ec/memmgr.h>

void ComponentMemoryManager::MovePools(ComponentPoolContainer&& pools) {
    componentPools_ = std::move(pools);
}

IComponentMemoryPool& ComponentMemoryManager::GetPool(ComponentType t) {
    return *componentPools_.at(t).get();
}

const ComponentPoolContainer& ComponentMemoryManager::GetAllPools() {
    return componentPools_;
}

GeneralComponentReference ComponentMemoryManager::AllocNewComponent(EntityIndex entity, ComponentType t) {
    return GetPool(t).AllocNewComponent(entity);
}

void ComponentMemoryManager::DestroyComponent(EntityIndex entity, ComponentType t) {
    GetPool(t).DestroyComponent(entity);
}

void ComponentMemoryManager::ForEachPool(const std::function<void(IComponentMemoryPool&)>& fn) {
    for (const auto& [t, pool] : GetAllPools()) {
        fn(*pool);
    }
}

void ComponentMemoryManager::ForAllComponents(const std::function<void(IComponent&)>& fn) {
    ForEachPool([fn](IComponentMemoryPool& pool) {
        pool.ForEach(fn);
    });
}