#include <latren/entity/component.h>
#include <latren/entity/entity.h>

std::vector<ComponentType> COMPONENT_TYPES;

IComponent::IComponent(const IComponent& c) {
    hasStarted_ = c.hasStarted_;
    hasHadFirstUpdate_ = c.hasHadFirstUpdate_;
    for (const auto& [k, v] : c.data.vars) {
        if (data.vars.count(k) != 0)
            v->CloneValuesTo(data.vars.at(k));
    }
    data = c.data;
    parent = c.parent;
    typeHash = c.typeHash;
}

std::optional<const type_info*> IComponent::GetComponentType(const std::string& name) {
    auto it = std::find_if(COMPONENT_TYPES.begin(), COMPONENT_TYPES.end(), [&](const auto& t) {
        return t.name == name;
    });
    if (it == COMPONENT_TYPES.end())
        return std::nullopt;
    else
        return it->type;
}
std::optional<const type_info*> IComponent::GetComponentType(size_t tHash) {
    auto it = std::find_if(COMPONENT_TYPES.begin(), COMPONENT_TYPES.end(), [&](const auto& t) {
        return t.type->hash_code() == tHash;
    });
    if (it == COMPONENT_TYPES.end())
        return std::nullopt;
    else
        return it->type;
}

std::optional<std::string> IComponent::GetComponentName(size_t tHash) {
    auto it = std::find_if(COMPONENT_TYPES.begin(), COMPONENT_TYPES.end(), [&](const auto& t) {
        return t.type->hash_code() == tHash;
    });
    if (it == COMPONENT_TYPES.end())
        return std::nullopt;
    else
        return it->name;
}

IComponent* IComponent::CreateComponent(const type_info* type, const ComponentData& data) {
    auto it = std::find_if(COMPONENT_TYPES.begin(), COMPONENT_TYPES.end(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == COMPONENT_TYPES.end())
        return nullptr;
    IComponent* c = it->initializer(data);
    c->typeHash = type->hash_code();
    return c;
}
IComponent* IComponent::CreateComponent(const std::string& name, const ComponentData& data) {
    auto it = std::find_if(COMPONENT_TYPES.begin(), COMPONENT_TYPES.end(), [&](const auto& t) {
        return t.name == name;
    });
    if (it == COMPONENT_TYPES.end())
        return nullptr;
    return CreateComponent(it->type, data);
}
IComponent* IComponent::CreateComponent(size_t hash, const ComponentData& data) {
    auto it = std::find_if(COMPONENT_TYPES.begin(), COMPONENT_TYPES.end(), [&](const auto& t) {
        return t.type->hash_code() == hash;
    });
    if (it == COMPONENT_TYPES.end())
        return nullptr;
    return CreateComponent(it->type, data);
}

bool IComponent::RegisterComponent(const type_info* type, IComponent*(*initializer)(const ComponentData&)) {
    if (GetComponentType(type->hash_code()) != std::nullopt) {
        return true;
    }
    std::string name = std::string(type->name());
    size_t i = name.find(" ");
    if (i != std::string::npos)
        name = name.substr(i + 1);

    COMPONENT_TYPES.push_back({ name, type, initializer });
    std::cout << "<Registered component " << name << ">" << std::endl;
    return true;
}