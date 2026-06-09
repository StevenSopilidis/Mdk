#pragma once

#include "core/container.h"

#include <cstdint>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace mdk::core
{
class ContainerManager
{
  public:
    ContainerManager() = default;
    ~ContainerManager();

    bool create_container(std::string_view rootfs, const std::string& command);
    void launch_reap_thread();
    void stop();

  private:
    std::shared_mutex                     mtx_;
    std::atomic<bool>                     running_{true};
    std::thread                           reaper_thread_;
    std::unordered_map<pid_t, Container*> containers_;
};
} // namespace mdk::core