#include <latren/io/files/cfg.h>

#include <algorithm>
#include <sstream>
#include <stack>
#include <queue>
#include <sstream>
#include <magic_enum/magic_enum.hpp>

using namespace Serializer;
using namespace CFG;

std::string StrReplace(const std::string& str, const std::string& from, const std::string& to) {
    std::string s = str;
    size_t pos;
    while ((pos = s.find(from)) != std::string::npos) {
        s.replace(pos, from.length(), to);
    }
    return s;
}
void TrimLeadingWhitespace(std::string& str) {
    while (!str.empty() && std::isspace(str.front())) {
        str.erase(0, 1);
    }
}
void TrimTrailingWhitespace(std::string& str) {
    while (!str.empty() && std::isspace(str.back())) {
        str.pop_back();
    }
}
void TrimWhitespace(std::string& str) {
    TrimLeadingWhitespace(str);
    TrimTrailingWhitespace(str);
}

bool ShouldParseAsNumber(const std::string& val, size_t first = 0) {
    if (val.length() <= first)
        return false;
    if (std::isdigit(val.at(first)))
        return true;
    if (val.at(first) == '-') {
        if (val.length() < first + 2)
            return false;
        if (val.at(first + 1) == '-')
            return false;
        if (val.at(first + 1) == '.')
            return val.length() > first + 2 && std::isdigit(val.at(2));
        return std::isdigit(val.at(first + 1));
    }
    if (val.at(first) == '.') {
        if (val.length() < first + 2)
            return false;
        return std::isdigit(val.at(first + 1));
    }
    return false;
}
bool ShouldParseAsString(const std::string& val, size_t first = 0) {
    if (val.length() <= first)
        return false;
    return std::isalpha(val.at(first)) || val.at(first) == '"' || val.at(first) == '\'';
}

std::optional<std::string> ParseString(const std::string& val, size_t first, size_t last, size_t* next, bool needsToBeWhole = false) {
    // with literals
    if (val.at(first) == '\'' || val.at(first) == '"') {
        char lit = val.at(first);
        size_t endLit = std::string::npos;
        for (size_t i = first + 1; i <= last; i++) {
            if (val.at(i) == lit) {
                endLit = i;
                break;
            }
        }
        if (endLit == std::string::npos)
            return std::nullopt;
        if (needsToBeWhole && endLit < last)
            return std::nullopt;
        if (next != nullptr)
            *next = endLit + 1;
        return val.substr(first + 1, endLit - first - 1);
    }
    // without literals
    else {
        for (size_t i = first + 1; i <= last; i++) {
            if (!(std::isalnum(val.at(i)) || val.at(i) == '_')) {
                if (!needsToBeWhole && std::isspace(val.at(i))) {
                    if (next != nullptr)
                        *next = first + i;
                    return val.substr(first, i - first);
                }
                return std::nullopt;
            }
        }
        if (next != nullptr)
            *next = last + 1;
        return val.substr(first, last - first + 1);
    }
    return std::nullopt;
}
std::optional<std::string> ParseString(const std::string& val, size_t first = 0, bool needsToBeWhole = false) {
    if (val.empty())
        return std::nullopt;
    return ParseString(val, first, val.length() - 1, nullptr, needsToBeWhole);
}

size_t RNextWithoutSpace(const std::string& val, size_t from) {
    size_t pos = 0;
    for (size_t i = from; i != 0; i--) {
        if (!std::isspace(val.at(i))) {
            pos = i;
            break;
        }
    }
    return pos;
}

size_t GetFirstNotInStringLiteral(const std::string& val, char find, size_t first = 0) {
    bool inLit = false;
    char lit;
    for (size_t i = first; i < val.length(); i++) {
        char c = val.at(i);
        if (c == find && !inLit) {
            return i;
        }
        else if (c == '\'' || c == '"') {
            if (inLit && c == lit)
                inLit = false;
            else {
                inLit = true;
                lit = c;
            }
        }
    }
    return std::string::npos;
}

