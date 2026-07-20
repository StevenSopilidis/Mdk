#pragma once

#include <concepts>
#include <csignal>
namespace mdk::core
{

template <int Signal, typename Lambda>
    requires std::invocable<Lambda, int> && std::same_as<std::invoke_result_t<Lambda, int>, void>
void InstallSignalHandler(Lambda handler)
{
    std::signal(Signal, handler);
}

} // namespace mdk::core