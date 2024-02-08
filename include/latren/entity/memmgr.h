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
    LATREN_API GeneralComponentReference AllocNewComponent(std::type_index);
    template <typename C>
    ComponentReference<C> AllocNewComponent() {
        return static_cast<const ComponentReference<C>&>(AllocNewComponent(typeid(C)));
    }
    LATREN_API void DeleteComponent(std::type_index, ComponentIndex);
    template <typename C>
    void DeleteComponent(ComponentIndex i) {
        DeleteComponent(typeid(C), i);
    }
};