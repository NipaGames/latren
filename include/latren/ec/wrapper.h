#pragma once

#include <type_traits>

class IComponent;

class IComponentWrapper {
public:
    virtual ~IComponentWrapper() { }
};

template <typename T, typename = std::enable_if_t<std::is_base_of_v<IComponent, T>>>
class ComponentWrapper : public IComponentWrapper {
public:
    T component;

    virtual ~ComponentWrapper() {
        component.IDelete();
    }
    template <typename... Args>
    ComponentWrapper(Args... args) : component(args...) { }
    T* operator->() const {
        return &component;
    }
    T& Get() {
        return component;
    }
    operator T&() {
        return Get();
    }

    template <typename... Args>
    static std::shared_ptr<ComponentWrapper<T>> CreateInstance(Args... args) {
        return std::make_shared<ComponentWrapper<T>>(args...);
    }
};

template <typename T>
using SharedComponentPtr = std::shared_ptr<ComponentWrapper<T>>;

template <typename Cast, typename T>
SharedComponentPtr<Cast> SharedComponentPtrCast(const SharedComponentPtr<T>& ptr) {
    // cover your eyes
    return std::static_pointer_cast<ComponentWrapper<Cast>>(std::static_pointer_cast<IComponentWrapper>(ptr));
}