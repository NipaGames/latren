#pragma once

#include <latren/defines/opengl.h>
#include "memmgr.h"

class IComponent;
class Transform;
class EntityManager;
class LATREN_API Entity {
private:
    EntityManager* entityManager_;
    EntityIndex index_;
public:
    Entity();
    Entity(EntityManager*, EntityIndex);
    EntityIndex GetIndex() const;
    operator EntityIndex() const { return GetIndex(); }
    GeneralComponentReference GetComponentReference(std::type_index) const;
    ComponentMemoryManager& GetComponentMemoryManager() const;
    template <typename C>
    ComponentReference<C> GetComponentReference() const {
        return { &GetComponentMemoryManager().GetPool<C>(), GetIndex() };
    }
    IComponent& AddComponent(std::type_index) const;
    template <typename C>
    C& AddComponent() const {
        return static_cast<C&>(AddComponent(typeid(C)));
    }
    IComponent& GetComponent(std::type_index) const;
    template <typename C>
    C& GetComponent() const {
        return GetComponentMemoryManager().GetPool<C>().GetComponent(GetIndex());
    }
    Transform& GetTransform() const;
};