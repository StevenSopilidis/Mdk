#include "container.h"

#include "utils/logger.h"

#include <csignal>
#include <sys/wait.h>

namespace mdk::core
{

#define STACK_SIZE (1024 * 1024)

Container::~Container() { delete[] stack_; }

Container* Container::Create(std::filesystem::path rootfs, const std::string& command)
{
    static uint64_t id_counter = 0;

    auto* container = new Container();

    container->stack_ = new char[STACK_SIZE];

    container->state_.id            = id_counter++;
    container->state_.pid           = -1;
    container->state_.rootfs        = rootfs;
    container->state_.command       = command;
    container->state_.created_at    = std::chrono::system_clock::now();
    container->state_.exited_at     = std::nullopt;
    container->state_.process_state = ContainerProcessState::Created;

    int flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWIPC | SIGCHLD;

    void* stack_top = container->stack_ + STACK_SIZE;
    stack_top       = (void*)((uintptr_t)stack_top & ~0xF); // make sure its 16-byte aligned

    pid_t pid = clone(child_func, stack_top, flags, &container->state_);

    if (pid == -1)
    {
        LOG_ERROR("clone failed");
        container->state_.process_state = ContainerProcessState::Dead;
        delete[] container->stack_;
        delete container;
        return nullptr;
    }

    container->state_.pid           = pid;
    container->state_.process_state = ContainerProcessState::Running;

    return container;
}

int Container::child_func(void* arg)
{
    auto* state = static_cast<ContainerState*>(arg);

    if (chdir(state->rootfs.c_str()) != 0)
    {
        LOG_ERROR("chdir failed");
        return 1;
    }

    const char* argv[] = {state->command.c_str(), nullptr};

    execve(argv[0], const_cast<char* const*>(argv), nullptr);

    LOG_ERROR("execve failed");
    return 1;
}

pid_t Container::get_pid() const { return state_.pid; }

std::uint64_t Container::get_id() const { return state_.id; }

void Container::mark_exited()
{
    state_.process_state = ContainerProcessState::Exited;
    state_.exited_at     = std::chrono::system_clock::now();
}

} // namespace mdk::core