ICFGField* ParseFieldValue(const std::string& val, size_t first = 0, size_t* next = nullptr) {
    if (next != nullptr)
        *next = std::string::npos;
    // number
    if (val.length() <= first)
        return nullptr;
    if (ShouldParseAsNumber(val, first)) {
        ICFGField* thisNode = nullptr;
        bool isFloat = false;
        size_t last = val.length();
        size_t fstnum = first;
        if (val.at(fstnum) == '-')
            fstnum++;
        for (size_t i = fstnum; i < val.length(); i++) {
            if (val.at(i) == '.') {
                if (!isFloat)
                    isFloat = true;
                else
                    return nullptr;
            }
            else if (!std::isdigit(val.at(i))) {
                last = i;
                break;
            }
        }
        try {
            if (isFloat) {
                CFGField<float>* floatNode = new CFGField<float>(CFGFieldType::FLOAT);
                floatNode->value = std::stof(val.substr(first, last));
                thisNode = floatNode;
            }
            else {
                CFGField<int>* intNode = new CFGField<int>(CFGFieldType::INTEGER);
                intNode->value = std::stoi(val.substr(first, last));
                thisNode = intNode;
            }
        }
        catch (const std::exception&) {
            return nullptr;
        }
        if (next != nullptr)
            *next = last;
        return thisNode;
    }
    // string
    if (ShouldParseAsString(val, first)) {
        CFGField<std::string>* thisNode = new CFGField<std::string>(CFGFieldType::STRING);
        std::optional<std::string> strVal = ParseString(val, first, val.length() - 1, next);
        if (!strVal.has_value())
            return nullptr;
        strVal = StrReplace(strVal.value(), "\\q", "\"");
        strVal = StrReplace(strVal.value(), "\\a", "'");
        thisNode->value = strVal.value();
        return thisNode;
    }
    return nullptr;
}

const std::unordered_set<char> TYPE_ANNOTATION_SPECIAL_SYMBOLS = {
    '[', ']', '{', '}', '_', ','
};
bool IsValidTypeAnnotationSymbol(char c) {
    return std::isspace(c) || std::isalnum(c) || TYPE_ANNOTATION_SPECIAL_SYMBOLS.find(c) != TYPE_ANNOTATION_SPECIAL_SYMBOLS.end();
}

// read type annotations from right starting from pos, return empty if no annotations were found
std::string ReadTypeAnnotation(const std::string& str, size_t& pos) {
    size_t len = 0;
    bool isTyped = false;
    for (int i = (int) pos; i >= 0; i--, len++) {
        char c = str.at(i);
        if (c == ':') {
            isTyped = true;
            break;
        }
        if (!IsValidTypeAnnotationSymbol(c)) {
            break;
        }
    }
    if (isTyped) {
        std::string typeAnnotation = str.substr(pos - len + 1, len);
        TrimWhitespace(typeAnnotation);
        // remove spaces when necessary
        for (size_t i = 0; i < typeAnnotation.length(); i++) {
            if (std::isspace(typeAnnotation.at(i))) {
                // this should be safe since the whitespace is trimmed first regardless
                if (TYPE_ANNOTATION_SPECIAL_SYMBOLS.find(typeAnnotation.at(i - 1)) != TYPE_ANNOTATION_SPECIAL_SYMBOLS.end()) {
                    typeAnnotation.erase(i, 1);
                    i -= 1;
                }
                else if (TYPE_ANNOTATION_SPECIAL_SYMBOLS.find(typeAnnotation.at(i + 1)) != TYPE_ANNOTATION_SPECIAL_SYMBOLS.end()) {
                    typeAnnotation.erase(i, 1);
                    i -= 2;
                }
            }
        }
        if (pos - len > 0)
            pos = RNextWithoutSpace(str, pos - len - 1);
        else
            pos = 0;
        return typeAnnotation;
    }
    return "";
}

std::unordered_map<std::string, CFGFieldType> CFG_PRIMITIVE_NAMES = {
    { "Str", CFGFieldType::STRING },
    { "Int", CFGFieldType::INTEGER },
    { "Flt", CFGFieldType::FLOAT }
};

