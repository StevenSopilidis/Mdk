#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <sched.h>
#include <string>
#include <vector>

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
    std::uint64_t                                        id{};
    pid_t                                                pid{-1};
    std::filesystem::path                                rootfs;
    std::chrono::system_clock::time_point                created_at{};
    std::optional<std::chrono::system_clock::time_point> exited_at;
    std::string                                          command;
    std::vector<std::string>                             argv;
    std::vector<std::string>                             env;
    ContainerProcessState                                process_state{ContainerProcessState::Created};
    std::optional<int>                                   wait_status;
};

} // namespace mdk::core
