#include "container_manager.h"

#include "utils/logger.h"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <mutex>
#include <sys/wait.h>
#include <unistd.h>

namespace mdk::core
{

ContainerManager::~ContainerManager()
{
    {
        std::unique_lock lock(mtx_);
        for (auto& [id, container] : containers_)
        {
            if (container && container->process_state() == ContainerProcessState::Running)
            {
                kill(container->get_pid(), SIGKILL);
            }
        }
    }

    Stop();

    if (reaper_thread_.joinable())
    {
        reaper_thread_.join();
    }
}

std::optional<CreateContainerResult>
ContainerManager::CreateContainer(std::string_view rootfs, const std::vector<std::string>& argv)
{
    auto container = Container::Create(std::filesystem::path{rootfs}, argv, {});

    if (container == nullptr)
    {
        LOG_ERROR("Container creation failed");
        return std::nullopt;
    }

    CreateContainerResult result{container->get_id(), container->get_pid()};

    {
        std::unique_lock lock(mtx_);
        pid_to_id_[container->get_pid()] = container->get_id();
        containers_[container->get_id()] = std::move(container);
    }

    LOG_INFO("Started container with pid: {} and id: {}", result.pid, result.id);

    return result;
}

void ContainerManager::Stop()
{
    running_.store(false, std::memory_order_release);
    kill(getpid(), SIGCHLD);
}

void ContainerManager::LaunchReapThread()
{
    if (reaper_thread_.joinable())
    {
        return;
    }

    reaper_thread_ = std::thread(
        [this]()
        {
            int status = 0;

            while (running_.load(std::memory_order_acquire))
            {
                pid_t pid = waitpid(-1, &status, 0);

                if (pid > 0)
                {
                    std::unique_lock lock(mtx_);

                    auto pid_it = pid_to_id_.find(pid);
                    if (pid_it == pid_to_id_.end())
                    {
                        continue;
                    }

                    auto id = pid_it->second;
                    pid_to_id_.erase(pid_it);

                    auto it = containers_.find(id);
                    if (it != containers_.end())
                    {
                        it->second->mark_exited(status);
                    }
                }
                else if (pid == -1 && errno == EINTR)
                {
                    continue;
                }
                else if (pid == -1 && errno == ECHILD)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        });
}

const std::unordered_map<std::uint64_t, std::unique_ptr<Container>>&
ContainerManager::GetContainers() const
{
    return containers_;
}

} // namespace mdk::core
