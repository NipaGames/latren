#pragma once

#include "../serializer.h"
#include "blueprints.h"
#include <latren/stage.h>

namespace Serializer {
    LATREN_API bool DeserializeComponentDataFromJSON(ComponentData&, const nlohmann::json&, const std::string& = "");
    class StageSerializer : public JSONFileSerializer {
    protected:
        Stage stage_;
        Serializer::BlueprintSerializer* blueprints_ = nullptr;
        bool ParseJSON() override;
        std::vector<DeserializedEntity> ParseEntities(const nlohmann::json& entities, int* = nullptr);
    public:
        LATREN_API Stage& GetStage();
        void UseBlueprints(Serializer::BlueprintSerializer* blueprints) {
            blueprints_ = blueprints;
        }
        StageSerializer(const std::string& p) {
            DeserializeFile(p);
        }
        LATREN_API StageSerializer() = default;
    };
};
