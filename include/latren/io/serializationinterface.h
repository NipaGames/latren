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

namespace Serialization {
    class DeserializationContext;
    
    template <typename... Params>
    using DeserializerFunction = std::function<bool(DeserializationContext&, Params...)>;
    typedef DeserializerFunction<const nlohmann::json&> JSONDeserializerFunction;


    class ISerializableFieldValueWrapper {
    public:
        virtual void CopyValueTo(void*) const = 0;
    };
    template <typename T>
    class SerializableFieldValueWrapper : public ISerializableFieldValueWrapper {
    public:
        T value;
        SerializableFieldValueWrapper() = default;
        SerializableFieldValueWrapper(const T& val) : value(val) { }
        SerializableFieldValueWrapper(T&& val) : value(std::move(val)) { }

        void CopyValueTo(void* ptr) const override {
            *(T*) ptr = value;
        }
    };
    class SerializableFieldValue {
    public:
        std::shared_ptr<ISerializableFieldValueWrapper> value;
        std::type_index type;
        ComponentDataContainerType containerType;
    };

    typedef std::unordered_map<std::string, SerializableFieldValue> SerializableFieldValueMap;
    struct TypedComponentData {
        ComponentType type;
        Serialization::SerializableFieldValueMap fields;
    };

    class DeserializationContext {
    private:
        int itemIndex_ = -1;
        std::size_t itemCount_ = 0;
        std::any itemContainer_;
    public:
        enum class DeserializerType {
            FIELD_VALUE,
            ANY_POINTER
        };
        DeserializerType type;

        // metadata
        std::string entityId;

        // SerializerType::FIELD_VALUE
        SerializableFieldValue* field = nullptr;

        // SerializerType::ANY_POINTER
        void* ptr;

        template <typename... Params>
        bool Deserialize(const DeserializerFunction<Params...>& fn, Params... params) {
            itemIndex_ = -1;
            return fn(*this, params...);
        }

        template <typename Param>
        bool DeserializeVector(const DeserializerFunction<Param>& fn, const std::vector<std::decay_t<Param>>& paramList) {
            itemCount_ = paramList.size();
            itemIndex_ = 0;
            for (const auto& params : paramList) {
                if (!fn(*this, params))
                    return false;
                itemIndex_++;
            }
            return true;
        }
        template <typename... Params>
        bool DeserializeVector(const DeserializerFunction<Params...>& fn, const std::vector<std::tuple<std::decay_t<Params>...>>& paramList) {
            itemCount_ = paramList.size();
            itemIndex_ = 0;
            for (const auto& paramsTuple : paramList) {
                auto params = std::tuple_cat(std::make_tuple(*this), paramsTuple);
                if (!std::apply(fn, params))
                    return false;
                itemIndex_++;
            }
            return true;
        }

        template <typename T>
        void Return(const T& val) {
            if constexpr (ComponentDataContainerTypeDeduction<T>::TYPE != ComponentDataContainerType::VECTOR) {
                if (itemIndex_ != -1) {
                    // create
                    if (itemIndex_ == 0) {
                        itemContainer_ = std::vector<T>(itemCount_);
                    }
                    std::vector<T>& vec = std::any_cast<std::vector<T>&>(itemContainer_);
                    vec[itemIndex_] = val;
                    // assign
                    if (itemIndex_ == vec.size() - 1) {
                        itemIndex_ = -1;
                        this->Return<std::vector<T>>(vec);
                    }
                    return;
                }
            }
            switch (type) {
                case DeserializerType::FIELD_VALUE:
                    field->value = std::make_shared<SerializableFieldValueWrapper<T>>(val);
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
        virtual bool CompareToComponentType(const SerializableFieldValue&) const = 0;
        virtual bool HasType(const std::type_info*) const = 0;
    };

    template <typename F, typename... T>
    class ValueDeserializer : public IValueDeserializer<F> {
    public:
        bool CompareToComponentType(const SerializableFieldValue& field) const override {
            bool found = false;
            ([&] {
                ComponentType type = typeid(T);
                switch (field.containerType) {
                    case ComponentDataContainerType::SINGLE:
                        type = typeid(T);
                        break;
                    case ComponentDataContainerType::VECTOR:
                        type = typeid(std::vector<T>);
                        break;
                }
                if (field.type.hash_code() == type.hash_code()) {
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
    void AssignDeserializer(std::vector<std::shared_ptr<IValueDeserializer<F>>>& vec, const F& f) {
        auto v = std::make_shared<ValueDeserializer<F, T...>>();
        v->fn = f;
        vec.push_back(v);
    }

    template <typename T, typename P, typename M>
    bool SetPointerValue(T* ptr, const P& param, const M& map) {
        auto it = std::find_if(map.rbegin(), map.rend(), [&](const auto& s) {
            return s->HasType(&typeid(T));
        });
        if (it == map.rend())
            return false;
        const auto& deserializer = *it;
        DeserializationContext context;
        context.type = DeserializationContext::DeserializerType::ANY_POINTER;
        context.ptr = ptr;
        return context.Deserialize<const P&>(deserializer->fn, param);
    }

    typedef std::vector<std::shared_ptr<IValueDeserializer<JSONDeserializerFunction>>> JSONDeserializerList;
    LATREN_API JSONDeserializerList& GetJSONDeserializerList();

    template <typename... T>
    void AssignJSONDeserializer(const JSONDeserializerFunction& f) {
        AssignDeserializer<JSONDeserializerFunction, T...>(GetJSONDeserializerList(), f);
    }

    template <typename T>
    void AssignJSONEnumDeserializer() {
        AssignDeserializer<JSONDeserializerFunction, T>(GetJSONDeserializerList(), [](Serialization::DeserializationContext& args, const nlohmann::json& j) {
            if (!j.is_string())
                return false;
            auto e = magic_enum::enum_cast<T>(j.get<std::string>());
            if (!e.has_value())
                return false;
            args.Return(e.value());
            return true;
        });
    }

    template <typename T>
    bool SetJSONPointerValue(T *ptr, const nlohmann::json &jsonVal) {
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

    class JSONComponentDeserializer {
    public:
        virtual bool ParseJSONComponentData(SerializableFieldValueMap&, const std::string&, const nlohmann::json&, const std::string& = "") const;
        virtual bool DeserializeComponentDataFromJSON(SerializableFieldValueMap&, const nlohmann::json&, const std::string& = "") const;
        virtual std::optional<TypedComponentData> DeserializeComponent(const std::string&, const nlohmann::json&) const;
        virtual bool DeserializeComponents(std::vector<Serialization::TypedComponentData>&, const nlohmann::json&, const std::string& = "") const;
    };
};
