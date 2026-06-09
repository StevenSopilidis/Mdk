#include "container_manager.h"

#include <cerrno>
#include <mutex>
#include <sys/wait.h>

namespace mdk::core
{

ContainerManager::~ContainerManager()
{
    running_.store(false, std::memory_order_release);

    for (auto& [pid, _] : containers_)
    {
        kill(pid, SIGKILL);
    }

    // wake reaper if it's blocked in waitpid
    kill(getpid(), SIGCHLD);

    if (reaper_thread_.joinable())
    {
        reaper_thread_.join();
    }
}

bool ContainerManager::create_container(std::string_view rootfs, const std::string& command)
{
    auto* container = Container::Create(rootfs, command);

    if (container == nullptr)
    {
        return false;
    }

    {
        std::unique_lock lock(mtx_);
        containers_[container->get_pid()] = container;
    }

    return true;
}

void ContainerManager::stop() { running_.store(false, std::memory_order_release); }

void ContainerManager::launch_reap_thread()
{
    reaper_thread_ = std::thread(
        [this]()
        {
            int status;

            while (running_)
            {
                pid_t pid = waitpid(-1, &status, 0);

                if (pid > 0)
                {
                    std::unique_lock lock(mtx_);

                    auto it = containers_.find(pid);
                    if (it != containers_.end())
                        it->second->mark_exited();
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

} // namespace mdk::core