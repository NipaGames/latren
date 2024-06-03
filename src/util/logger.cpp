#include <latren/util/logging.h>

void Logging::InitSPDLog(const std::string& pattern, LogLevel logLevel) {
    spdlog::set_pattern(pattern);
    if (logLevel == LogLevel::DEBUG)
        spdlog::set_level(spdlog::level::debug);
}
