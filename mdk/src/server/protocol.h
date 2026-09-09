#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace mdk::server
{

inline constexpr std::string_view kServerPath    = "/tmp/mdk_daemon.sock";
inline constexpr std::uint32_t    kMaxFrameBytes = 1024 * 1024;

bool                   WriteFrame(int fd, std::string_view payload);
std::optional<std::string> ReadFrame(int fd);

} // namespace mdk::server
