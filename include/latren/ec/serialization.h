#pragma once

#include "component.h"

namespace ComponentSerialization {
    LATREN_API std::optional<ComponentTypeData> GetComponentType(const std::string&);
    LATREN_API std::optional<ComponentTypeData> GetComponentType(ComponentType);
    template <typename T>
    static std::optional<ComponentTypeData> GetComponentType() { return GetComponentType(typeid(T)); }

    LATREN_API std::optional<std::string> GetComponentName(ComponentType);
    template <typename T>
    static std::optional<std::string> GetComponentName() { return GetComponentName(typeid(T)); }

    LATREN_API std::unique_ptr<IComponentMemoryPool> CreateComponentMemoryPool(ComponentType);
    template <typename T>
    static std::unique_ptr<IComponentMemoryPool> CreateComponentMemoryPool() { return CreateComponentMemoryPool(typeid(T)); }
    LATREN_API ComponentPoolContainer CreateComponentMemoryPools();

    LATREN_API IComponent* CreateComponent(ComponentType, const ComponentData& = ComponentData());
    LATREN_API IComponent* CreateComponent(const std::string&, const ComponentData& = ComponentData());
    template <typename T>
    static IComponent* CreateComponent(const ComponentData& data = ComponentData()) { return CreateComponent(typeid(T), data); }

    LATREN_API TypedComponentData CreateComponentData(ComponentType);
    template <typename T>
    static TypedComponentData CreateComponentData() { return CreateComponentData(typeid(T)); }

    template <typename C>
    IComponent* CreateRawInstance(const ComponentData& data) {
        IComponent* c = new C();
        for (const auto& [k, v] : data.vars) {
            v->CloneValuesTo(c->data.vars[k]);
        }
        return c;
    }

    template <typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<IComponent, T>>>
    std::shared_ptr<T> CreateInstance(Args... args) {
        std::shared_ptr<T> instance = std::make_shared<T>(args...);
        std::static_pointer_cast<IComponent>(instance)->UseDeleteDestructor(true);
        return instance;
    }
    LATREN_API const bool RegisterComponent(
        const char*,
        const std::type_info&,
        const std::function<IComponent*(const ComponentData&)>&,
        const std::function<std::unique_ptr<IComponentMemoryPool>()>&);

    template <typename C>
    static const bool RegisterComponent(const char* name) {
        return RegisterComponent(
            name,
            typeid(C),
            CreateRawInstance<C>,
            std::make_unique<ComponentMemoryPool<C>>
        );
    }
};

#define LATREN_REGISTER_COMPONENT(C) ComponentSerialization::RegisterComponent<C>(#C)