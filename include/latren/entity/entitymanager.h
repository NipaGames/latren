#pragma once

#include <list>

#include "entity.h"

class EntityManager {
private:
    std::list<std::unique_ptr<Entity>> entities_;
public:
    LATREN_API Entity& CreateEntity(const std::string& = "");
    LATREN_API Entity& AddEntity(const Entity&);
    LATREN_API void RemoveEntities(const std::string&);
    LATREN_API void RemoveEntity(size_t);
    LATREN_API size_t CountEntities(const std::string&);
    LATREN_API size_t CountEntities();
    LATREN_API void ClearEntities();
    LATREN_API bool HasEntity(size_t);
    // also creates a new entity if it doesn't exist
    LATREN_API Entity& operator[](const std::string& id);
    LATREN_API Entity& GetEntity(const std::string& id);
    LATREN_API const std::list<std::unique_ptr<Entity>>& GetEntities();
    LATREN_API friend class Game;
};