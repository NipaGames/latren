#pragma once

#include <string>

namespace Latren {
    struct VersionInfo {
        int major;
        int minor;
    };

    enum class BuildType {
        DEBUG,
        RELEASE
    };

     VersionInfo GetBuildVersion();
     BuildType GetBuildType();
     std::string FormatVersion(const VersionInfo& = GetBuildVersion(), const BuildType& = GetBuildType());
};