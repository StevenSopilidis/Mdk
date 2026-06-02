#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <sched.h>
#include <string_view>

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

class Container
{
  public:
    ~Container();

    static Container* Create(std::filesystem::path rootfs, std::string& command);

    void mark_exited();

    [[nodiscard]] pid_t         get_pid() const;
    [[nodiscard]] std::uint64_t get_id() const;

  private:
    Container() = default;

    static int child_func(void* arg);

    ContainerState state_;
    char*          stack_;
};

} // namespace mdk::core