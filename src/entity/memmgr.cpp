#include <latren/entity/memmgr.h>

void ComponentMemoryManager::MovePools(ComponentPoolContainer&& pools) {
    componentPools_ = std::move(pools);
}

GeneralComponentReference ComponentMemoryManager::AllocNewComponent(std::type_index t) {
    return componentPools_.at(t)->AllocNewComponent();
}

void ComponentMemoryManager::DeleteComponent(std::type_index t, ComponentIndex i) {
    componentPools_.at(t)->DeleteComponent(i);
}