#pragma once

#include "../serializer.h"
#include <vector>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <magic_enum/magic_enum.hpp>

namespace CFG {
    /*
    * CFGFieldType::NUMBER is kind of weird, you could argue
    * that a FLOAT will do in any case a NUMBER would, but they
    * are actually a bit different.
    * Both of them will accept INTEGER inputs, but FLOAT will
    * cast INTEGERs into floats at the validation stage. However,
    * NUMBER will keep the original type. NUMBER's type can be checked
    * with HasType<float/int>.
    * 
    * now, what i don't know is if there's actually any practical uses
    * for NUMBER where FLOAT wouldn't work since you'll have to cast it
    * anyway when retrieving the value.
    */
    enum class CFGFieldType {
        STRING,
        NUMBER,
        INTEGER,
        FLOAT,
        ARRAY,
        STRUCT,
        STRUCT_MEMBER_REQUIRED,
        RAW,
        CUSTOM
    };
    #define CFG_ARRAY(type) CFG::CFGFieldType::ARRAY, type
    #define CFG_REQUIRE(type) CFG::CFGFieldType::STRUCT_MEMBER_REQUIRED, type
    #define CFG_STRUCT(...) CFG::CFGFieldType::STRUCT, __VA_ARGS__
    #define CFG_VEC2(type) CFG_STRUCT(CFG_REQUIRE(type), CFG_REQUIRE(type))

    LATREN_API bool IsValidType(CFGFieldType, CFGFieldType, bool = true);
    
    enum class CFGStringLiteral {
        APOSTROPHES,
        QUOTES
    };
    struct CFGFormatting {
        int indents;
        // actually the format doesn't even support commas yet lol
        // at least not in the regex
        CFGStringLiteral stringLiteral;
    };
    const CFGFormatting STANDARD_FORMATTING = {
        2,
        CFGStringLiteral::APOSTROPHES
    };
    
    struct CFGStructuredField {
        std::string name;
        std::vector<CFGFieldType> types;
        bool required = false;
        bool isObject = false;
        std::vector<CFGStructuredField> objectParams;
        CFGStructuredField(const std::string& n, bool r, const std::vector<CFGFieldType>& t) : name(n), required(r), types(t) { }
        CFGStructuredField(const std::string& n, const std::vector<CFGFieldType>& t) : CFGStructuredField(n, false, t) { }
        template <typename... Field>
        CFGStructuredField(const std::string& n, bool r, const Field&... t) : name(n), required(r) {
            (types.push_back(t), ...);
        }
        template <typename... Field>
        CFGStructuredField(const std::string& n, const Field&... t) : CFGStructuredField(n, false, t...) { }
        CFGStructuredField(const std::string& n, bool r, const std::vector<CFGStructuredField>& v) :
            isObject(true),
            name(n),
            required(r),
            objectParams(v)
        { }
        CFGStructuredField(const std::string& n, const std::vector<CFGStructuredField>& v) : CFGStructuredField(n, false, v) { }
    };

    inline CFGStructuredField Mandatory(const std::string& n, const std::vector<CFGStructuredField>& v) {
        return CFGStructuredField { n, true, v };
    }
    inline CFGStructuredField Mandatory(const std::string& n, const std::vector<CFGFieldType>& v) {
        return CFGStructuredField { n, true, v };
    }
    template <typename... Field>
    inline CFGStructuredField Mandatory(const std::string& n, const Field&... t) {
        return CFGStructuredField { n, true, t... };
    }

    inline CFGStructuredField Optional(const std::string& n, const std::vector<CFGStructuredField>& v) {
        return CFGStructuredField { n, false, v };
    }
    inline CFGStructuredField Optional(const std::string& n, const std::vector<CFGFieldType>& v) {
        return CFGStructuredField { n, false, v };
    }
    template <typename... Field>
    inline CFGStructuredField Optional(const std::string& n, const Field&... t) {
        return CFGStructuredField { n, false, t... };
    }

    template <typename T>
    class CFGField;

