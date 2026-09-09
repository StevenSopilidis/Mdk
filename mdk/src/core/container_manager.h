#pragma once

#include "core/container.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace mdk::core
{

struct CreateContainerResult
{
    std::uint64_t id{0};
    pid_t         pid{-1};
};

class ContainerManager
{
  public:
    ContainerManager() = default;
    ~ContainerManager();

    std::optional<CreateContainerResult> CreateContainer(std::string_view                rootfs,
                                                         const std::vector<std::string>& argv);
    void                                 LaunchReapThread();
    void                                 Stop();

  private:
    std::shared_mutex                                          mtx_;
    std::atomic<bool>                                          running_{true};
    std::thread                                                reaper_thread_;
    std::unordered_map<std::uint64_t, std::unique_ptr<Container>> containers_;
    std::unordered_map<pid_t, std::uint64_t>                   pid_to_id_;
};

} // namespace mdk::core
