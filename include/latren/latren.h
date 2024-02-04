#pragma once

#include <string>

#ifdef _WIN32
#ifdef LATREN_EXPORT_DLL
#define LATREN_API __declspec(dllexport)
#else
#define LATREN_API __declspec(dllimport)
#endif
#else
#define LATREN_API
#endif

namespace Latren {
    struct VersionInfo {
        int major;
        int minor;
    };

    enum class BuildType {
        DEBUG,
        RELEASE
    };

    LATREN_API constexpr VersionInfo GetBuildVersion();
    LATREN_API constexpr BuildType GetBuildType();
    LATREN_API std::string FormatVersion(const VersionInfo& = GetBuildVersion(), const BuildType& = GetBuildType());
};