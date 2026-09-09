#include "container.h"

#include "utils/logger.h"

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <csignal>
#include <cstring>
#include <memory>
#include <sched.h>
#include <string>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <vector>

namespace mdk::core
{

namespace
{
constexpr std::size_t kStackSize = 1024 * 1024;

std::vector<char*> ToCStringArray(std::vector<std::string>& strings)
{
    std::vector<char*> out;
    out.reserve(strings.size() + 1);
    for (auto& s : strings)
    {
        out.push_back(s.data());
    }
    out.push_back(nullptr);
    return out;
}

std::string JoinCommand(const std::vector<std::string>& argv)
{
    std::string out;
    for (std::size_t i = 0; i < argv.size(); ++i)
    {
        if (i > 0)
        {
            out += ' ';
        }
        out += argv[i];
    }
    return out;
}
} // namespace

std::unique_ptr<Container> Container::Create(std::filesystem::path    rootfs,
                                             std::vector<std::string> argv,
                                             std::vector<std::string> env)
{
    static std::uint64_t id_counter = 0;

    if (argv.empty())
    {
        LOG_ERROR("Container create requires a command");
        return nullptr;
    }

    if (env.empty())
    {
        env = {"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", "HOME=/",
               "TERM=xterm"};
    }

    auto container = std::unique_ptr<Container>(new Container());

    container->stack_               = std::make_unique<char[]>(kStackSize);
    container->state_.id            = id_counter++;
    container->state_.pid           = -1;
    container->state_.rootfs        = std::move(rootfs);
    container->state_.argv          = std::move(argv);
    container->state_.env           = std::move(env);
    container->state_.command       = JoinCommand(container->state_.argv);
    container->state_.created_at    = std::chrono::system_clock::now();
    container->state_.exited_at     = std::nullopt;
    container->state_.process_state = ContainerProcessState::Created;

    int flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWIPC | SIGCHLD;

    void* stack_top = container->stack_.get() + kStackSize;
    stack_top       = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(stack_top) & ~0xF);

    pid_t pid = clone(ChildFunc, stack_top, flags, &container->state_);

    if (pid == -1)
    {
        LOG_ERROR("clone failed with errno: {}", errno);
        container->state_.process_state = ContainerProcessState::Dead;
        return nullptr;
    }

    container->state_.pid           = pid;
    container->state_.process_state = ContainerProcessState::Running;

    return container;
}

int Container::ChildFunc(void* arg)
{
    auto* state = static_cast<ContainerState*>(arg);

    // --------------------------------------------------
    // 1. Isolate mount namespace
    // --------------------------------------------------
    if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) != 0)
    {
        LOG_ERROR("MS_PRIVATE failed: {}", strerror(errno));
        return 1;
    }

    // --------------------------------------------------
    // 2. Bind + pivot root
    // --------------------------------------------------
    if (mount(state->rootfs.c_str(), state->rootfs.c_str(), nullptr, MS_BIND | MS_REC, nullptr) !=
        0)
    {
        LOG_ERROR("bind mount failed: {}", strerror(errno));
        return 1;
    }

    auto old_root = state->rootfs / ".oldroot";

    if (mkdir(old_root.c_str(), 0755) != 0 && errno != EEXIST)
    {
        LOG_ERROR("mkdir oldroot failed: {}", strerror(errno));
        return 1;
    }

    if (syscall(SYS_pivot_root, state->rootfs.c_str(), old_root.c_str()) != 0)
    {
        LOG_ERROR("pivot_root failed: {}", strerror(errno));
        return 1;
    }

    chdir("/");

    umount2("/.oldroot", MNT_DETACH);
    rmdir("/.oldroot");

    // ==================================================
    // 3. CORE MOUNT POINTS
    // ==================================================
    mkdir("/proc", 0555);
    mkdir("/sys", 0555);
    mkdir("/dev", 0755);

    if (mount("proc", "/proc", "proc", 0, nullptr) != 0)
    {
        LOG_ERROR("proc mount failed: {}", strerror(errno));
        return 1;
    }

    if (mount("sysfs", "/sys", "sysfs", 0, nullptr) != 0)
    {
        LOG_ERROR("sysfs mount failed: {}", strerror(errno));
        return 1;
    }

    if (mount("tmpfs", "/dev", "tmpfs", MS_NOSUID | MS_STRICTATIME, "mode=755") != 0)
    {
        LOG_ERROR("dev tmpfs failed: {}", strerror(errno));
        return 1;
    }

    mkdir("/dev/pts", 0755);

    if (mount("devpts", "/dev/pts", "devpts", 0, "newinstance,ptmxmode=666") != 0)
    {
        LOG_ERROR("devpts failed: {}", strerror(errno));
        return 1;
    }

    // ==================================================
    // 7. DEVICE NODES
    // ==================================================
    auto mk = [&](const char* path, mode_t mode, int maj, int min)
    {
        if (mknod(path, mode, makedev(maj, min)) != 0 && errno != EEXIST)
        {
            LOG_ERROR("mknod {} failed: {}", path, strerror(errno));
        }
    };

    mk("/dev/null", S_IFCHR | 0666, 1, 3);
    mk("/dev/zero", S_IFCHR | 0666, 1, 5);
    mk("/dev/full", S_IFCHR | 0666, 1, 7);
    mk("/dev/random", S_IFCHR | 0666, 1, 8);
    mk("/dev/urandom", S_IFCHR | 0666, 1, 9);
    mk("/dev/tty", S_IFCHR | 0666, 5, 0);

    // ==================================================
    // 8. EXEC
    // ==================================================
    auto argv = ToCStringArray(state->argv);
    auto env  = ToCStringArray(state->env);

    execve(argv[0], argv.data(), env.data());

    LOG_ERROR("execve failed: {}", strerror(errno));
    return 1;
}

pid_t Container::get_pid() const { return state_.pid; }

std::uint64_t Container::get_id() const { return state_.id; }

ContainerProcessState Container::process_state() const { return state_.process_state; }

void Container::ReleaseStack() { stack_.reset(); }

void Container::mark_exited(int wait_status)
{
    state_.process_state = ContainerProcessState::Exited;
    state_.exited_at     = std::chrono::system_clock::now();
    state_.wait_status   = wait_status;
    ReleaseStack();
}

} // namespace mdk::core