std::optional<std::vector<CFGFieldType>> ParseTypeAnnotation(const std::string& str, bool allowNonPrimitives = false, const CFGCustomTypes& ctypes = { }) {
    std::vector<CFGFieldType> types;
    int structIndents = 0;
    int arrayIndents = 0;
    bool awaitingTypeAfterComma = false;
    for (size_t pos = 0; pos < str.length(); pos++) {
        char c = str.at(pos);
        switch (c) {
            case '[':
                structIndents++;
                types.push_back(CFGFieldType::ARRAY);
                continue;
            case ']':
                structIndents--;
                continue;
            case '{':
                arrayIndents++;
                types.push_back(CFGFieldType::STRUCT);
                continue;
            case '}':
                arrayIndents--;
                continue;
        }
        if (std::isalpha(c) || c == '_') {
            // fuck it
            size_t end = str.find_first_not_of(
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789_",
            pos);
            if (end == std::string::npos)
                end = str.length();
            std::string type = str.substr(pos, end - pos);
            pos = end - 1;

            auto primitive = CFG_PRIMITIVE_NAMES.find(type);
            if (primitive == CFG_PRIMITIVE_NAMES.end()) {
                if (!allowNonPrimitives)
                    return std::nullopt;
                CFGCustomTypes::const_iterator custom;
                if (ctypes.empty() || (custom = ctypes.find(type)) == ctypes.end())
                    types.push_back(CFGFieldType::CUSTOM);
                else
                    types.insert(types.end(), custom->second.begin(), custom->second.end());
            }
            else {
                types.push_back(primitive->second);
            }
            awaitingTypeAfterComma = false;
        }
        else if (c == ',') {
            awaitingTypeAfterComma = true;
        }
        else {
            return std::nullopt;
        }
    }
    if (arrayIndents != 0 || structIndents != 0 || awaitingTypeAfterComma) {
        return std::nullopt;
    }
    return types;
}

CFGParseTreeNode<std::string>* CreateIndentationTree(std::stringstream& buffer) {
    std::string line;
    int currentIndent = 0;
    CFGParseTreeNode<std::string>* root = new CFGParseTreeNode<std::string>();
    CFGParseTreeNode<std::string>* n = root;
    while (std::getline(buffer, line, '\n')) {
        std::string expr = line;
        int indent = 0;
        while (!expr.empty() && (expr.at(0) == ' ' || expr.at(0) == '\t')) {
            expr = expr.substr(1);
            indent++;
        }
        // comments
        bool inStrLiteral = false;
        for (int i = 0; i < expr.length(); i++) {
            if (expr.at(i) == '\'' || expr.at(i) == '"') {
                // i can't remember if this shit even supports escapes but here we go anyway
                if (i == 0 || expr.at(i - 1) != '\\')
                    inStrLiteral = !inStrLiteral;
            }
            else if (expr.at(i) == '#' && !inStrLiteral) {
                expr.resize(i);
                break;
            }
        }
        // remove trailing whitespace
        while (!expr.empty() && (expr.at(expr.length() - 1) == ' ' || expr.at(expr.length() - 1) == '\t')) {
            expr.pop_back();
        }
        if (expr.empty())
            continue;
        if (indent < currentIndent) {
            currentIndent = indent;
            if (n->parent == nullptr) {
                delete root;
                return nullptr;
            }
            n = n->parent;
        }
        else if (indent > currentIndent) {
            currentIndent = indent;
            n = n->children.back();
        }
        CFGParseTreeNode<std::string>* newN = new CFGParseTreeNode<std::string>();
        newN->value = expr;
        n->AddChild(newN);
    }
    return root;
}

