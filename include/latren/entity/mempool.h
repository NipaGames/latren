#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <typeindex>
#include <latren/util/idfactory.h>

typedef size_t ComponentIndex;

class IComponent;
struct GeneralComponentReference;
class IComponentMemoryPool {
public:
    virtual GeneralComponentReference AllocNewComponent() = 0;
    virtual void DeleteComponent(ComponentIndex) = 0;
    virtual IComponent& GetComponent(ComponentIndex) = 0;
    template <typename C>
    C& GetComponent(ComponentIndex i) {
        return static_cast<C&>(GetComponent(i));
    }
};
typedef std::unordered_map<std::type_index, std::unique_ptr<IComponentMemoryPool>> ComponentPoolContainer;

struct GeneralComponentReference {
    IComponentMemoryPool* pool;
    ComponentIndex index;
    IComponent& GetComponent() const {
        return pool->GetComponent(index);
    };
    operator IComponent&() const { return GetComponent(); }
    IComponent* operator->() const { return &GetComponent(); }
};

template <typename C, typename = std::enable_if_t<!std::is_same_v<C, IComponent>>>
struct ComponentReference : public GeneralComponentReference {
    C& GetComponent() const {
        return pool->GetComponent<C>(index);
    };
    operator C&() const { return GetComponent(); }
    C* operator->() const { return &GetComponent(); }
};

template <typename C, typename = std::enable_if_t<!std::is_same_v<C, IComponent>>>
class ComponentMemoryPool : public IComponentMemoryPool, public IDFactory {
private:
    std::vector<C> components_;
    std::unordered_map<ComponentIndex, size_t> references_;
    ComponentIndex currentIndex_ = 0;
public:
    GeneralComponentReference AllocNewComponent() override {
        C& c = components_.emplace_back(C());
        ComponentIndex i = NextID();
        references_[i] = components_.size() - 1;
        return ComponentReference<C> { this, i };
    }
    void DeleteComponent(ComponentIndex i) override {
        if (references_.count(i) == 0)
            return;
        size_t pos = references_.at(i);
        if (pos >= components_.size())
            return;
        components_.erase(components_.begin() + pos);
        references_.erase(i);
        for (auto&[k, v] : references_) {
            if (v > pos)
                v--;
        }
    }
    IComponent& GetComponent(ComponentIndex i) override {
        return components_.at(references_.at(i));
    }
    void ForEach(std::function<void(C&)> fn) {
        std::for_each(components_.begin(), components_.end(), fn);
    }
};