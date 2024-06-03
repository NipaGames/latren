#include <latren/ec/serialization.h>
#include <spdlog/spdlog.h>

std::vector<ComponentTypeData>& GetComponentTypes() {
    static std::vector<ComponentTypeData> componentTypes;
    return componentTypes;
}

std::optional<ComponentTypeData> ComponentSerialization::GetComponentType(const std::string& name) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.name == name;
    });
    if (it == GetComponentTypes().cend())
        return std::nullopt;
    else
        return *it;
}
std::optional<ComponentTypeData> ComponentSerialization::GetComponentType(ComponentType type) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().cend())
        return std::nullopt;
    else
        return *it;
}

std::optional<std::string> ComponentSerialization::GetComponentName(ComponentType type) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().cend())
        return std::nullopt;
    else
        return it->name;
}

std::unique_ptr<IComponentMemoryPool> ComponentSerialization::CreateComponentMemoryPool(ComponentType type) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().cend())
        return nullptr;
    return it->memPoolInitializer();
}

ComponentPoolContainer ComponentSerialization::CreateComponentMemoryPools() {
    ComponentPoolContainer pools;
    for (const ComponentTypeData& t : GetComponentTypes()) {
        pools.insert({ t.type, t.memPoolInitializer() });
    }
    return pools;
}

IComponent* ComponentSerialization::CreateComponent(ComponentType type, const ComponentData& data) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().cend())
        return nullptr;
    IComponent* c = it->componentInitializer(data);
    c->UseDeleteDestructor(true);
    return c;
}
IComponent* ComponentSerialization::CreateComponent(const std::string& name, const ComponentData& data) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.name == name;
    });
    if (it == GetComponentTypes().cend())
        return nullptr;
    return CreateComponent(it->type, data);
}

TypedComponentData ComponentSerialization::CreateComponentData(ComponentType type) {
    TypedComponentData data = TypedComponentData(type);
    IComponent* dummy = CreateComponent(type);
    for (const auto& [k, v] : dummy->data.vars) {
        data.vars[k] = v;
        data.vars[k]->DetachPointer();
    }
    delete dummy;
    return data;
}

const bool ComponentSerialization::RegisterComponent(
    const char* name,
    const std::type_info& type,
    const std::function<IComponent*(const ComponentData&)>& componentInitializer,
    const std::function<std::unique_ptr<IComponentMemoryPool>()>& memPoolInitializer)
{
    if (GetComponentType(type) != std::nullopt)
        return true;

    GetComponentTypes().push_back({ std::string(name), type, componentInitializer, memPoolInitializer });
    spdlog::info("Registered component {}", name);
    return true;
}