ICFGField* ParseIndentTreeNodes(CFGParseTreeNode<std::string>* node, bool isRoot = false) {
    std::optional<std::string> name = std::nullopt;
    std::string val = node->value;
    std::string typeAnnotation = "";
    std::vector<CFGFieldType> types;

    size_t equals = GetFirstNotInStringLiteral(node->value, '=');
    
    size_t lastOfName = 0;
    if (equals != std::string::npos && equals > 0) {
        lastOfName = RNextWithoutSpace(node->value, equals - 1);
        // type annotation before equals
        typeAnnotation = ReadTypeAnnotation(node->value, lastOfName);
        if (!typeAnnotation.empty()) {
            auto parsedTypes = ParseTypeAnnotation(typeAnnotation, true);
            if (!parsedTypes.has_value())
                return nullptr;
            types = parsedTypes.value();
        }
    }

    // object or array (inline value left null)
    if (isRoot || equals == node->value.length() - 1) {
        if (!isRoot && lastOfName > 0) {
            std::optional<std::string> opt = ParseString(node->value, 0, lastOfName, nullptr, true);
            if (!opt.has_value())
                return nullptr;
            name = opt.value();
        }
        CFGObject* thisNode = new CFGObject{ name, CFGFieldType::ARRAY };
        thisNode->typeAnnotation = typeAnnotation;
        for (auto* child : node->children) {
            ICFGField* item = ParseIndentTreeNodes(child);
            if (item != nullptr)
                thisNode->AddItem(item);
            else {
                delete thisNode;
                return nullptr;
            }
        }
        return thisNode;
    }

    // field name
    if (equals != std::string::npos) {
        val = node->value.substr(equals + 1);
        TrimLeadingWhitespace(val);
        if (ShouldParseAsString(node->value)) {
            std::optional<std::string> opt = ParseString(node->value, 0, lastOfName, nullptr, true);
            if (!opt.has_value())
                return nullptr;
            name = opt.value();
        }
    }

    size_t valLast = val.length() - 1;
    // type annotations after equals
    if (typeAnnotation.empty()) {
        typeAnnotation = ReadTypeAnnotation(val, valLast);
        if (!typeAnnotation.empty()) {
            auto parsedTypes = ParseTypeAnnotation(typeAnnotation, true);
            if (!parsedTypes.has_value())
                return nullptr;
            types = parsedTypes.value();
        }
    }

    // struct elements
    std::vector<ICFGField*> fields;
    size_t next = 0;
    while (next != std::string::npos && next <= valLast) {
        ICFGField* f = ParseFieldValue(val, next, &next);
        if (f != nullptr)
            fields.push_back(f);
        else {
            for (ICFGField* f : fields)
                delete f;
            return nullptr;
        }
        while (next <= valLast && std::isspace(val.at(next)))
            next++;
    }
    if (fields.empty())
        return nullptr;
    
    ICFGField* ret = nullptr;
    if (fields.size() > 1) {
        CFGObject* thisNode = new CFGObject{ name, CFGFieldType::STRUCT };
        for (ICFGField* field : fields) {
            thisNode->AddItem(field);
        }
        ret = thisNode;
    }
    else {
        ret = fields.front();
        ret->name = name;
    }
    if (ret != nullptr) {
        ret->typeAnnotation = typeAnnotation;
        return ret;
    }
    for (ICFGField* f : fields)
        delete f;
    return nullptr;
}

ICFGField* CFG::CreateNewCFGField(const ICFGField* copyFrom) {
    return CreateNewCFGField(copyFrom->type, copyFrom);
}
ICFGField* CFG::CreateNewCFGField(CFGFieldType t) {
    return CreateNewCFGField<ICFGField>(t, nullptr);
}

bool ValidateCFGFieldType(ICFGField*, std::vector<CFGFieldType>);

bool CFG::IsValidType(CFGFieldType in, CFGFieldType expected, bool allowCasts) {
    if (in == expected)
        return true;
    if (!allowCasts)
        return false;
    switch (expected) {
        case CFGFieldType::NUMBER:
            return in == CFGFieldType::INTEGER || in == CFGFieldType::FLOAT;
        case CFGFieldType::FLOAT:
            return in == CFGFieldType::INTEGER || in == CFGFieldType::NUMBER;
    }
    return false;
}

bool ValidateSubItems(ICFGField* node, const std::vector<CFGFieldType>& types) {
    const CFGObject* arrayObject = static_cast<const CFGObject*>(node);
    // yeah could just as well be a raw loop but will keep this for now
    auto validate = [&](CFG::ICFGField* f) {
        return !ValidateCFGFieldType(f, { types.begin() + 1, types.end() });
    };
    return !std::any_of(arrayObject->GetItems().begin(), arrayObject->GetItems().end(), validate);
}

