#pragma once

#include <string>
#include <latren/latren.h>

#include "fs.h"

class ResourcePath {
private:
    std::string path_;
public:
    ResourcePath() = default;
    ResourcePath(const std::string& p) : path_(p) { }
    LATREN_API const std::string& GetPath() const;
    operator std::string() const { return GetPath(); }
    LATREN_API bool IsEmpty() const;
    LATREN_API std::fs::path ParsePath(const std::fs::path&) const;
};

LATREN_API ResourcePath operator ""_res(const char*, size_t);