    class ICFGField {
    public:
        std::optional<std::string> name;
        CFGFieldType type;
        std::string typeAnnotation;
        CFGField<std::vector<ICFGField*>>* parent = nullptr;
        bool automaticallyCreated = false;
        ICFGField() = default;
        ICFGField(CFGFieldType t) : type(t) { }
        ICFGField(const std::optional<std::string>& n, CFGFieldType t) : name(n), type(t) { }
        virtual bool HasType(const std::type_info&) const { return false; }
        template <typename T>
        const T& GetValue() const {
            return dynamic_cast<const CFGField<T>*>(this)->GetValue();
        }
        template <typename Type>
        bool HasType() const {
            return HasType(typeid(Type));
        }
        virtual void CopyValueTo(void*) const { }
    };

    template <typename T>
    class CFGField : public ICFGField {
    public:
        using ICFGField::ICFGField;
        T value;
        CFGField(const T& val) : value(val) { }
        const std::vector<ICFGField*>& GetItems() const { return static_cast<const std::vector<ICFGField*>&>(value); }
        std::vector<ICFGField*>& GetItems() { return static_cast<std::vector<ICFGField*>&>(value); }
        void AddItem(ICFGField* f) {
            static_cast<std::vector<ICFGField*>&>(value).push_back(f);
            f->parent = this;
        }
        const ICFGField* GetItemByName(const std::string& name) const {
            auto it = std::find_if(GetItems().begin(), GetItems().end(), [&](const ICFGField* field) {
                if (field == nullptr)
                    return false;
                return name == field->name;
            });
            if (it == GetItems().end())
                return nullptr;
            return *it;
        }
        template <typename F>
        const CFGField<F>* GetItemByName(const std::string& name) const {
            return static_cast<const CFGField<F>*>(GetItemByName(name));
        }
        const ICFGField* GetItemByIndex(int i) const {
            if (i >= GetItems().size())
                return nullptr;
            return GetItems().at(i);
        }
        template <typename F>
        const CFGField<F>* GetItemByIndex(int i) const {
            return static_cast<const CFGField<F>*>(GetItemByIndex(i));
        }
        const CFGField<std::vector<ICFGField*>>* GetObjectByName(const std::string& name) const {
            return GetItemByName<std::vector<CFG::ICFGField*>>(name);
        }
        const T& GetValue() const {
            return value;
        }
        template <typename F>
        std::vector<F> Values() const {
            std::vector<F> vec;
            for (const ICFGField* item : GetItems()) {
                vec.push_back(static_cast<const CFGField<F>*>(item)->GetValue());
            }
            return vec;
        }
        template <typename F>
        std::vector<F> GetItemValues(const std::string& name) const {
            const CFGObject* obj = GetObjectByName(name);
            if (obj != nullptr)
                return obj->Values<F>();
            else
                return std::vector<F>();
        }
        void CopyValueTo(void* ptr) const override {
            if (ptr == nullptr)
                return;
            *static_cast<T*>(ptr) = value;
        }
        bool HasType(const std::type_info& type) const override {
            return typeid(T).hash_code() == type.hash_code();
        }
    };

    typedef CFGField<std::vector<ICFGField*>> CFGObject;

    template <typename T>
    ICFGField* CreateNewCFGField(const void* ptr) {
        CFGField<T>* field = new CFGField<T>();
        if (ptr != nullptr)
            field->value = *static_cast<const T*>(ptr);
        
        if constexpr (std::is_same_v<T, std::string>)
            field->type = CFGFieldType::STRING;
        else if constexpr (std::is_same_v<T, float>)
            field->type = CFGFieldType::FLOAT;
        else if constexpr (std::is_same_v<T, int>)
            field->type = CFGFieldType::INTEGER;
        // could be any non-primitive really but we'll just assume it's a struct by default
        else if constexpr (std::is_same_v<T, ICFGField*>)
            field->type = CFGFieldType::STRUCT;
        return field;
    }

