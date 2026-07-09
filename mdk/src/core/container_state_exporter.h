#pragma once

#include "container_state.h"

#include <ostream>

namespace mdk::core
{
class ContainerState;

template <typename E>
concept ContainerStateExporter =
    requires(E exporter, const ContainerState& state, std::ostream& out) {
        { exporter.Export(state, out) } -> std::same_as<void>;
    };

template <ContainerStateExporter Exporter>
void ExportState(const ContainerState& state, std::ostream& out)
{
    Exporter{}.Export(state, out);
}

} // namespace mdk::core