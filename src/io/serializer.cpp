#include <latren/io/serializer.h>
#include <spdlog/spdlog.h>

using namespace Serializer;

JSONDeserializerList& Serializer::GetJSONDeserializerList() {
    static JSONDeserializerList jsonDeserializers;
    return jsonDeserializers;
}

bool Serializer::UseInputFileStream(const std::string& path, std::ios_base::openmode mode, const std::function<bool(std::ifstream&)>& fn) {
    std::ifstream f(path, mode);
    if (f.fail()) {
        spdlog::error("Can't read file '{}'!", path);
        return false;
    }
    return fn(f);
}

bool Serializer::UseOutputFileStream(const std::string& path, std::ios_base::openmode mode, const std::function<bool(std::ofstream&)>& fn) {
    std::ofstream f(path, mode);
    if (f.fail()) {
        spdlog::error("Can't' write to file '{}'!", path);
        return false;
    }
    return fn(f);
}

bool Serializer::ParseJSONComponentData(SerializableFieldValueMap& fields, const std::string& k, const nlohmann::json& jsonVal, const std::string& entityId) {
    if (fields.count(k) == 0)
        return false;
    
    const SerializableFieldValue& field = fields.at(k);
    auto it = std::find_if(GetJSONDeserializerList().begin(), GetJSONDeserializerList().end(), [&](const auto& s) {
        return s->CompareToComponentType(field);
    });
    if (it == GetJSONDeserializerList().end())
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