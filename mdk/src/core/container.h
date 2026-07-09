#pragma once

#include "core/container_state_exporter.h"

#include <cstdint>
#include <filesystem>
#include <sched.h>

namespace mdk::core
{

class Container
{
  public:
    ~Container();

    static Container* Create(std::filesystem::path rootfs, const std::string& command);

    template <ContainerStateExporter Exporter> void ExportState(std::ostream& out)
    {
        Exporter{}.Export(state_, out);
    }

    void mark_exited();

    [[nodiscard]] pid_t         get_pid() const;
    [[nodiscard]] std::uint64_t get_id() const;

  private:
    Container() = default;

    static int ChildFunc(void* arg);

    ContainerState state_;
    char*          stack_;
};

} // namespace mdk::core