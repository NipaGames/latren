#pragma once

#include <string>
#include <latren/latren.h>

#include "fs.h"

class  ResourcePath {
private:
    std::string path_;
public:
    ResourcePath() = default;
    ResourcePath(const char* p) : path_(p) { }
    ResourcePath(const std::string& p) : path_(p) { }
    ResourcePath(const std::fs::path& p) : path_(p.generic_string()) { }
    ResourcePath(const ResourcePath& p0, const ResourcePath& p1) : path_(p0.GetUnparsedPathStr() + "/" + p1.GetUnparsedPathStr()) { }
    const std::string& GetUnparsedPathStr() const { return path_; }
    std::fs::path GetParsedPath() const;
    std::string GetParsedPathStr() const { return GetParsedPath().generic_string(); }
    bool IsEmpty() const;
};

 ResourcePath operator ""_res(const char*, std::size_t);
// parsed path
 std::string operator ""_resp(const char*, std::size_t);