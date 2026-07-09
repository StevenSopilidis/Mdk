#pragma once

#include "core/container_state.h"
#include "third-party/nlohmann/json.hpp"

namespace
{
inline std::int64_t to_unix(const std::chrono::system_clock::time_point& tp)
{
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}
} // namespace

namespace mdk::core
{
struct JsonContainerStateExporter
{
    void Export(const ContainerState& state, std::ostream& out)
    {
        nlohmann::json j = {{"id", state.id},
                            {"pid", state.pid},
                            {"rootfs", state.rootfs.string()},
                            {"created_at", to_unix(state.created_at)},
                            {"command", state.command},
                            {"process_state", state.process_state}};

        if (state.exited_at)
            j["exited_at"] = to_unix(*state.exited_at);
        else
            j["exited_at"] = nullptr;

        out << j.dump(2);
    }
};
} // namespace mdk::core