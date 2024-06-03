#pragma once

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <magic_enum/magic_enum.hpp>

#include <latren/ec/serializable.h>
#include <latren/latren.h>

// this is just messed up, sorry
// may god have mercy on anyone who has chosen to read the following code
// -----------------------------
/*
    Well, I'll write a small documentation for all this deserialization crap

    So, for example, somehow we need to turn the following JSON:
        pos: [ 0.0, 2.0, 1.0 ]
    into:
        glm::vec3(0.0f, 2.0f, 1.0f)

    given that we know the type (glm::vec3, my magnificent "reflection" interface will do that),
    we need to call the deserializer of the corresponding type.

    These are defined in registertypes.cpp (Game::RegisterDeserializers).
*/
#include <iostream>
class ComponentData;
namespace Serializer {
    class DeserializationArgs;
    
    template <typename T>
    using DeserializerFunction = std::function<bool(DeserializationArgs&, T)>;
    typedef DeserializerFunction<const nlohmann::json&> JSONDeserializerFunction;

    enum class DeserializerType {
        COMPONENT_DATA,
        ANY_POINTER
    };

    class DeserializationArgs {
    public:
        DeserializerType type;

        // metadata
        std::string entityId;

        // SerializerType::COMPONENT_DATA
        ComponentData* ctData = nullptr;
        std::string ctK;

        // SerializerType::ANY_POINTER
        void* ptr;

        DeserializationArgs(const DeserializerType& t) : type(t) { }

        template <typename T>
        void Return(const T& val) {
            switch (type) {
                case DeserializerType::COMPONENT_DATA:
                    ctData->Set(ctK, val);
                    break;
                case DeserializerType::ANY_POINTER:
                    *static_cast<T*>(ptr) = val;
                    break;
            }
        }
    };

    template <typename F>
    class IValueDeserializer {
    public:
        F fn;
        virtual bool CompareToComponentType(std::shared_ptr<IComponentDataValue>) const = 0;
        virtual bool HasType(const std::type_info*) const = 0;
    };

    template <typename F, typename... T>
    class ValueDeserializer : public IValueDeserializer<F> {
    public:
        bool CompareToComponentType(std::shared_ptr<IComponentDataValue> d) const override {
            bool found = false;
            ([&] {
                ComponentType type = typeid(T);
                switch (d->containerType) {
                    case ComponentDataContainerType::SINGLE:
                        type = typeid(T);
                        break;
                    case ComponentDataContainerType::VECTOR:
                        type = typeid(std::vector<T>);
                        break;
                }
                if (d->type.hash_code() == type.hash_code()) {
                    found = true;
                    return;
                }
            } (), ...);
            return found;
        }
        bool HasType(const std::type_info* t) const override {
            bool found = false;
            ([&] {
                if (t->hash_code() == typeid(T).hash_code()) {
                    found = true;
                    return;
                }
            } (), ...);
            return found;
        }
    };
    
    template <typename F,  typename... T>
    void AddDeserializer(std::vector<std::shared_ptr<IValueDeserializer<F>>>& vec, const F& f) {
        auto count = std::count_if(vec.begin(), vec.end(), [&](const auto& s) {
            bool found = false;
            ([&] {
                if (s->HasType(&typeid(T))) {
                    found = true;
                    return;
                }
            } (), ...);
            return found;
        });
        if (count == 0) {
            auto v = std::make_shared<ValueDeserializer<F, T...>>();
            v->fn = f;
            vec.push_back(v);
        }
    }

    template <typename T, typename P, typename M>
    bool SetPointerValue(T* ptr, const P& param, const M& map) {
        auto it = std::find_if(map.begin(), map.end(), [&](const auto& s) {
            return s->HasType(&typeid(T));
        });
        if (it == map.end())
            return false;
        const auto& deserializer = *it;
        DeserializationArgs args(DeserializerType::ANY_POINTER);
        args.ptr = ptr;
        return (deserializer->fn)(args, param);
    }

