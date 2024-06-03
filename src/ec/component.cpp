#include <latren/ec/component.h>
#include <latren/ec/entity.h>
#include <latren/systems.h>
#include <spdlog/spdlog.h>

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

ComponentType IComponent::GetType() const {
    return typeid(IComponent);
}

ComponentData& IComponent::GetData() {
    return data;
}

bool IComponent::HasStarted() const {
    return hasStarted_;
}

void IComponent::UseDeleteDestructor(bool use) {
    useDeleteDestructor_ = use;
}

IComponent::operator EntityIndex() const {
    return parent.GetIndex();
}

double IComponent::GetTime() {
    return Systems::GetTime();
}