CFGField<float>* CastIntegerToFloat(const CFGField<int>* intField) {
    CFGField<float>* floatField = dynamic_cast<CFGField<float>*>(CreateNewCFGField<float>(intField));
    floatField->type = CFGFieldType::FLOAT;
    floatField->parent = intField->parent;
    if (intField->parent != nullptr) {
        auto& otherItems = const_cast<CFGField<int>*>(intField)->parent->GetItems();
        auto it = std::find(otherItems.begin(), otherItems.end(), intField);
        if (it != otherItems.end())
            *it = floatField;
    }
    delete intField;
    return floatField;
}

bool ValidateStruct(ICFGField* node, const std::vector<CFGFieldType>& types) {
    CFGObject* structNode;
    if (node->type != CFGFieldType::STRUCT) {
        std::optional<std::string> name = node->name;
        ICFGField* field = CreateNewCFGField(node);
        if (field == nullptr)
            return false;
        structNode = new CFGObject{ name, CFGFieldType::STRUCT };
        structNode->AddItem(field);
        auto& parentItems = node->parent->GetItems();
        *std::find(parentItems.begin(), parentItems.end(), node) = structNode;
        delete node;
    }
    else
        structNode = static_cast<CFGObject*>(node);
    std::queue<CFGFieldType> typesQueue(std::deque<CFGFieldType>(types.begin(), types.end()));
    std::queue<CFGFieldType> receivedTypes;
    for (const auto& item : structNode->GetItems())
        receivedTypes.push(item->type);
    int memberIndex = 0;
    while (!typesQueue.empty()) {
        if (typesQueue.front() == CFGFieldType::STRUCT_MEMBER_REQUIRED) {
            typesQueue.pop();
            if (receivedTypes.empty())
                return false;
        }
        else {
            if (receivedTypes.empty()) {
                ICFGField* field = CreateNewCFGField(typesQueue.front());
                field->automaticallyCreated = true;
                structNode->AddItem(field);
            }
        }
        if (!receivedTypes.empty()) {
            if (!IsValidType(receivedTypes.front(), typesQueue.front()))
                return false;
            if ((receivedTypes.front() == CFGFieldType::INTEGER && typesQueue.front() == CFGFieldType::FLOAT)) {
                // cast integer into float
                structNode->GetItems()[memberIndex] = CastIntegerToFloat(structNode->GetItemByIndex<int>(memberIndex));
            }
        }
        typesQueue.pop();
        if (!receivedTypes.empty()) {
            receivedTypes.pop();
            ++memberIndex;
        }
    }
    if (!receivedTypes.empty())
        return false;
    return true;
}

bool ValidateCFGFieldType(ICFGField* node, std::vector<CFGFieldType> types) {
    if (types.empty())
        return false;
    if (node == nullptr)
        return false;
    if (types.at(0) == CFGFieldType::STRUCT) {
        bool success = ValidateStruct(node, { types.begin() + 1, types.end() });
        if (!success) {
            std::stringstream expected;
            for (auto it = types.cbegin() + 1; it != types.cend(); it++) {
                bool req = false;
                if (*it == CFGFieldType::STRUCT_MEMBER_REQUIRED) {
                    req = true;
                    ++it;
                }
                expected << fmt::format("{{{}}}{}", req ? "req" : "opt", magic_enum::enum_name(*it));
                if ((it + 1) != types.cend())
                    expected << ", ";
            }
            std::stringstream received;
            if (node->type != CFGFieldType::STRUCT)
                received << magic_enum::enum_name(node->type);
            else {
                const CFGObject* structNode = static_cast<const CFGObject*>(node);
                for (auto it = structNode->GetItems().cbegin(); it != structNode->GetItems().cend(); it++) {
                    received << magic_enum::enum_name((*it)->type);
                    if ((it + 1) != structNode->GetItems().cend())
                        received << ", ";
                }
            }
            spdlog::warn("Invalid struct field types! (Expected [{}] but received [{}])", expected.str(), received.str());
        }
        return success;
    }
    if (!IsValidType(node->type, types.at(0))) {
        spdlog::warn("Invalid field type for '{}'! (Expected {} but received {})",
            node->name.value_or("UNNAMED_FIELD"),
            magic_enum::enum_name(types.at(0)),
            magic_enum::enum_name(node->type));
        return false;
    }
    if ((node->type == CFGFieldType::INTEGER && types.at(0) == CFGFieldType::FLOAT)) {
        node = CastIntegerToFloat(dynamic_cast<const CFGField<int>*>(node));
    }
    if (types.at(0) == CFGFieldType::ARRAY) {
        if (!ValidateSubItems(node, types))
            return false;
    }
    return true;
}

