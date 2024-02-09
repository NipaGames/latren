#include <latren/io/serializer.h>

using namespace Serializer;

JSONSerializerList& Serializer::GetJSONSerializerList() {
    static JSONSerializerList jsonSerializers;
    return jsonSerializers;
}

bool Serializer::ParseJSONComponentData(ComponentData& data, const std::string& k, const nlohmann::json& jsonVal, const std::string& entityId) {
    auto dataVal = data.GetComponentDataValue(k);
    if (dataVal == nullptr)
        return false;
    
    auto it = std::find_if(GetJSONSerializerList().begin(), GetJSONSerializerList().end(), [&](const auto& s) {
        return s->CompareToComponentType(data.GetComponentDataValue(k));
    });
    if (it == GetJSONSerializerList().end())
        return false;
    
    const auto& serializer = *it;
    SerializationArgs args(SerializerType::COMPONENT_DATA);
    args.entityId = entityId;
    switch (dataVal->containerType) {
        case ComponentDataContainerType::SINGLE:
            args.ctData = &data;
            args.ctK = k;
            return (serializer->fn)(args, jsonVal);
        case ComponentDataContainerType::VECTOR:
            if (!jsonVal.is_array())
                return false;

            ComponentData arrayWrapper;
            args.ctData = &arrayWrapper;
            bool hasAnyFailed = false;
            int i = 0;
            for (auto e : jsonVal) {
                std::string key = std::to_string(i++);
                args.ctK = key;
                if (!(serializer->fn)(args, e))
                    hasAnyFailed = true;
            }
            dataVal->CopyValuesFromComponentDataArray(arrayWrapper);
            return !hasAnyFailed;
    }
    return false; 
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