    typedef std::vector<std::shared_ptr<IValueDeserializer<JSONDeserializerFunction>>> JSONDeserializerList;
    LATREN_API JSONDeserializerList& GetJSONDeserializerList();

    template <typename... T>
    void AddJSONDeserializer(const JSONDeserializerFunction& f) {
        AddDeserializer<JSONDeserializerFunction, T...>(GetJSONDeserializerList(), f);
    }

    template <typename T>
    void AddJSONEnumDeserializer() {
        AddDeserializer<JSONDeserializerFunction, T>(GetJSONDeserializerList(), [](Serializer::DeserializationArgs& args, const nlohmann::json& j) {
            if (!j.is_string())
                return false;
            auto e = magic_enum::enum_cast<T>((std::string) j);
            if (!e.has_value())
                return false;
            args.Return(e.value());
            return true;
        });
    }

    LATREN_API bool ParseJSONComponentData(ComponentData&, const std::string&, const nlohmann::json&, const std::string& = "");
    template <typename T>
    bool SetJSONPointerValue(T* ptr, const nlohmann::json& jsonVal) {
        return SetPointerValue<T>(ptr, jsonVal, GetJSONDeserializerList());
    }

    enum class SerializationStatus {
        OK,
        FAILED
    };

    // the destructor must be called from the dll too
    LATREN_API bool UseInputFileStream(const std::string&, std::ios_base::openmode, const std::function<bool(std::ifstream&)>&);
    LATREN_API bool UseOutputFileStream(const std::string&, std::ios_base::openmode, const std::function<bool(std::ofstream&)>&);

    template <typename T, std::ios_base::openmode Mode = static_cast<std::ios_base::openmode>(0)>
    class IFileSerializer {
    protected:
        std::string path_;
        SerializationStatus status_;
        T data_;
        virtual bool StreamRead(std::ifstream&) = 0;
        virtual bool StreamWrite(std::ofstream&) = 0;
    public:
        virtual ~IFileSerializer() = default;
        virtual void DeserializeFile() {
            status_ = UseInputFileStream(path_, Mode, [&](std::ifstream& f) {
                return this->StreamRead(f);
            }) ? SerializationStatus::OK : SerializationStatus::FAILED;
        }
        virtual void SetPath(const std::string& p) {
            path_ = p;
        }
        virtual void DeserializeFile(const std::string& p) {
            SetPath(p);
            DeserializeFile();
        }
        virtual void SerializeFile() {
            status_ = UseOutputFileStream(path_, Mode, [&](std::ofstream& f) {
                return this->StreamWrite(f);
            }) ? SerializationStatus::OK : SerializationStatus::FAILED;
        }
        virtual void SerializeFile(const std::string& p) {
            SetPath(p);
            SerializeFile();
        }
        virtual void SetData(const T& data) {
            data_ = data;
        }
        virtual T& GetData() { return data_; }
        virtual const T& GetData() const { return data_; }
        IFileSerializer(const std::string& p) : path_(p) { }
        IFileSerializer() = default;
        SerializationStatus GetReturnStatus() { return status_; }
        bool Success() { return GetReturnStatus() == SerializationStatus::OK; }
    };

    template <typename T>
    class SerializerItemInterface {
    typedef std::unordered_map<std::string, T> ItemsContainer;
    protected:
        ItemsContainer items_;
    public:
        const ItemsContainer& GetItems() const {
            return items_;
        }
        const T& GetItem(const std::string& item) const {
            return items_.at(item);
        }
        bool HasItem(const std::string& item) const {
            return items_.find(item) != items_.end();
        }
        // copy all items into an external container
        virtual void Register(ItemsContainer& container) {
            for (auto& item : items_) {
                container.insert(item);
            }
        }
    };

    class LATREN_API JSONFileSerializer : public IFileSerializer<nlohmann::json> {
    using IFileSerializer::IFileSerializer;
    protected:
        virtual bool ParseJSON() = 0;
        virtual bool StreamRead(std::ifstream&) override;
        virtual bool StreamWrite(std::ofstream&) override;
    public:
        virtual ~JSONFileSerializer() = default;
    };
};
