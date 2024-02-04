#include <latren/entity/entity.h>
#include <latren/util/idfactory.h>
#include <latren/gamewindow.h>
#include <latren/graphics/renderer.h>
#include <latren/game.h>

inline IDFactory GLOBAL_ENTITY_ID_FACTORY;

Entity::Entity(const std::string& id) : id(id) {
    id_ = GLOBAL_ENTITY_ID_FACTORY.NextID();
    AddComponent<Transform>();
}

Entity::Entity(const Entity& e) {
    id_ = GLOBAL_ENTITY_ID_FACTORY.NextID();
    CopyFrom(e);
}

Entity::Entity(Entity&& e) : 
    id_(e.id_),
    id(e.id),
    components_(e.components_), 
    transform(e.transform)
{
    for (IComponent* c : components_) {
        c->parent = this;
    }
    e.components_.clear();
}

const Entity& Entity::operator=(const Entity& e) {
    id_ = e.id_;
    CopyFrom(e);
    return *this;
}

Entity::~Entity() {
    for (IComponent* component : components_) {
        delete component;
    }
    components_.clear();
}

EntityID Entity::GetID() const {
    return id_;
}

void Entity::CopyFrom(const Entity& e) {
    id = e.id;
    components_.clear();
    for (IComponent* c : e.components_) {
        IComponent* c2 = c->Clone();
        c2->parent = this;
        components_.push_back(c2);
    }
    transform = GetComponent<Transform>();
}

IComponent* const Entity::GetComponent(const type_info* type) const {
    size_t tHash = type->hash_code();
    for (auto c : components_) {
        if (c->typeHash == tHash)
            return c;
    }
    return nullptr;
}
IComponent* Entity::GetComponent(const std::string& name) const {
    auto ct = IComponent::GetComponentType(name);
    if (!ct.has_value())
        return nullptr;
    return GetComponent(ct.value());
}
IComponent* Entity::GetComponent(size_t tHash) const {
    auto ct = IComponent::GetComponentType(tHash);
    if (!ct.has_value())
        return nullptr;
    return GetComponent(ct.value());
}

void Entity::RemoveComponent(size_t tHash) {
    for (auto it = components_.begin(); it != components_.end(); ++it) {
        IComponent* c = *it;
        if (c->typeHash == tHash) {
            components_.erase(it);
            delete c;
            return;
        }
    }
}
void Entity::RemoveComponent(const std::string& name) {
    auto ct = IComponent::GetComponentType(name);
    if (!ct.has_value())
        RemoveComponent(ct.value()->hash_code());
}

IComponent* Entity::AddComponent(const type_info* type, const ComponentData& data) {
    IComponent* c = IComponent::CreateComponent(type, data);
    if (c == nullptr)
        return nullptr;
    c->parent = this;
    components_.push_back(c);
    if (type->hash_code() == typeid(Transform).hash_code())
        transform = dynamic_cast<Transform*>(c);
    return c;
}
IComponent* Entity::AddComponent(const std::string& name, const ComponentData& data) {
    auto ct = IComponent::GetComponentType(name);
    if (!ct.has_value())
        return nullptr;
    return AddComponent(ct.value(), data);
}
IComponent* Entity::AddComponent(size_t tHash, const ComponentData& data) {
    auto ct = IComponent::GetComponentType(tHash);
    if (!ct.has_value())
        return nullptr;
    return AddComponent(ct.value(), data);
}

void Entity::OverrideComponentValues(const Entity& e) {
    for (auto c : e.components_) {
        IComponent* mc = GetComponent(c->typeHash);
        if (mc == nullptr)
            AddComponent(c->typeHash, c->data);
        else {
            for (const auto&[k, v] : c->data.vars) {
                v->CloneValuesTo(mc->data.vars[k]);
            }
        }
    }
}

void Entity::Start() {
    for (IComponent* component : components_) {
        if (!component->hasStarted_)
            component->IStart();
        component->hasStarted_ = true;
    }
}

void Entity::Update() {
    for (int i = 0; i < components_.size(); i++) {
        IComponent* component = components_.at(i);
        if (!component->hasHadFirstUpdate_) {
            component->IFirstUpdate();
            component->hasHadFirstUpdate_ = true;
        }
        component->IUpdate();
    }
}

void Entity::FixedUpdate() {
    for (auto component : components_) {
        component->IFixedUpdate();
    }
}

void Entity::Destroy(EntityManager& mgr) {
    mgr.RemoveEntity(id_);
}

void Entity::Destroy() {
    Destroy(Game::GetGameInstanceBase()->GetEntityManager());
}

std::vector<std::string> Entity::ListComponentNames() const {
    std::vector<std::string> names;
    for (const IComponent* c : components_) {
        auto optn = IComponent::GetComponentName(c->typeHash);
        if (optn.has_value())
            names.push_back(optn.value());
    }
    return names;
}