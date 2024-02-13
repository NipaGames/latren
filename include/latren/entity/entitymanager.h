#pragma once

#include <list>

#include "component.h"
#include "memmgr.h"
#include <latren/util/idfactory.h>

class EntityManager : public IDFactory<EntityIndex> {
friend class Game;
private:
    ComponentMemoryManager componentMemoryManager_;
public:
    LATREN_API void Setup();
    LATREN_API void StartAll();
    LATREN_API void UpdateAll();
    LATREN_API void FixedUpdateAll();
    LATREN_API ComponentMemoryManager& GetComponentMemory();
    LATREN_API Entity CreateEntity();
    LATREN_API GeneralComponentReference AddComponent(EntityIndex, std::type_index);
    LATREN_API void DeleteComponent(EntityIndex, std::type_index);
    LATREN_API void ClearEverything();
    LATREN_API size_t GetTotalPoolBytes();
};