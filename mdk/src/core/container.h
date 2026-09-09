#pragma once

#include "core/container_state_exporter.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace mdk::core
{

class Container
{
  public:
    ~Container() = default;

    static std::unique_ptr<Container> Create(std::filesystem::path      rootfs,
                                             std::vector<std::string>   argv,
                                             std::vector<std::string>   env);

    template <ContainerStateExporter Exporter> void ExportState(std::ostream& out)
    {
        Exporter{}.Export(state_, out);
    }

    void mark_exited(int wait_status);

    [[nodiscard]] pid_t                 get_pid() const;
    [[nodiscard]] std::uint64_t         get_id() const;
    [[nodiscard]] ContainerProcessState process_state() const;

  private:
    Container() = default;

    static int ChildFunc(void* arg);

    void ReleaseStack();

    ContainerState           state_;
    std::unique_ptr<char[]>  stack_;
};

} // namespace mdk::core
