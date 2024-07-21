#include <latren/io/paths.h>

std::unordered_map<std::string, ResourcePath>& GetPathVars() {
    static std::unordered_map<std::string, ResourcePath> PATH_VARS = Paths::DEFAULT_PATH_VARS;
    return PATH_VARS; 
}

const ResourcePath& Paths::GetGlobalPathVar(const std::string& val) {
    if (val == "cwd")
        return std::fs::current_path().generic_string();
    return GetPathVars().at(val);
}

void Paths::SetGlobalPathVar(const std::string& key, const ResourcePath& val) {
    GetPathVars()[key] = val;
}