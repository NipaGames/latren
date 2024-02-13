#pragma once

#include "component.h"
#include "memmgr.h"
#include <latren/util/idfactory.h>

struct GlobalEntityData {
    std::string name;
};

class EntityManager : public IDFactory<EntityIndex> {
friend class Game;
private:
    ComponentMemoryManager componentMemoryManager_;
    // i'm not actually sure if the names of destroyed components should be dynamically removed
    // at the moment they're not
    std::unordered_map<std::string, EntityIndex> entityNames_;
    // these however are dynamically managed (faster entityindex lookup)
    std::unordered_map<EntityIndex, GlobalEntityData> entityData_;
public:
    LATREN_API void Setup();
    LATREN_API void StartAll();
    LATREN_API void UpdateAll();
    LATREN_API void FixedUpdateAll();
    LATREN_API Entity GetNamedEntity(const std::string&);
    LATREN_API const GlobalEntityData& GetEntityData(EntityIndex);
    LATREN_API ComponentMemoryManager& GetComponentMemory();
    LATREN_API Entity CreateEntity(const std::string& = "");
    LATREN_API GeneralComponentReference AddComponent(EntityIndex, std::type_index);
    LATREN_API void DeleteComponent(EntityIndex, std::type_index);
    LATREN_API void ClearEverything();
    LATREN_API size_t GetTotalPoolBytes();
};