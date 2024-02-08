#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <typeindex>

enum class ComponentDataContainerType {
    SINGLE,
    VECTOR
};

class ComponentData;
class IComponentDataValue {
public:
    std::string name;
    std::type_index type;
    ComponentDataContainerType containerType;
    IComponentDataValue() : type(typeid(IComponentDataValue)) { }
    virtual void CloneValuesTo(const std::shared_ptr<IComponentDataValue>&) = 0;
    // Only present in vector types
    virtual void CopyValuesFromComponentDataArray(const ComponentData&) = 0;
};

typedef std::unordered_map<std::string, std::shared_ptr<IComponentDataValue>> VariableMap;
class ComponentData {
public:
    VariableMap vars;
    template <typename T>
    const T& Get(const std::string& key) {
        if (!vars.count(key))
            ComponentDataValue<T>::Create(key, nullptr, vars);
        std::shared_ptr<ComponentDataValue<T>> data = std::static_pointer_cast<ComponentDataValue<T>>(vars.at(key));
        if (data->ptr == nullptr)
            return data->val;
        else
            return *data->ptr;
    }
    template <typename T>
    void Set(const std::string& key, const T& val) {
        if (!vars.count(key))
            ComponentDataValue<T>::Create(key, nullptr, vars);
        std::shared_ptr<ComponentDataValue<T>> data = std::static_pointer_cast<ComponentDataValue<T>>(vars.at(key));
        data->val = val;
        if (data->ptr != nullptr)
            *data->ptr = val;
    }
    const std::shared_ptr<IComponentDataValue> GetComponentDataValue(const std::string& key) {
        if (!vars.count(key))
            return nullptr;
        return vars.at(key);
    }
};

template <typename T, ComponentDataContainerType Container>
class ComponentDataValueBase : public IComponentDataValue {
public:
    T* ptr = nullptr;
    T val;
    ComponentDataValueBase() {
        this->type = typeid(T);
        this->containerType = Container;
    }
    ComponentDataValueBase(const std::string& name, T* ptr) : ComponentDataValueBase() {
        this->name = name;
        this->ptr = ptr;
        if (ptr != nullptr)
            val = *ptr;
    }
    virtual void CloneValuesTo(const std::shared_ptr<IComponentDataValue>& c) override {
        std::shared_ptr<ComponentDataValueBase<T, Container>> data = std::static_pointer_cast<ComponentDataValueBase<T, Container>>(c);
        data->name = name;
        data->val = val;
        if (data->ptr != nullptr) {
            *data->ptr = val;
        }
    }
    virtual void CopyValuesFromComponentDataArray(const ComponentData&) { }
};

template <typename T>
class ComponentDataValue : public ComponentDataValueBase<T, ComponentDataContainerType::SINGLE> {
public:
    using ComponentDataValueBase<T, ComponentDataContainerType::SINGLE>::ComponentDataValueBase;
    static std::shared_ptr<ComponentDataValue<T>> Create(const std::string& name, T* ptr, VariableMap& map) {
        std::shared_ptr<ComponentDataValue<T>> c = std::make_shared<ComponentDataValue<T>>(name, ptr);
        map[name] = c;
        return c;
    }
};

template <typename T>
class ComponentDataValue<std::vector<T>> : public ComponentDataValueBase<std::vector<T>, ComponentDataContainerType::VECTOR> {
public:
    using VecT = std::vector<T>;
    using ComponentDataValueBase<std::vector<T>, ComponentDataContainerType::VECTOR>::ComponentDataValueBase;
    virtual void CopyValuesFromComponentDataArray(const ComponentData& data) override {
        if (this->ptr == nullptr)
            return;
        this->ptr->reserve(data.vars.size());
        for (auto v : data.vars) {
            if (v.second->type != typeid(T))
                continue;
            auto dataValue = std::dynamic_pointer_cast<ComponentDataValue<T>>(v.second);
            int i = std::stoi(v.first);
            this->ptr->insert(this->ptr->begin() + i, dataValue->val);
            this->val = *this->ptr;
        }
    }
    static std::shared_ptr<ComponentDataValue<VecT>> Create(const std::string& name, std::vector<T>* ptr, VariableMap& map) {
        std::shared_ptr<ComponentDataValue<VecT>> c = std::make_shared<ComponentDataValue<VecT>>(name, ptr);
        map[name] = c;
        return c;
    }
};

#define LE_REGISTER_COMPONENT_DATA_VALUE(name) \
std::shared_ptr<ComponentDataValue<decltype(name)>> _valPtr_##name = ComponentDataValue<decltype(name)>::Create(#name, &name, this->data.vars);
#define LE_RCDV(name) LE_REGISTER_COMPONENT_DATA_VALUE(name)

#define LE_DEFINE_COMPONENT_DATA_VALUE(T, name, val) \
T name = val; \
LE_REGISTER_COMPONENT_DATA_VALUE(name)
#define LE_DCDV(T, name, val) LE_DEFINE_COMPONENT_DATA_VALUE(T, name, val)

#define LE_DEFINE_COMPONENT_DATA_VALUE_DEFAULT(T, name) \
T name; \
LE_REGISTER_COMPONENT_DATA_VALUE(name)
#define LE_DCDV_D(T, name) LE_DEFINE_COMPONENT_DATA_VALUE_DEFAULT(T, name)