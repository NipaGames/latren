#pragma once

#include <optional>
#include <memory>

#include <latren/latren.h>
#include "mempool.h"
#include "serializable.h"
#include "entity.h"

class IComponent;
struct ComponentTypeData {
    std::string name;
    ComponentType type;
    std::function<IComponent*(const ComponentData&)> componentInitializer;
    std::function<std::unique_ptr<IComponentMemoryPool>()> memPoolInitializer;
};

class LATREN_API IComponent {
protected:
    bool hasStarted_ = false;
    bool useDeleteDestructor_ = false;
public:
    ComponentData data;
    Entity parent;
    IComponentMemoryPool* pool = nullptr;
    
    IComponent() = default;
    IComponent(const IComponent&);
    virtual void IStart() = 0;
    virtual void IDelete() = 0;
    virtual void Delete() = 0;
    virtual void IUpdate() = 0;
    virtual void IFixedUpdate() = 0;
    virtual bool ForwardType(ComponentType) = 0;
    virtual ComponentType GetType() const;
    virtual ComponentData& GetData();
    virtual bool HasStarted() const;
    virtual void UseDeleteDestructor(bool);

    operator EntityIndex() const;
    template <typename C>
    ComponentReference<C> CreateReference() const {
        return { pool, *this };
    }
    GeneralComponentReference CreateReference() const {
        return { pool, *this };
    }
    template <typename C>
    operator ComponentReference<C>() const { return CreateReference<C>(); }
    operator GeneralComponentReference() const { return CreateReference(); }

    template <typename C, typename Return, typename... Params>
    std::function<void(Params...)> LambdaByReference(const std::function<Return(C&, Params...)> lambda) {
        ComponentReference<C> ref = *this;
        if (ref.IsNull()) {
            return [=](Params&&... params) {
                lambda(static_cast<C&>(*this), params...);
            };
        }
        return [=](Params&&... params) mutable {
            lambda(ref, params...);
        };
    }
    template <typename F>
    auto LambdaByReference(F f) {
        return LambdaByReference(std::function(f));
    }

    static double GetTime();
};

template <class Derived>
class Component : public IComponent {
friend class Entity;
private:
    ComponentType type_ = typeid(Derived);
public:
    void IStart() override {
        hasStarted_ = true;
        static_cast<Derived*>(this)->Start();
    }
    void IDelete() override {
        static_cast<Derived*>(this)->Delete();
    }
    void IUpdate() override {
        static_cast<Derived*>(this)->Update();
    }
    void IFixedUpdate() override {
        static_cast<Derived*>(this)->FixedUpdate();
    }
    virtual bool ForwardType(ComponentType t) override {
        type_ = t;
        return true;
    }
    virtual ComponentType GetType() const override {
        return type_;
    }

    template <typename T>
    const T& GetValue(const std::string& key) {
        return data.Get<T>(key);
    }
    template <typename T>
    void SetValue(const std::string& key, T val) {
        data.Set(key, val);
    }
    
    virtual void Start() { }
    virtual void Delete() { }
    virtual void Update() { }
    virtual void FixedUpdate() { }
};