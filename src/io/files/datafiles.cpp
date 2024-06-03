#include <latren/io/resourcemanager.h>
#include <fstream>
#include <sstream>

using namespace Resources;

std::optional<std::string> TextFileManager::LoadResource(const std::fs::path& path) {
    std::ifstream fs = std::ifstream(path);
    std::stringstream sstream;
    sstream << fs.rdbuf();
    return sstream.str();
}

std::optional<BinaryFile> BinaryFileManager::LoadResource(const std::fs::path& path) {
    return { };
}

std::optional<nlohmann::json> JSONFileManager::LoadResource(const std::fs::path& path) {
    std::ifstream fs = std::ifstream(path);
    return nlohmann::json::parse(fs);
}

std::optional<CFG::CFGObject*> CFGFileManager::LoadResource(const std::fs::path& path) {
    return nullptr;
}
CFGFileManager::~CFGFileManager() {
    for (const auto& [k, v] : items_) {
        delete v;
    }
}
