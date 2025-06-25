#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <any>
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
    virtual std::function<bool(void*, const std::any&)> GetAssignmentOperator() const = 0;
};

struct SerializableField {
    std::ptrdiff_t offset;
    int index;
    std::type_index type;
    ComponentDataContainerType containerType;
    std::function<bool(void*, const std::any&)> assignmentOperator;
};
typedef std::unordered_map<std::string, SerializableField> SerializableFieldMap;

namespace GlobalSerialization {
    static constexpr inline char EMPTY_VAR_NAME[] = "";

     void ToggleQueueing(bool);
     void PushSerializable(ISerializable*);
     void PopSerializables();
     SerializableFieldMap PopSerializables(const IComponent*);
};

template <typename T>
struct ComponentDataContainerTypeDeduction {
    static constexpr ComponentDataContainerType TYPE = ComponentDataContainerType::SINGLE;
};
template <typename T, typename A>
struct ComponentDataContainerTypeDeduction<std::vector<T, A>> {
    static constexpr ComponentDataContainerType TYPE = ComponentDataContainerType::VECTOR;
};

template <typename T, const char* Name>
class Serializable : public ISerializable {
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
    template <const char* OtherName>
    Serializable<Type, Name>& operator=(const Serializable<Type, OtherName>& v) {
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
    const char* GetName() const override {
        return Name;
    }
    ComponentType GetType() const override {
        return typeid(Type);
    }
    ComponentDataContainerType GetContainerType() const override {
        return ComponentDataContainerTypeDeduction<Type>::TYPE;
    }
    std::function<bool(void*, const std::any&)> GetAssignmentOperator() const override {
        return [](void* ptr, const std::any& val) -> bool {
            Type& thisVal = *static_cast<Type*>(ptr);
            if (val.type().hash_code() != typeid(Type).hash_code())
                return false;
            thisVal = std::any_cast<const T&>(val);
            return true;
        };
    };
};

// https://stackoverflow.com/a/13842784
#define COMBINE_VARGS(...) __VA_ARGS__

// if the type contains commas, wrap it in COMBINE_VARGS
#define SERIALIZABLE(Type, name) static constexpr const char _varName_##name[] = #name; Serializable<Type, _varName_##name> name