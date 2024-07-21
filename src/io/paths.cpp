#include <latren/io/paths.h>

std::unordered_map<std::string, ResourcePath> CreateDefaultPathVars() {
    std::unordered_map<std::string, ResourcePath> vars = Paths::DEFAULT_PATH_VARS;
    vars["cwd"] = std::fs::current_path().generic_string();
    return vars;
}

std::unordered_map<std::string, ResourcePath>& GetPathVars() {
    static std::unordered_map<std::string, ResourcePath> PATH_VARS = CreateDefaultPathVars();
    return PATH_VARS; 
}

const ResourcePath& Paths::GetGlobalPathVar(const std::string& val) {
    return GetPathVars().at(val);
}

void Paths::SetGlobalPathVar(const std::string& key, const ResourcePath& val) {
    GetPathVars()[key] = val;
}