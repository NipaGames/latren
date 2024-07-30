#pragma once

#include "../serializationinterface.h"
#include "blueprints.h"
#include <latren/stage.h>

namespace Serialization {
    class LATREN_API StageSerializer : public JSONFileSerializer, public Serialization::JSONComponentDeserializer {
    protected:
        Stage stage_;
        Serialization::BlueprintSerializer* blueprints_ = nullptr;
        virtual bool ParseJSON() override;
        virtual bool ParseEntityBlueprints(DeserializedEntity&, const std::string&);
        virtual std::vector<DeserializedEntity> ParseEntities(const nlohmann::json& entities, int* = nullptr);
    public:
        virtual Stage& GetStage();
        void UseBlueprints(Serialization::BlueprintSerializer* blueprints) {
            blueprints_ = blueprints;
        }
        StageSerializer(const std::string& p) {
            DeserializeFile(p);
        }
        StageSerializer() = default;
    };
};
