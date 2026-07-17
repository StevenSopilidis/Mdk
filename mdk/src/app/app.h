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
    App();
    void Run();
    void ProcessCommand(int argc, char** argv);
    void Stop();

  private:
    void HandleHelp(ArgParser& argParser);
    void HandleRunRaw(ArgParser& argParser);

    std::atomic<bool>      running_;
    std::thread            main_loop_thread_;
    core::ContainerManager container_manager_;
    Logger&                logger_;
};
} // namespace mdk::app