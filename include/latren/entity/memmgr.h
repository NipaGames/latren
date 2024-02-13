#pragma once

#include <latren/latren.h>
#include "mempool.h"

class ComponentMemoryManager {
private:
    // optimally all the pools would be stored contiguously but this is the best i can come up with
    // afaik std any doesn't store the memory itself
    ComponentPoolContainer componentPools_;
public:
    LATREN_API void MovePools(ComponentPoolContainer&&);
    LATREN_API IComponentMemoryPool& GetPool(std::type_index);
    template <typename C>
    ComponentMemoryPool<C>& GetPool() {
        return static_cast<ComponentMemoryPool<C>&>(GetPool(typeid(C)));
    }
    LATREN_API const ComponentPoolContainer& GetAllPools();
    LATREN_API GeneralComponentReference AllocNewComponent(EntityIndex, std::type_index);
    template <typename C>
    ComponentReference<C> AllocNewComponent(EntityIndex entity) {
        return static_cast<const ComponentReference<C>&>(AllocNewComponent(entity, typeid(C)));
    }
    LATREN_API void DeleteComponent(EntityIndex, std::type_index);
    template <typename C>
    void DeleteComponent(EntityIndex entity) {
        DeleteComponent(entity, typeid(C));
    }
    template <typename C>
    void ForEachComponent(const std::function<void(C&)>& fn) {
        GetPool<C>().ForEach(fn);
    }
    LATREN_API void ForEachPool(const std::function<void(IComponentMemoryPool&)>&);
    LATREN_API void ForAllComponents(const std::function<void(IComponent&)>&);
    template <typename C>
    void ForEachDerivedComponent(const std::function<void(C&, IComponentMemoryPool&)>& fn) {
        ForEachPool([&](IComponentMemoryPool& pool) {
            if (pool.CanCastComponentsTo<C>()) {
                pool.ForEach([&](IComponent& c) {
                    fn(dynamic_cast<C&>(c), pool);
                });
            }
        });
    }
};