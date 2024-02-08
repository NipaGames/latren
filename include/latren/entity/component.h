#pragma once

#include <optional>
#include <iostream>
#include <spdlog/spdlog.h>

#include <latren/latren.h>
#include "serializable.h"

class IComponent;
struct ComponentType {
    std::string name;
    std::type_index type;
    IComponent*(*initializer)(const ComponentData&);
};

class Entity;
class IForwardComponent;
class LATREN_API IComponent {
friend class Entity;
private:
    bool hasStarted_ = false;
    bool hasHadFirstUpdate_ = false;
public:
    ComponentData data;
    Entity* parent;
    virtual IComponent* Clone() const = 0;
    virtual ~IComponent() { }
    IComponent() { }
    IComponent(const IComponent&);
    virtual void IStart() = 0;
    virtual void IFirstUpdate() = 0;
    virtual void IUpdate() = 0;
    virtual void IFixedUpdate() = 0;
    virtual bool ForwardType(const std::type_index&, const std::function<IComponent*(const IComponent*)>&) = 0;
    virtual std::type_index GetType() const { return typeid(IComponent); }

    template <typename C>
    static IComponent* CreateInstance(const ComponentData& data) {
        IComponent* c = new C();
        for (const auto&[k, v] : data.vars) {
            v->CloneValuesTo(c->data.vars[k]);
        }
        return c;
    }

    static std::optional<ComponentType> GetComponentType(const std::string&);
    static std::optional<ComponentType> GetComponentType(const std::type_index&);
    static std::optional<std::string> GetComponentName(const std::type_index&);

    static IComponent* CreateComponent(const std::type_index&, const ComponentData& = ComponentData());
    static IComponent* CreateComponent(const std::string&, const ComponentData& = ComponentData());

    static bool RegisterComponent(const type_info&, IComponent*(*)(const ComponentData&));
    template <typename C>
    static bool RegisterComponent() {
        return RegisterComponent(typeid(C), CreateInstance<C>);
    }
};

template <class Component>
class RegisterComponent {
private:
    inline static bool _isRegistered = IComponent::RegisterComponent<Component>();
};

template <class Derived>
class Component : public IComponent, public RegisterComponent<Derived> {
friend class Entity;
private:
    std::function<IComponent*(const IComponent*)> cloneFn_ = [](const IComponent* c) {
        return new Derived(dynamic_cast<const Derived&>(*c));
    };
    std::type_index type_ = typeid(Derived);
public:
    virtual ~Component() = default;
    virtual IComponent* Clone() const override {
        return cloneFn_(this);
    }
    void IStart() override { dynamic_cast<Derived*>(this)->Start(); }
    void IUpdate() override { dynamic_cast<Derived*>(this)->Update(); }
    void IFirstUpdate() override { dynamic_cast<Derived*>(this)->FirstUpdate(); }
    void IFixedUpdate() override { dynamic_cast<Derived*>(this)->FixedUpdate(); }
    virtual bool ForwardType(const std::type_index& t, const std::function<IComponent*(const IComponent*)>& clone) override {
        type_ = t;
        cloneFn_ = clone;
        return true;
    }
    virtual std::type_index GetType() const override {
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
    virtual void Update() { }
    virtual void FixedUpdate() { }
    virtual void FirstUpdate() { }
};