#include <latren/io/resourcemanager.h>
#include <fstream>
#include <sstream>

using namespace Resources;

std::optional<std::string> TextFileManager::LoadResource(const ResourcePath& path) {
    std::ifstream fs = std::ifstream(path.GetParsedPath());
    std::stringstream sstream;
    sstream << fs.rdbuf();
    return sstream.str();
}

std::optional<BinaryFile> BinaryFileManager::LoadResource(const ResourcePath& path) {
    return { };
}

std::optional<nlohmann::json> JSONFileManager::LoadResource(const ResourcePath& path) {
    std::ifstream fs = std::ifstream(path.GetParsedPath());
    return nlohmann::json::parse(fs);
}

std::optional<CFG::CFGObject*> CFGFileManager::LoadResource(const ResourcePath& path) {
    return nullptr;
}
CFGFileManager::~CFGFileManager() {
    for (const auto& [k, v] : items_) {
        delete v;
    }
}
