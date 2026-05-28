#pragma once

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <format>
#include <mutex>
#include <ostream>
#include <unistd.h>

namespace mdk::utils
{
enum class LogLevel : uint8_t
{
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

template <LogLevel level> constexpr std::string_view get_log_level_color()
{
    switch (level)
    {
    case LogLevel::Trace:
        return "\x1b[90m";
    case LogLevel::Debug:
        return "\x1b[36m";
    case LogLevel::Info:
        return "\x1b[32m";
    case LogLevel::Warn:
        return "\x1b[33m";
    case LogLevel::Error:
        return "\x1b[31m";
    case LogLevel::Fatal:
        return "\x1b[1;31m";
    }
    return "";
}

template <LogLevel level> constexpr std::string_view get_log_level_tag()
{
    switch (level)
    {
    case LogLevel::Trace:
        return "TRACE";
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Fatal:
        return "FATAL";
    }
    return "?";
}

class Logger
{
  public:
    Logger(const Logger&)            = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&)      = delete;

    static Logger& getInstance();

    template <LogLevel lvl, typename... Args>
    void log(const char* file, int line, std::format_string<Args...> fmt, Args&&... args)
    {
        write<lvl>(file, line, std::format(fmt, std::forward<Args>(args)...));
    }

  private:
    template <LogLevel lvl> void write(const char* file, int line, std::string_view msg)
    {
        std::lock_guard lock(mtx_);

        fprintf(stderr, "%s %s[%s]%s pid=%d %s:%d %.*s\n", isoNow().c_str(),
                get_log_level_color<lvl>().data(), get_log_level_tag<lvl>().data(), "\x1b[0m",
                ::getpid(), std::filesystem::path(file).filename().string().c_str(), line,
                (int)msg.size(), msg.data());

        std::fflush(stderr);
    }

    std::string isoNow();

    Logger() = default;

    std::mutex mtx_;
};
} // namespace mdk::utils

#define LOG_TRACE(fmt, ...)                                                                        \
    ::mdk::utils::Logger::getInstance().log<::mdk::utils::LogLevel::Trace>(__FILE__, __LINE__,     \
                                                                           fmt, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...)                                                                        \
    ::mdk::utils::Logger::getInstance().log<::mdk::utils::LogLevel::Debug>(__FILE__, __LINE__,     \
                                                                           fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...)                                                                         \
    ::mdk::utils::Logger::getInstance().log<::mdk::utils::LogLevel::Info>(__FILE__, __LINE__, fmt, \
                                                                          ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)                                                                         \
    ::mdk::utils::Logger::getInstance().log<::mdk::utils::LogLevel::Warn>(__FILE__, __LINE__, fmt, \
                                                                          ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...)                                                                        \
    ::mdk::utils::Logger::getInstance().log<::mdk::utils::LogLevel::Error>(__FILE__, __LINE__,     \
                                                                           fmt, ##__VA_ARGS__)

#define LOG_FATAL(fmt, ...)                                                                        \
    ::mdk::utils::Logger::getInstance().log<::mdk::utils::LogLevel::Fatal>(__FILE__, __LINE__,     \
                                                                           fmt, ##__VA_ARGS__);    \
    exit(-1);