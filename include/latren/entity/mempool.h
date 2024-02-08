#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "component.h"

typedef size_t ComponentIndex;
struct ComponentReference;
class IComponentMemoryPool {
public:
    virtual ComponentReference AllocNewComponent(const ComponentData& = ComponentData()) = 0;
    virtual IComponent& GetComponent(ComponentIndex) const = 0;
    template <typename C>
    C& GetComponent(ComponentIndex i) const {
        return dynamic_cast<C&>(GetComponent(i));
    }
};

struct ComponentReference {
    IComponentMemoryPool* pool;
    ComponentIndex index;
    IComponent& GetComponent() const {
        return pool->GetComponent(index);
    };
    operator IComponent&() const {
        return GetComponent();
    }
};

template <typename C>
class ComponentMemoryPool {
private:
    std::vector<C> components_;
    std::unordered_map<ComponentIndex, size_t> references_;
    ComponentIndex currentIndex_ = 0;
public:
    ComponentReference AllocNewComponent(const ComponentData& data = ComponentData()) override {
        components_.push_back(C(data));
        references_[currentIndex_] = components_.size() - 1;
        return { this, currentIndex_++ };
    }
    IComponent& GetComponent(ComponentIndex i) override const {
        return components_.at(references_.at(i));
    }
};

class MasterMemoryPool {
    // optimally all the pools would be stored contiguously but this is the best i can come up with
    // afaik std any doesn't store the memory itself
    std::unordered_map<std::type_index, std::unique_ptr<IComponentMemoryPool>> componentPools_;
};