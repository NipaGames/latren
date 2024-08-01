#include <latren/io/serializationinterface.h>
#include <latren/ec/serialization.h>
#include <spdlog/spdlog.h>

using namespace Serialization;

JSONDeserializerList& Serialization::GetJSONDeserializerList() {
    static JSONDeserializerList jsonDeserializers;
    return jsonDeserializers;
}

bool Serialization::UseInputFileStream(const std::string& path, std::ios_base::openmode mode, const std::function<bool(std::ifstream&)>& fn) {
    std::ifstream f(path, mode);
    if (f.fail()) {
        spdlog::error("Can't read file '{}'!", path);
        return false;
    }
    return fn(f);
}

bool Serialization::UseOutputFileStream(const std::string& path, std::ios_base::openmode mode, const std::function<bool(std::ofstream&)>& fn) {
    std::ofstream f(path, mode);
    if (f.fail()) {
        spdlog::error("Can't' write to file '{}'!", path);
        return false;
    }
    return fn(f);
}

bool JSONFileSerializer::StreamRead(std::ifstream& f) {
    try {
        data_ = nlohmann::json::parse(f);
    }
    catch (std::exception& e) {
        spdlog::error("[" + path_ + "] Invalid JSON syntax!");
        spdlog::debug(e.what());
        return false;
    }
    return ParseJSON();
}

bool JSONFileSerializer::StreamWrite(std::ofstream& f) {
    try {
        f << data_.dump();
    }
    catch (std::exception& e) {
        spdlog::error("[" + path_ + "] Can't serialize JSON!");
        spdlog::debug(e.what());
        return false;
    }
    return true;
}

bool JSONComponentDeserializer::ParseJSONComponentData(SerializableFieldValueMap& fields, const std::string& k, const nlohmann::json& jsonVal, const std::string& entityId) const {
    if (fields.count(k) == 0)
        return false;
    
    const SerializableFieldValue& field = fields.at(k);
    auto it = std::find_if(GetJSONDeserializerList().rbegin(), GetJSONDeserializerList().rend(), [&](const auto& s) {
        return s->CompareToComponentType(field);
    });
    if (it == GetJSONDeserializerList().rend())
        return false;
    
    const auto& deserializer = *it;
    DeserializationContext context;
    context.type = DeserializationContext::DeserializerType::FIELD_VALUE;
    context.field = &fields.at(k);
    bool success = false;
    switch (field.containerType) {
        case ComponentDataContainerType::SINGLE:
            success = context.Deserialize<const nlohmann::json&>(deserializer->fn, jsonVal);
            break;
        case ComponentDataContainerType::VECTOR:
            if (!jsonVal.is_array())
                return false;
            success = context.DeserializeVector(deserializer->fn, jsonVal.get<std::vector<nlohmann::json>>());
            break;
    }
    return success; 
}

bool JSONComponentDeserializer::DeserializeComponentDataFromJSON(SerializableFieldValueMap& fields, const nlohmann::json& json, const std::string& entityId) const {
    for (const auto& [k, v] : json.items()) {
        if (!ParseJSONComponentData(fields, k, v, entityId))
            return false;
    }
    return true;
}

TypedComponentData JSONComponentDeserializer::DeserializeComponent(const ComponentTypeData& typeData, const nlohmann::json& json) const {
    SerializableFieldValueMap map;
    for (const auto& [name, field] : typeData.serializableFields) {
        map.insert({ name, { nullptr, field.type, field.containerType } });
    }
    return TypedComponentData {
        typeData.type,
        map
    };
}

bool JSONComponentDeserializer::DeserializeComponents(std::vector<Serialization::TypedComponentData>& components, const nlohmann::json& json, const std::string& entityId) const {
    for (const auto& [ck, cv] : json.items()) {
        // not a component
        if (!cv.is_object())
            continue;
        
        if (!ComponentSerialization::IsComponentRegistered(ck)) {
            spdlog::warn("Component '{}' not found!", ck);
            return false;
        }
        Serialization::TypedComponentData data = DeserializeComponent(ComponentSerialization::GetComponentType(ck), cv);
        bool success = DeserializeComponentDataFromJSON(data.fields, cv, entityId);
        if (!success) {
            spdlog::warn("Failed deserializing component '{}'!", ck);
            return false;
        }
        components.push_back(data);
    }
    return true;
}