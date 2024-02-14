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

IComponent& Entity::GetComponent(ComponentType t) const {
    return GetManager()->GetComponent(*this, t);
}

Transform& Entity::GetTransform() const {
    return GetComponent<Transform>();
}

const std::string& Entity::GetName() const {
    return GetManager()->GetEntityData(*this).name;
}