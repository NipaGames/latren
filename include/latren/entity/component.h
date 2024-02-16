#pragma once

#include <optional>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

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

class IForwardComponent;
class LATREN_API IComponent {
protected:
    bool hasStarted_ = false;
    bool useDeleteDestructor_ = false;
public:
    ComponentData data;
    Entity parent;
    IComponentMemoryPool* pool;
    
    IComponent() { }
    IComponent(const IComponent&);
    virtual void IStart() = 0;
    virtual void IDelete() = 0;
    virtual void IUpdate() = 0;
    virtual void IFixedUpdate() = 0;
    virtual bool ForwardType(ComponentType) = 0;
    virtual ComponentType GetType() const { return typeid(IComponent); }
    virtual ComponentData& GetData() { return data; }
    virtual bool HasStarted() const { return hasStarted_; }
    virtual void UseDeleteDestructor(bool use) { useDeleteDestructor_ = use; }

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

    static std::optional<ComponentTypeData> GetComponentType(const std::string&);
    static std::optional<ComponentTypeData> GetComponentType(ComponentType);
    template <typename T>
    static std::optional<ComponentTypeData> GetComponentType() { return GetComponentType(typeid(T)); }

    static std::optional<std::string> GetComponentName(ComponentType);
    template <typename T>
    static std::optional<std::string> GetComponentName() { return GetComponentName(typeid(T)); }

    static std::unique_ptr<IComponentMemoryPool> CreateComponentMemoryPool(ComponentType);
    template <typename T>
    static std::unique_ptr<IComponentMemoryPool> CreateComponentMemoryPool() { return CreateComponentMemoryPool(typeid(T)); }
    static ComponentPoolContainer CreateComponentMemoryPools();

    static IComponent* CreateComponent(ComponentType, const ComponentData& = ComponentData());
    static IComponent* CreateComponent(const std::string&, const ComponentData& = ComponentData());
    template <typename T>
    static IComponent* CreateComponent(const ComponentData& data = ComponentData()) { return CreateComponent(typeid(T), data); }

    template <typename C>
    static IComponent* CreateRawInstance(const ComponentData& data) {
        IComponent* c = new C();
        for (const auto& [k, v] : data.vars) {
            v->CloneValuesTo(c->data.vars[k]);
        }
        return c;
    }

    template <typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<IComponent, T>>>
    static std::shared_ptr<T> CreateInstance(Args... args) {
        std::shared_ptr<T> instance = std::make_shared<T>(args...);
        std::static_pointer_cast<IComponent>(instance)->UseDeleteDestructor(true);
        return instance;
    }


    static TypedComponentData CreateComponentData(ComponentType);
    template <typename T>
    static TypedComponentData CreateComponentData() { return CreateComponentData(typeid(T)); }

    static bool RegisterComponent(const type_info&,
        const std::function<IComponent*(const ComponentData&)>&,
        const std::function<std::unique_ptr<IComponentMemoryPool>()>&);
    template <typename C>
    static bool RegisterComponent() {
        return RegisterComponent(
            typeid(C),
            CreateRawInstance<C>,
            std::make_unique<ComponentMemoryPool<C>>
        );
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
    ComponentType type_ = typeid(Derived);
public:
    virtual ~Component() {
        if (useDeleteDestructor_)
            IDelete();
    }
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