#pragma once

#include "serializationinterface.h"
#include "files/cfg.h"

namespace ComponentSerialization {
    template <typename T>
    bool DeserializeJSONNumber(Serialization::DeserializationContext& args, const nlohmann::json& j) {
        if (!j.is_number())
            return false;
        args.Return<T>(j);
        return true;
    }

    template <std::size_t S>
    bool DeserializeJSONVector(Serialization::DeserializationContext& args, const nlohmann::json& j) {
        typedef glm::vec<S, float> Vector;
        Vector vec;
        if (j.is_number()) vec = Vector(j);
        else if (j.is_array() && j.size() == S) {
            for (int i = 0; i < S; i++) {
                if (!j[i].is_number()) return false;
                vec[i] = j[i];
            }
        }
        else return false;
        args.Return(vec);
        return true;
    }

    template <std::size_t S, typename T>
    bool DeserializeCFGVector(Serialization::DeserializationContext& args, const CFG::ICFGField* field, CFG::CFGFieldType type) {
        if (field->type != CFG::CFGFieldType::STRUCT && field->type != CFG::CFGFieldType::ARRAY)
            return false;
        const CFG::CFGObject* obj = static_cast<const CFG::CFGObject*>(field);
        const auto& items = obj->GetItems();
        if (items.size() != S)
            return false;
        glm::vec<S, T> vec;
        for (int i = 0; i < S; i++) {
            const CFG::ICFGField* child = items.at(i);
            if (!IsValidType(child->type, type))
                return false;
            vec[i] = child->GetValue<T>();
        }
        args.Return(vec);
        return true;
    }

    template <std::size_t S, typename T, CFG::CFGFieldType F>
    bool DeserializeCFGVector(Serialization::DeserializationContext& args, const CFG::ICFGField* field) {
        return DeserializeCFGVector<S, T>(args, field, F);
    }

    template <std::size_t S, typename T>
    bool DeserializeCFGVectorAuto(Serialization::DeserializationContext& args, const CFG::ICFGField* field) {
        std::size_t typeHash = typeid(T).hash_code();
        if (CFG::GetCFGTypeMap().find(typeHash) == CFG::GetCFGTypeMap().end())
            return false;
        return DeserializeCFGVector<S, T>(args, field, CFG::GetCFGTypeMap().at(typeHash).front());
    }

};