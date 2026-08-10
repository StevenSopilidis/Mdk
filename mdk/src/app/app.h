#pragma once

#include "core/container_manager.h"
#include "utils/arg_parser.h"
#include "utils/logger.h"

namespace mdk::app
{

using namespace mdk::utils;

class App
{
  public:
    void Run();
    void ProcessCommand(ArgParser& argParser);
    void Stop();

    static App& GetInstance();

  private:
    App() = default;
    void HandleHelp(ArgParser& argParser);
    void HandleRunRaw(ArgParser& argParser);

    static void DefaultSignalHandler(int);

    std::atomic<bool>      running_;
    std::jthread           main_loop_thread_;
    core::ContainerManager container_manager_;
};
} // namespace mdk::app