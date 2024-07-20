#include <latren/io/resourcepath.h>
#include <latren/io/paths.h>

#include <iostream>

const std::string& ResourcePath::GetPath() const {
    return path_;
}

bool ResourcePath::IsEmpty() const {
    return GetPath().empty();
}

std::fs::path ResourcePath::ParsePath(const std::fs::path& dir) const {
    std::string p = path_;
    std::fs::path d = dir;
    bool latrenDir = false;
    if (p.rfind("latren:", 0) == 0) {
        p.erase(0, 7);
        latrenDir = true;
    }
    if (latrenDir) {
        d = Paths::LATREN_CORE_RESOURCES_DIR / std::fs::relative(dir, Paths::RESOURCES_DIR);
    }
    return d / p;
}

ResourcePath operator ""_res(const char* str, std::size_t s) {
    return ResourcePath(std::string(str, s));
}