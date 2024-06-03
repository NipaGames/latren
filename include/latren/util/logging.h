#pragma once

#include <latren/latren.h>
#include <spdlog/spdlog.h>
#include <string>

namespace Logging {
    inline constexpr std::string_view DEFAULE_SPDLOG_PATTERN = "[%T] %^%-10l%$ %v";

    enum class LogLevel {
        DEFAULT,
        DEBUG
    };

    LATREN_API void InitSPDLog(const std::string& = std::string(DEFAULE_SPDLOG_PATTERN), LogLevel = LogLevel::DEFAULT);
};