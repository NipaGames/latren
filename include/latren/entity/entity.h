#pragma once

#include <latren/defines/opengl.h>
#include "component.h"

typedef size_t EntityID;

class Transform;
class EntityManager;
class Entity {
private:
    std::vector<IComponent*> components_;
    EntityID id_;
public:
    Transform* transform;
    std::string id;

    LATREN_API Entity(const std::string& = "");
    LATREN_API Entity(const Entity&);
    LATREN_API Entity(Entity&&);
    LATREN_API const Entity& operator=(const Entity&);
    LATREN_API virtual ~Entity();

    LATREN_API void Start();
    LATREN_API void Update();
    LATREN_API void FixedUpdate();
    LATREN_API void CopyFrom(const Entity&);
    LATREN_API void Destroy(EntityManager& mgr);
    LATREN_API void Destroy();
    LATREN_API EntityID GetID() const;
    LATREN_API IComponent* const GetComponent(const std::type_index&) const;
    template <typename C>
    C* const GetComponent() const {
        return static_cast<C*>(GetComponent(typeid(C)));
    }
    LATREN_API IComponent* GetComponent(const std::string&) const;
    LATREN_API void RemoveComponent(const std::type_index&);
    LATREN_API void RemoveComponent(const std::string&);
    template <typename C>
    void RemoveComponent() {
        RemoveComponent(typeid(C));
    }
    template <typename... C>
    void RemoveComponents() {
        ([&] {
            RemoveComponent<C>();
        } (), ...);
    }
    template <typename C, typename... Args>
    C* AddComponent(Args... args) {
        C* c = new C((args)...);
        c->parent = this;
        components_.push_back(static_cast<IComponent*>(c));
        if (typeid(C) == typeid(Transform))
            transform = dynamic_cast<Transform*>(c);
        return c;
    }
    LATREN_API IComponent* AddComponent(const std::type_index&, const ComponentData& = ComponentData());
    LATREN_API IComponent* AddComponent(const std::string&, const ComponentData& = ComponentData());
    LATREN_API std::vector<std::string> ListComponentNames() const;
    LATREN_API void OverrideComponentValues(const Entity& e);
};