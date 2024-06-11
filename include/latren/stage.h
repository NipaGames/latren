#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <latren/defines/opengl.h>
#include <nlohmann/json.hpp>

#include "ec/entity.h"
#include "io/serializer.h"

struct DeserializedEntity {
    std::string id;
    std::vector<Serializer::TypedComponentData> components;
};

struct Stage {
    std::string id;
    // additional game-specific data
    nlohmann::json data;

    std::vector<DeserializedEntity> entities;
    std::unordered_set<EntityIndex> instantiatedEntities;
};
