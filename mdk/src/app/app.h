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
    App(int argc, char** argv);
    void Run();
    void Stop();

  private:
    void HandleHelp();
    void HandleRunRaw();

    std::atomic<bool>      running_;
    std::thread            main_loop_thread_;
    core::ContainerManager container_manager_;
    Logger&                logger_;
    ArgParser              arg_parser_;
};
} // namespace mdk::app