#pragma once

#include "component.h"

namespace ComponentSerialization {
     const std::vector<ComponentTypeData>& GetComponentTypes();
     bool IsComponentRegistered(const std::string&);
     bool IsComponentRegistered(ComponentType);
     const ComponentTypeData& GetComponentType(const std::string&);
     const ComponentTypeData& GetComponentType(ComponentType);
    template <typename T>
    static const ComponentTypeData& GetComponentType() { return GetComponentType(typeid(T)); }

     std::optional<std::string> GetComponentName(ComponentType);
    template <typename T>
    static std::optional<std::string> GetComponentName() { return GetComponentName(typeid(T)); }

     std::unique_ptr<IComponentMemoryPool> CreateComponentMemoryPool(ComponentType);
    template <typename T>
    static std::unique_ptr<IComponentMemoryPool> CreateComponentMemoryPool() { return CreateComponentMemoryPool(typeid(T)); }
     ComponentPoolContainer CreateComponentMemoryPools();

    template <typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<IComponent, T>>>
    std::shared_ptr<T> CreateInstance(Args... args) {
        std::shared_ptr<T> instance = std::make_shared<T>(args...);
        std::static_pointer_cast<IComponent>(instance)->UseDeleteDestructor(true);
        return instance;
    }
     const bool RegisterComponent(
        const char*,
        const std::type_info&,
        const std::function<IComponent*()>&,
        const std::function<std::unique_ptr<IComponentMemoryPool>()>&);

    template <typename C>
    static const bool RegisterComponent(const char* name) {
        return RegisterComponent(
            name,
            typeid(C),
            []() {
                C* c = new C();
                c->OverrideType(typeid(C));
                return c;
            },
            std::make_unique<ComponentMemoryPool<C>>
        );
    }

     void RegisterCoreComponents();
     void RegisterCoreDeserializers();
};

#define LATREN_REGISTER_COMPONENT(C) ComponentSerialization::RegisterComponent<C>(#C)