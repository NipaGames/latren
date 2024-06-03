#pragma once

#include <string>
#include <latren/latren.h>

#include "fs.h"

class LATREN_API ResourcePath {
private:
    std::string path_;
public:
    ResourcePath() = default;
    ResourcePath(const std::string& p) : path_(p) { }
    const std::string& GetPath() const;
    operator std::string() const { return GetPath(); }
    bool IsEmpty() const;
    std::fs::path ParsePath(const std::fs::path&) const;
};

LATREN_API ResourcePath operator ""_res(const char*, size_t);