    template <typename T>
    ICFGField* CreateNewCFGField(const ICFGField* copyFrom = nullptr) {
        const void* ptr = nullptr;
        if (copyFrom != nullptr)
            ptr = &static_cast<const CFGField<T>*>(copyFrom)->value;
        CFGField<T>* field = static_cast<CFGField<T>*>(CreateNewCFGField<T>(ptr));
        // fuck copy constructors and assignment operators
        if (copyFrom != nullptr) {
            field->name = copyFrom->name;
            field->automaticallyCreated = copyFrom->automaticallyCreated;
            field->type = copyFrom->type;
        }
        return field;
    }
    template <typename T>
    ICFGField* CreateNewCFGField(CFGFieldType type, const T* copyFrom) {
        ICFGField* field;
        switch (type) {
            case CFGFieldType::STRING:
                field = CreateNewCFGField<std::string>(copyFrom);
                break;
            case CFGFieldType::NUMBER:
            case CFGFieldType::FLOAT:
                field = CreateNewCFGField<float>(copyFrom);
                break;
            case CFGFieldType::INTEGER:
                field = CreateNewCFGField<int>(copyFrom);
                break;
            case CFGFieldType::ARRAY:
                field = CreateNewCFGField<std::vector<ICFGField*>>(copyFrom);
                break;
            case CFGFieldType::STRUCT:
                field = CreateNewCFGField<std::vector<ICFGField*>>(copyFrom);
                break;
        }
        field->type = type;
        return field;
    }
    LATREN_API ICFGField* CreateNewCFGField(CFGFieldType);
    LATREN_API ICFGField* CreateNewCFGField(const ICFGField*);

    // serialize cfg object to a stringstream
    LATREN_API void Dump(const CFGObject*, std::stringstream&, const CFGFormatting& = STANDARD_FORMATTING);
    // serialize cfg object to a string
    LATREN_API std::string Dump(const CFGObject*, const CFGFormatting& = STANDARD_FORMATTING);

    template <typename T>
    struct CFGParseTreeNode {
        std::vector<CFGParseTreeNode<T>*> children;
        CFGParseTreeNode<T>* parent = nullptr;
        T value;
        ~CFGParseTreeNode() {
            children.clear();
            if constexpr (std::is_pointer<T>::value) {
                delete value;
            }
        }
        void AddChild(CFGParseTreeNode* child) {
            children.push_back(child);
            child->parent = this;
        }
    };

    #define CFG_TYPE_PAIR(T, CFGT) { typeid(T).hash_code(), { CFGT } }
    typedef std::unordered_map<size_t, const std::vector<CFG::CFGFieldType>> CFGTypeMap;
    extern const CFGTypeMap CFG_TYPES_;
    const CFGTypeMap& GetCFGTypeMap();

    typedef std::vector<CFGStructuredField> CFGStructuredFields;
    typedef std::unordered_map<std::string, std::vector<CFGFieldType>> CFGCustomTypes;

    struct CFGFileTemplate {
        CFGStructuredFields fields;
        CFGCustomTypes types;
    };

    class CFGFileTemplateFactory {
    public:
        virtual CFGStructuredFields DefineFields() const { return { }; }
        virtual CFGCustomTypes DefineCustomTypes() const { return { }; }
        CFGFileTemplate CreateTemplate() const { return { DefineFields(), DefineCustomTypes() }; }
        operator CFGFileTemplate() { return CreateTemplate(); }
    };
}

namespace Serializer {
    class CFGSerializer : public IFileSerializer<CFG::CFGObject*> {
    private:
        CFG::CFGFileTemplate fileTemplate_;
    public:
        LATREN_API ~CFGSerializer();
        CFGSerializer(const CFG::CFGFileTemplate& f) : fileTemplate_(f) { }
        CFGSerializer() = default;
        LATREN_API static CFG::CFGObject* ParseCFG(std::stringstream&, const CFG::CFGFileTemplate&);
        LATREN_API bool Validate(const CFG::CFGFileTemplate&);
        LATREN_API bool StreamRead(std::ifstream&) override;
        LATREN_API bool StreamWrite(std::ofstream&) override;
        void SetCFGFileTemplate(const CFG::CFGFileTemplate& f) { fileTemplate_ = f; }
    };
}


namespace Serializer {
    typedef SerializerFunction<const CFG::ICFGField*> CFGSerializerFunction;
    typedef std::vector<std::shared_ptr<IValueSerializer<CFGSerializerFunction>>> CFGSerializerList;
    CFGSerializerList& GetCFGSerializerList();
    
    template <typename... T>
    void* AddCFGSerializer(const CFGSerializerFunction& f) {
        return AddSerializer<CFGSerializerFunction, T...>(GetCFGSerializerList(), f);
    }

    template <typename T>
    bool SetCFGPointerValue(T* ptr, const CFG::ICFGField* field) {
        return SetPointerValue<T>(ptr, field, GetCFGSerializerList());
    }
};