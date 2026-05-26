#include "logger.h"

#include <chrono>
#include <cstdio>
#include <string_view>

namespace mdk::utils
{

std::string Logger::IsoNow()
{
    auto        now        = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm utc_tm = *std::gmtime(&now_time_t);

    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

Logger& Logger::GetInstance()
{
    static Logger logger;

    return logger;
}
} // namespace mdk::utils