bool ValidateCFGFields(CFGObject* node, const CFGStructuredFields& fields) {
    for (const auto& f : fields) {
        auto it = std::find_if(node->GetItems().begin(), node->GetItems().end(), [&](const ICFGField* field) { return f.name == field->name; });
        if (it == node->GetItems().end()) {
            if (f.required) {
                spdlog::warn("Missing mandatory field '{}'!", f.name);
                return false;
            }
            else {
                ICFGField* newField;
                if (f.isObject)
                    newField = new CFGObject();
                else
                    newField = CreateNewCFGField(f.types.at(0));
                newField->automaticallyCreated = true;
                newField->name = f.name;
                node->AddItem(newField);
                continue;
            }
        }
        if (f.isObject) {
            CFGObject* object = static_cast<CFGObject*>(*it);
            if (object == nullptr)
                return false;
            if(!ValidateCFGFields(object, f.objectParams))
                return false;
        }
        else {
            if(!ValidateCFGFieldType(*it, f.types))
                return false;
        }
    }
    return true;
}

bool ConvertCFGFieldTypesToPrimitives(ICFGField* node, const CFGCustomTypes& types) {
    std::vector<CFGFieldType> conformTypes;
    if (!node->typeAnnotation.empty()) {
        std::vector<CFGFieldType> conformTypes = ParseTypeAnnotation(node->typeAnnotation, true, types).value();
        return ValidateCFGFieldType(node, conformTypes);
    }
    if (node->type == CFGFieldType::STRUCT || node->type == CFGFieldType::ARRAY) {
        CFGObject* obj = static_cast<CFGObject*>(node);
        if (obj == nullptr)
            return false;
        for (ICFGField* f : obj->GetItems()) {
            if (!ConvertCFGFieldTypesToPrimitives(f, types))
                return false;
        }
    }
    return true;
}

bool ValidateCFG(CFGObject* root, const CFGFileTemplate& tmp) {
    if (!ConvertCFGFieldTypesToPrimitives(root, tmp.types))
        return false;
    return ValidateCFGFields(root, tmp.fields);
}

CFGSerializer::~CFGSerializer() {
    delete data_;
}

CFGObject* ParseIndentTree(CFGParseTreeNode<std::string>* strRoot) {
    return static_cast<CFGObject*>(ParseIndentTreeNodes(strRoot, true));
}

CFGObject* CFGSerializer::ParseCFG(std::stringstream& buffer, const CFGFileTemplate& fileFormat) {
    CFGParseTreeNode<std::string>* strRoot = CreateIndentationTree(buffer);
    if (strRoot == nullptr) {
        spdlog::warn("Failed creating a CFG indentation tree!");
        return nullptr;
    }
    CFGObject* root = ParseIndentTree(strRoot);
    delete strRoot;
    if (root == nullptr) {
        spdlog::warn("Invalid CFG format!");
        delete root;
        return nullptr;
    }
    if (!ValidateCFG(root, fileFormat)) {
        spdlog::warn("Field validation failed!");
        delete root;
        return nullptr;
    }
    return root;
}

bool CFGSerializer::Validate(const CFGFileTemplate& tmp) {
    return ValidateCFG(data_, tmp);
}

std::string CFGDecimal(const ICFGField* field) {
    std::string decimal = std::to_string(field->GetValue<float>());
    // remove trailing zeros from the decimal part
    while (decimal.length() > 3 && decimal.at(decimal.length() - 1) == '0') {
        if (decimal.at(decimal.length() - 2) == '.')
            break;
        decimal.pop_back();
    }
    return decimal;
}

