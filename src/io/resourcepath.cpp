#include <latren/io/resourcepath.h>
#include <latren/io/paths.h>

#include <iostream>

bool ResourcePath::IsEmpty() const {
    return GetUnparsedPathStr().empty();
}

std::fs::path ResourcePath::GetParsedPath() const {
    std::string p = path_;
    std::size_t begin, end;
    while ((begin = p.find("${")) != std::string::npos) {
        if (begin + 2 == p.size())
            break;
        end = p.find("}", begin + 2);
        if (end == std::string::npos)
            break;
        
        std::string pathVar = p.substr(begin + 2, end - begin - 2);
        // preferring this over the recursive GetParsedPath for pathVar
        p.replace(begin, end - begin + 1, Paths::GetGlobalPathVar(pathVar).GetUnparsedPathStr());
    }
    return std::fs::path(p);
}

ResourcePath operator ""_res(const char* str, std::size_t s) {
    return ResourcePath(std::string(str, s));
}

std::string operator ""_resp(const char* str, std::size_t s) {
    return ResourcePath(std::string(str, s)).GetParsedPathStr();
}