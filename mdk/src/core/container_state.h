#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <sched.h>
namespace mdk::core
{

enum class ContainerProcessState : std::uint8_t
{
    Created,
    Running,
    Exited,
    Paused,
    Dead,
};

struct ContainerState
{
    std::uint64_t                                        id;
    pid_t                                                pid;
    std::filesystem::path                                rootfs;
    std::chrono::system_clock::time_point                created_at;
    std::optional<std::chrono::system_clock::time_point> exited_at;
    std::string                                          command;
    ContainerProcessState                                process_state;
};

} // namespace mdk::core