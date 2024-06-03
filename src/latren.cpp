#include <latren/latren.h>

#include <sstream>
#include <algorithm>
#include <magic_enum/magic_enum.hpp>

Latren::VersionInfo Latren::GetBuildVersion() {
    #if defined(LATREN_VERSION_MAJ) && defined(LATREN_VERSION_MIN)
    return { LATREN_VERSION_MAJ, LATREN_VERSION_MIN };
    #endif
    return { -1, -1 };
}

Latren::BuildType Latren::GetBuildType() {
    #ifdef DEBUG_BUILD
    return Latren::BuildType::DEBUG;
    #endif
    return Latren::BuildType::RELEASE;
}

std::string Latren::FormatVersion(const Latren::VersionInfo& vInfo, const Latren::BuildType& buildType) {
    std::string buildTypeStr = (std::string) magic_enum::enum_name(buildType);
    std::transform(buildTypeStr.begin(), buildTypeStr.end(), buildTypeStr.begin(), ::tolower);
    buildTypeStr[0] = std::toupper(buildTypeStr[0]);
    // ran into linker errors using fmt
    std::stringstream ss;
    ss << 'v' << vInfo.major << '.' << vInfo.minor << " [" << buildTypeStr << ']';
    return ss.str();
}