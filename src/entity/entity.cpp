#include <latren/entity/component.h>
#include <latren/util/idfactory.h>
#include <latren/gamewindow.h>
#include <latren/graphics/renderer.h>
#include <latren/game.h>

Entity::Entity() { }
Entity::Entity(EntityManager* mgr, EntityIndex i) : entityManager_(mgr), index_(i) { }

EntityIndex Entity::GetIndex() const {
    return index_;
}

EntityManager* Entity::GetManager() const {
    return entityManager_;
}

GeneralComponentReference Entity::GetComponentReference(ComponentType t) const {
    return { &GetManager()->GetComponentPool(t), *this };
}

IComponent& Entity::AddComponent(ComponentType t) const {
    GeneralComponentReference ref = GetManager()->AddComponent(*this, t);
    ref->parent = *this;
    return ref;
}
IComponent& Entity::AddComponent(const std::string& t) const {
    return AddComponent(IComponent::GetComponentType(t)->type);
}

void Entity::RemoveComponent(ComponentType t) const {
    GetManager()->DestroyComponent(*this, t);
}
void Entity::RemoveComponent(const std::string& t) const {
    auto ct = IComponent::GetComponentType(t);
    if (!ct.has_value()) {
        spdlog::warn("Component {} doesn't exist!", t);
        return;
    }
    RemoveComponent(ct->type);
}

IComponent& Entity::GetComponent(ComponentType t) const {
    return GetManager()->GetComponent(*this, t);
}
IComponent& Entity::GetComponent(const std::string& t) const {
    return GetComponent(IComponent::GetComponentType(t)->type);
}

bool Entity::HasComponent(ComponentType t) const {
    const auto& components = GetManager()->GetEntityData(*this).components;
    return components.find(t) != components.end();
}
bool Entity::HasComponent(const std::string& t) const {
    auto ct = IComponent::GetComponentType(t);
    if (!ct.has_value()) {
        spdlog::warn("Component {} doesn't exist!", t);
        return false;
    }
    return HasComponent(ct->type);
}

Transform& Entity::GetTransform() const {
    return GetComponent<Transform>();
}

const std::string& Entity::GetName() const {
    return GetManager()->GetEntityData(*this).name;
}

void Entity::Destroy() {
    GetManager()->DestroyEntity(*this);
}