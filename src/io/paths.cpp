#include <latren/io/paths.h>

#include <algorithm>
#include <spdlog/spdlog.h>

std::unordered_map<std::string, ResourcePath> CreateDefaultPathVars() {
    std::unordered_map<std::string, ResourcePath> vars = Paths::DEFAULT_PATH_VARS;
    vars["cwd"] = std::fs::current_path().generic_string();
    return vars;
}

std::unordered_map<std::string, ResourcePath>& GetGlobalPathVars() {
    static std::unordered_map<std::string, ResourcePath> PATH_VARS = CreateDefaultPathVars();
    return PATH_VARS;
}

std::vector<std::pair<std::string, ResourcePath>> Paths::ListGlobalPathVars() {
    std::vector<std::pair<std::string, ResourcePath>> pairs;
    pairs.assign(GetGlobalPathVars().begin(), GetGlobalPathVars().end());
    std::sort(pairs.begin(), pairs.end(), [](const auto& l, const auto& r) {
        return l.first < r.first;
    });
    return pairs;
}

const ResourcePath EMPTY_PATH = "";
const ResourcePath& Paths::GetGlobalPathVar(const std::string& var) {
    if (GetGlobalPathVars().count(var) == 0) {
        spdlog::warn("Path variable '{}' not found!", var);
        return EMPTY_PATH;
    }
    return GetGlobalPathVars().at(var);
}

void Paths::SetGlobalPathVar(const std::string& var, const ResourcePath& val) {
    GetGlobalPathVars()[var] = val;
}