#include <latren/ec/serialization.h>
#include <spdlog/spdlog.h>

std::vector<ComponentTypeData>& GetComponentTypes() {
    static std::vector<ComponentTypeData> componentTypes;
    return componentTypes;
}

const std::vector<ComponentTypeData>& ComponentSerialization::GetComponentTypes() {
    return ::GetComponentTypes();
}

bool ComponentSerialization::IsComponentRegistered(const std::string& name) {
    return std::find_if(GetComponentTypes().begin(), GetComponentTypes().end(), [&](const auto& t) {
        return t.name == name;
    }) != GetComponentTypes().end();
}
bool ComponentSerialization::IsComponentRegistered(ComponentType type) {
    return std::find_if(GetComponentTypes().begin(), GetComponentTypes().end(), [&](const auto& t) {
        return t.type == type;
    }) != GetComponentTypes().end();
}

const ComponentTypeData& ComponentSerialization::GetComponentType(const std::string& name) {
    auto it = std::find_if(GetComponentTypes().begin(), GetComponentTypes().end(), [&](const auto& t) {
        return t.name == name;
    });
    return *it;
}
const ComponentTypeData& ComponentSerialization::GetComponentType(ComponentType type) {
    auto it = std::find_if(GetComponentTypes().begin(), GetComponentTypes().end(), [&](const auto& t) {
        return t.type == type;
    });
    return *it;
}

std::optional<std::string> ComponentSerialization::GetComponentName(ComponentType type) {
    auto it = std::find_if(GetComponentTypes().begin(), GetComponentTypes().end(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().end())
        return std::nullopt;
    else
        return it->name;
}

std::unique_ptr<IComponentMemoryPool> ComponentSerialization::CreateComponentMemoryPool(ComponentType type) {
    auto it = std::find_if(GetComponentTypes().begin(), GetComponentTypes().end(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().end())
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

#include <iostream>
const bool ComponentSerialization::RegisterComponent(
    const char* name,
    const std::type_info& type,
    const std::function<IComponent*()>& componentInitializer,
    const std::function<std::unique_ptr<IComponentMemoryPool>()>& memPoolInitializer)
{
    if (IsComponentRegistered(name))
        return true;

    GlobalSerialization::ToggleQueueing(true);
    IComponent* dummy = componentInitializer();
    auto fields = GlobalSerialization::PopSerializables(dummy);
    delete dummy;
    GlobalSerialization::ToggleQueueing(false);
    ::GetComponentTypes().push_back({
        std::string(name),
        type,
        fields,
        componentInitializer,
        memPoolInitializer
    });
    spdlog::info("Registered component {} (serializable fields: {})", name, fields.size());
    return true;
}