void CFGFieldValueToString(const ICFGField* field, std::stringstream& ss, const CFGFormatting& format, int& indents) {
    const CFGObject* obj;
    if (field->automaticallyCreated)
        return;
    switch (field->type) {
        case CFGFieldType::NUMBER:
        case CFGFieldType::FLOAT:
            ss << CFGDecimal(field);
            break;
        case CFGFieldType::INTEGER:
            ss << std::to_string(field->GetValue<int>());
            break;
        case CFGFieldType::STRING:
            switch (format.stringLiteral) {
                case CFGStringLiteral::APOSTROPHES:
                    ss << '\'' + StrReplace(field->GetValue<std::string>(), "'", "\\a") + '\'';
                    break;
                case CFGStringLiteral::QUOTES:
                    ss << '"' + StrReplace(field->GetValue<std::string>(), "\"", "\\q") + '"';
                    break;
            }
            break;
        case CFGFieldType::STRUCT:
            obj = dynamic_cast<const CFGObject*>(field);
            for (int i = 0; i < obj->GetItems().size(); i++) {
                if (obj->GetItemByIndex(i)->automaticallyCreated)
                    continue;
                CFGFieldValueToString(obj->GetItemByIndex(i), ss, format, indents);
                if (i < obj->GetItems().size() - 1) {
                    ss << ' ';
                }
            }
            break;
        case CFGFieldType::ARRAY:
            obj = dynamic_cast<const CFGObject*>(field);
            indents += format.indents;
            for (int i = 0; i < obj->GetItems().size(); i++) {
                ss << std::string(indents, ' ');
                const ICFGField* e = obj->GetItemByIndex(i);
                if (e->name.has_value())
                    ss << e->name.value() << ": ";
                CFGFieldValueToString(e, ss, format, indents);
                if (i < obj->GetItems().size() - 1) {
                    ss << '\n';
                }
            }
            indents -= format.indents;
            break;
    }
}

void CFGFieldValueToString(const ICFGField* field, std::stringstream& ss, const CFGFormatting& format) {
    int indents = 0;
    CFGFieldValueToString(field, ss, format, indents);
}

void CFG::Dump(const CFGObject* root, std::stringstream& ss, const CFGFormatting& formatting) {
    for (const ICFGField* child : root->GetItems()) {
        if (child->type == CFGFieldType::RAW) {
            ss << child->GetValue<std::string>();
            continue;
        }
        if (child->name.has_value()) {
            ss << child->name.value() << " = ";
        }
        if (child->type == CFGFieldType::ARRAY)
            ss << '\n';
        CFGFieldValueToString(child, ss, formatting);
        ss << '\n';
    }
}

std::string CFG::Dump(const CFGObject* root, const CFGFormatting& formatting) {
    std::stringstream ss;
    Dump(root, ss, formatting);
    return ss.str();
}

bool CFGSerializer::StreamRead(std::ifstream& f) {
    std::stringstream buffer;
    buffer << f.rdbuf();
    data_ = ParseCFG(buffer, fileTemplate_);
    if (data_ == nullptr) {
        // create an empty object, don't wan't to null check every time
        data_ = new CFGObject();
        spdlog::warn("({}) CFG parsing failed!", path_);
        return false;
    }
    return true;
}

bool CFGSerializer::StreamWrite(std::ofstream& f) {
    f << CFG::Dump(data_);
    return true;
}

const CFGTypeMap CFG::CFG_TYPES_ = {
    CFG_TYPE_PAIR(std::string, CFGFieldType::STRING),
    CFG_TYPE_PAIR(int, CFGFieldType::INTEGER),
    CFG_TYPE_PAIR(bool, CFGFieldType::INTEGER),
    CFG_TYPE_PAIR(float, CFGFieldType::FLOAT),
    CFG_TYPE_PAIR(glm::vec2, CFG_VEC2(CFGFieldType::FLOAT)),
    CFG_TYPE_PAIR(glm::ivec2, CFG_VEC2(CFGFieldType::INTEGER))
};
const CFGTypeMap& CFG::GetCFGTypeMap() {
    return CFG_TYPES_;
}

CFGSerializerList& Serializer::GetCFGSerializerList() {
    static CFGSerializerList cfgSerializers;
    return cfgSerializers;
}