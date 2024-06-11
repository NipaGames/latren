#include <latren/ec/component.h>
#include <latren/ec/entity.h>
#include <latren/systems.h>
#include <spdlog/spdlog.h>

ComponentType IComponent::GetType() const {
    return typeid(IComponent);
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