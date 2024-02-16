#pragma once

#include <type_traits>

class IComponent;

class IComponentWrapper { };

template <typename T, typename = std::enable_if_t<std::is_base_of_v<IComponent, T>>>
class ComponentWrapper : public IComponentWrapper {
public:
    T component;

    ~ComponentWrapper() {
        component.IDelete();
    }
    template <typename... Args>
    ComponentWrapper(Args... args) : component(args...) { }
    T* operator->() {
        return &component;
    }
    T& Get() {
        return component;
    }
    template <typename Cast, typename = std::enable_if_t<std::is_base_of_v<Cast, T>>>
    operator ComponentWrapper<Cast>&() {
        return static_cast<ComponentWrapper<Cast>&>(static_cast<IComponentWrapper&>(*this));
    }

    template <typename... Args>
    static std::shared_ptr<ComponentWrapper<T>> CreateInstance(Args... args) {
        return std::make_shared<ComponentWrapper<T>>(args...);
    }
};

template <typename T>
using SharedComponentPtr = std::shared_ptr<ComponentWrapper<T>>;