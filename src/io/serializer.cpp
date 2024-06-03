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

bool Serializer::ParseJSONComponentData(ComponentData& data, const std::string& k, const nlohmann::json& jsonVal, const std::string& entityId) {
    auto dataVal = data.GetComponentDataValue(k);
    if (dataVal == nullptr)
        return false;
    
    auto it = std::find_if(GetJSONDeserializerList().begin(), GetJSONDeserializerList().end(), [&](const auto& s) {
        return s->CompareToComponentType(data.GetComponentDataValue(k));
    });
    if (it == GetJSONDeserializerList().end())
        return false;
    
    const auto& deserializer = *it;
    DeserializationArgs args(DeserializerType::COMPONENT_DATA);
    args.entityId = entityId;
    switch (dataVal->containerType) {
        case ComponentDataContainerType::SINGLE:
            args.ctData = &data;
            args.ctK = k;
            return (deserializer->fn)(args, jsonVal);
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
                if (!(deserializer->fn)(args, e))
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