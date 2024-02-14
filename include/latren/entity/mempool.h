#pragma once

#include <algorithm>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <typeindex>

typedef size_t EntityIndex;
typedef std::type_index ComponentType;

class IComponent;
template <typename C>
using VerifyComponent = std::enable_if_t<std::is_base_of_v<IComponent, C>>;
template <typename C>
using VerifyNonVirtualComponent = std::enable_if_t<std::is_base_of_v<IComponent, C> && !std::is_same_v<C, IComponent>>;

struct GeneralComponentReference;
class IComponentMemoryPool {
protected:
    virtual IComponent* GetFirstComponent() = 0;
public:
    virtual GeneralComponentReference AllocNewComponent(EntityIndex) = 0;
    virtual void DestroyComponent(EntityIndex) = 0;
    virtual void ClearAllComponents() = 0;
    virtual IComponent& GetComponentBase(EntityIndex) = 0;
    template <typename C, typename = VerifyComponent<C>>
    C& GetComponent(EntityIndex i) {
        return static_cast<C&>(GetComponentBase(i));
    }
    template <typename C>
    bool CanCastComponentsTo() {
        return dynamic_cast<C*>(GetFirstComponent()) != nullptr;
    }
    virtual void ForEach(const std::function<void(IComponent&)>&) = 0;
    virtual size_t GetAllocatedBytes() const = 0;
    virtual size_t GetReferenceOverheadBytes() const = 0;
    size_t GetTotalBytes() { return GetAllocatedBytes() + GetReferenceOverheadBytes(); }
};
typedef std::unordered_map<ComponentType, std::unique_ptr<IComponentMemoryPool>> ComponentPoolContainer;

struct GeneralComponentReference {
    IComponentMemoryPool* pool;
    EntityIndex index;
    IComponent& GetComponentBase() {
        return pool->GetComponentBase(index);
    };
    template <typename C>
    C& CastComponent() {
        return dynamic_cast<C&>(pool->GetComponentBase(index));
    };
    operator IComponent&() { return GetComponentBase(); }
    IComponent* operator->() { return &GetComponentBase(); }
    void Delete() {
        pool->DestroyComponent(index);
    }
    bool operator==(const GeneralComponentReference& cmp) const {
        return (pool == cmp.pool) && (index == cmp.index);
    }
    bool operator!=(const GeneralComponentReference& cmp) const { return !operator==(cmp); }
};

template <typename C, typename = VerifyNonVirtualComponent<C>>
struct ComponentReference : public GeneralComponentReference {
    C& GetComponent() {
        return pool->GetComponent<C>(index);
    };
    operator C&() { return GetComponent(); }
    C* operator->() { return &GetComponent(); }
};

template <typename C, typename = VerifyNonVirtualComponent<C>>
class ComponentMemoryPool : public IComponentMemoryPool {
private:
    std::vector<C> components_;
    std::unordered_map<EntityIndex, size_t> references_;
protected:
    IComponent* GetFirstComponent() override {
        if (components_.empty())
            return nullptr;
        return &components_.front();
    }
public:
    GeneralComponentReference AllocNewComponent(EntityIndex entity) override {
        C& c = components_.emplace_back();
        c.pool = this;
        references_[entity] = components_.size() - 1;
        return ComponentReference<C> { this, entity };
    }
    void DestroyComponent(EntityIndex entity) override {
        if (references_.find(entity) == references_.end())
            return;
        size_t pos = references_.at(entity);
        if (pos >= components_.size())
            return;
        components_.erase(components_.begin() + pos);
        references_.erase(entity);
        for (auto& [k, v] : references_) {
            if (v > pos)
                v--;
        }
    }
    void ClearAllComponents() override {
        references_.clear();
        components_.clear();
    }
    IComponent& GetComponentBase(EntityIndex entity) override {
        return components_.at(references_.at(entity));
    }
    C& GetComponent(EntityIndex entity) {
        return static_cast<C&>(GetComponentBase(entity));
    }
    void ForEach(const std::function<void(C&)>& fn) {
        std::for_each(components_.begin(), components_.end(), fn);
    }
    void ForEach(const std::function<void(IComponent&)>& fn) override {
        ForEach(static_cast<const std::function<void(C&)>&>(fn));
    }
    // todo implement
    size_t GetReferenceOverheadBytes() const override {
        return 0;
    }
    size_t GetAllocatedBytes() const override {
        return sizeof(std::vector<C>) + components_.size() * sizeof(C);
    }
};