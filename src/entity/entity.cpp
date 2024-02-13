#include <latren/entity/component.h>
#include <latren/util/idfactory.h>
#include <latren/gamewindow.h>
#include <latren/graphics/renderer.h>
#include <latren/game.h>

Entity::Entity() { }
Entity::Entity(EntityManager* mgr, EntityIndex i) : entityManager_(mgr), index_(i) { }

ComponentMemoryManager& Entity::GetComponentMemoryManager() const {
    return entityManager_->GetComponentMemory();
}

EntityIndex Entity::GetIndex() const {
    return index_;
}

GeneralComponentReference Entity::GetComponentReference(std::type_index t) const {
    return { &GetComponentMemoryManager().GetPool(t), GetIndex() };
}

IComponent& Entity::AddComponent(std::type_index t) const {
    GeneralComponentReference ref = GetComponentMemoryManager().AllocNewComponent(GetIndex(), t);
    ref->parent = *this;
    return ref;
}

IComponent& Entity::GetComponent(std::type_index t) const {
    return GetComponentMemoryManager().GetPool(t).GetComponentBase(GetIndex());
}

Transform& Entity::GetTransform() const {
    return GetComponent<Transform>();
}

const std::string& Entity::GetName() const {
    return entityManager_->GetEntityData(*this).name;
}