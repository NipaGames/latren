#pragma once

#include <latren/latren.h>
#include <latren/defines/opengl.h>
#include <vector>
#include <string>
#include <typeinfo>
#include <algorithm>

// hell yeah these again
// fuck the compiler!!!
namespace CFG {
    class ICFGField;
    template <typename>
    class CFGField;
    typedef CFGField<std::vector<ICFGField*>> CFGObject;
    struct CFGFileTemplate;
};

enum class MetaType {
    NONE,
    COMMENT,
    NEWLINE
};
// more whackass reflection
struct SerializableStructMemberData {
    std::string name;
    void* addr;
    const std::type_info* type;
    MetaType metaType;
    std::string metaData;
};

struct LATREN_API SerializableStruct {
private:
    int _metaCounter = 0;
protected:
    std::vector<SerializableStructMemberData> members_;
    void AddMember(const SerializableStructMemberData&);
    template <typename T>
    T MemberField(const std::string& name, T* addr, const T& val) {
        AddMember({ name, addr, &typeid(T), MetaType::NONE });
        return val;
    }
    void* AddNewline();
    void* AddComment(const std::string&);
public:
    const SerializableStructMemberData& GetMemberData(const std::string&);
    void* GetMemberAddress(const std::string&);
    template <typename T>
    T& operator=(const SerializableStruct& copyFrom) {
        members_.clear();
        for (const auto& m : copyFrom.GetMembers()) {
            if (m.addr != nullptr && GetMemberAddress(m.name) != nullptr) {
                *reinterpret_cast<char*>(GetMemberAddress(m.name)) = *reinterpret_cast<char*>(m.addr);
            }
            else {
                auto it = std::find_if(members_.begin(), members_.end(), [&](const auto& sm) { return sm.name == m.name; });
                if (it != members_.end()) {
                    *it = m;
                }
                else {
                    members_.push_back(m);
                }
            }
        }
        return *this;
    }
    template <typename T>
    T* GetMemberAddress(const std::string& name) {
        return static_cast<T*>(this->GetMemberAddress(name));
    }
    CFG::CFGObject* CFGSerialize() const;
    void CFGDeserialize(const CFG::CFGObject*);
    const std::vector<SerializableStructMemberData>& GetMembers() const { return members_; }
    CFG::CFGFileTemplate CreateCFGTemplate() const;
};

#define _GENERATE_META_NAME(x, y) x##y
#define GENERATE_META_NAME(y) _GENERATE_META_NAME(_meta_ln, y)

#define DATA_FIELD(T, name, val) T name = this->MemberField(#name, &name, val)
#define META_NEWLINE() void* GENERATE_META_NAME(__LINE__) = this->AddNewline()
#define META_COMMENT(comment) void* GENERATE_META_NAME(__LINE__) = this->AddComment(comment)