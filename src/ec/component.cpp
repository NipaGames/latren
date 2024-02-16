#include <latren/ec/component.h>
#include <latren/ec/entity.h>

IComponent::IComponent(const IComponent& c) :
    hasStarted_(c.hasStarted_),
    parent(c.parent),
    pool(c.pool)
{
    for (const auto& [k, v] : c.data.vars) {
        if (data.vars.find(k) != data.vars.end())
            v->CloneValuesTo(data.vars.at(k));
    }
    data = c.data;
}

IComponent::operator EntityIndex() const {
    return parent.GetIndex();
}

std::vector<ComponentTypeData>& GetComponentTypes() {
    static std::vector<ComponentTypeData> componentTypes;
    return componentTypes;
}

std::optional<ComponentTypeData> IComponent::GetComponentType(const std::string& name) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.name == name;
    });
    if (it == GetComponentTypes().cend())
        return std::nullopt;
    else
        return *it;
}
std::optional<ComponentTypeData> IComponent::GetComponentType(ComponentType type) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().cend())
        return std::nullopt;
    else
        return *it;
}

std::optional<std::string> IComponent::GetComponentName(ComponentType type) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().cend())
        return std::nullopt;
    else
        return it->name;
}

std::unique_ptr<IComponentMemoryPool> IComponent::CreateComponentMemoryPool(ComponentType type) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().cend())
        return nullptr;
    return it->memPoolInitializer();
}

ComponentPoolContainer IComponent::CreateComponentMemoryPools() {
    ComponentPoolContainer pools;
    for (const ComponentTypeData& t : GetComponentTypes()) {
        pools.insert({ t.type, t.memPoolInitializer() });
    }
    return pools;
}

IComponent* IComponent::CreateComponent(ComponentType type, const ComponentData& data) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.type == type;
    });
    if (it == GetComponentTypes().cend())
        return nullptr;
    IComponent* c = it->componentInitializer(data);
    c->UseDeleteDestructor(true);
    return c;
}
IComponent* IComponent::CreateComponent(const std::string& name, const ComponentData& data) {
    auto it = std::find_if(GetComponentTypes().cbegin(), GetComponentTypes().cend(), [&](const auto& t) {
        return t.name == name;
    });
    if (it == GetComponentTypes().cend())
        return nullptr;
    return CreateComponent(it->type, data);
}

TypedComponentData IComponent::CreateComponentData(ComponentType type) {
    TypedComponentData data = TypedComponentData(type);
    IComponent* dummy = CreateComponent(type);
    for (const auto& [k, v] : dummy->data.vars) {
        data.vars[k] = v;
        data.vars[k]->DetachPointer();
    }
    delete dummy;
    return data;
}

bool IComponent::RegisterComponent(
    const type_info& type,
    const std::function<IComponent*(const ComponentData&)>& componentInitializer,
    const std::function<std::unique_ptr<IComponentMemoryPool>()>& memPoolInitializer)
{
    if (GetComponentType(type) != std::nullopt) {
        return true;
    }
    std::string name = std::string(type.name());
    size_t i = name.find(" ");
    if (i != std::string::npos)
        name = name.substr(i + 1);

    GetComponentTypes().push_back({ name, type, componentInitializer, memPoolInitializer });
    std::cout << "<Registered component " << name << ">" << std::endl;
    return true;
}