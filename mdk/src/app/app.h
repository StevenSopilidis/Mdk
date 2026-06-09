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

  private:
    void handle_help();
    void handle_run_raw();

    core::ContainerManager container_manager_;
    Logger&                logger_;
    ArgParser              arg_parser_;
};
} // namespace mdk::app