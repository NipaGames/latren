#pragma once

#include "entitymanager.h"
#include "memmgr.h"
#include <latren/defines/opengl.h>

class IComponent;
class Transform;

// basically a wrapper around entitymanager, doesn't store anything by itself
// you could also think this as a pointer to entitymanager which actually stores the data
class LATREN_API Entity {
private:
    EntityManager* entityManager_;
    EntityIndex index_;
public:
    Entity();
    Entity(EntityManager*, EntityIndex);
    EntityIndex GetIndex() const;
    EntityManager* GetManager() const;
    operator EntityIndex() const { return GetIndex(); }
    GeneralComponentReference GetComponentReference(ComponentType) const;
    template <typename C>
    ComponentReference<C> GetComponentReference() const {
        return { &GetManager()->GetComponentPool<C>(), *this };
    }
    IComponent& AddComponent(ComponentType) const;
    template <typename C>
    C& AddComponent() const {
        return static_cast<C&>(AddComponent(typeid(C)));
    }
    IComponent& GetComponent(ComponentType) const;
    template <typename C>
    C& GetComponent() const {
        return GetManager()->GetComponent<C>(*this);
    }
    Transform& GetTransform() const;
    const std::string& GetName() const;
};