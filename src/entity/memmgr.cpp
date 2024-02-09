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

GeneralComponentReference ComponentMemoryManager::AllocNewComponent(std::type_index t) {
    return GetPool(t).AllocNewComponent();
}

void ComponentMemoryManager::DeleteComponent(std::type_index t, ComponentIndex i) {
    GetPool(t).DeleteComponent(i);
}

void ComponentMemoryManager::ForEachPool(const std::function<void(IComponent&)>& fn) {
    for (const auto&[t, pool] : GetAllPools()) {
        pool->ForEach(fn);
    }
}