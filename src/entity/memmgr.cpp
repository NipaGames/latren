#include <latren/entity/memmgr.h>

void ComponentMemoryManager::MovePools(ComponentPoolContainer&& pools) {
    componentPools_ = std::move(pools);
}

IComponentMemoryPool& ComponentMemoryManager::GetPool(std::type_index t) {
    return *componentPools_.at(t).get();
}

const ComponentPoolContainer& ComponentMemoryManager::GetAllPools() {
    return componentPools_;
}

GeneralComponentReference ComponentMemoryManager::AllocNewComponent(EntityIndex entity, std::type_index t) {
    return GetPool(t).AllocNewComponent(entity);
}

void ComponentMemoryManager::DeleteComponent(EntityIndex entity, std::type_index t) {
    GetPool(t).DeleteComponent(entity);
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