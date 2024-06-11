#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include "mempool.h"
#include <latren/latren.h>
#include <latren/util/templatestr.h>

enum class ComponentDataContainerType {
    SINGLE,
    VECTOR
};

class IComponent;
class ISerializable {
public:
    virtual const void* GetPtr() const = 0;
    virtual const char* GetName() const = 0;
    virtual std::type_index GetType() const = 0;
    virtual ComponentDataContainerType GetContainerType() const = 0;
};

struct SerializableField {
    ptrdiff_t offset;
    int index;
    std::type_index type;
    ComponentDataContainerType containerType;
};
typedef std::unordered_map<std::string, SerializableField> SerializableFieldMap;

namespace GlobalSerialization {
    static constexpr inline char EMPTY_VAR_NAME[] = "";

    LATREN_API void ToggleQueueing(bool);
    LATREN_API void PushSerializable(ISerializable*);
    LATREN_API void PopSerializables();
    LATREN_API SerializableFieldMap PopSerializables(const IComponent*);
};

template <typename T>
struct ComponentDataContainerTypeDeduction {
    static constexpr ComponentDataContainerType TYPE = ComponentDataContainerType::SINGLE;
};
template <typename T, typename A>
struct ComponentDataContainerTypeDeduction<std::vector<T, A>> {
    static constexpr ComponentDataContainerType TYPE = ComponentDataContainerType::VECTOR;
};

template <typename T, typename... Other>
class Serializable : ISerializable {
public:
    using Type = T;
protected:
    Type val_;
public:
    virtual ~Serializable() { }
    template <typename... Args>
    Serializable(Args... args) : val_(args...) {
        GlobalSerialization::PushSerializable(this);
    }
    template <typename... Args>
    Serializable<Type, Other...>& operator=(const Serializable<Type, Args...>& v) {
        val_ = v.Get();
        return *this;
    }
    const Type& Get() const {
        return val_;
    }
    Type& Get() {
        return val_;
    }
    operator const Type&() const {
        return Get();
    }
    operator Type&() {
        return Get();
    }
    const Type* operator->() const {
        return &Get();
    }
    Type* operator->() {
        return &Get();
    }

    const void* GetPtr() const override {
        return &val_;
    }
    virtual const char* GetName() const override {
        return "";
    }
    ComponentType GetType() const override {
        return typeid(Type);
    }
    ComponentDataContainerType GetContainerType() const override {
        return ComponentDataContainerTypeDeduction<Type>::TYPE;
    }
};

template <typename T, char... VarName>
class Serializable<T, TemplateString<VarName...>> : public Serializable<T> {
using Serializable<T>::Serializable;
public:
    virtual const char* GetName() const override {
        return ReadTemplateString<VarName...>();
    }
};

#define SERIALIZABLE(Type, name) Serializable<Type, decltype(#name""_tstr)> name