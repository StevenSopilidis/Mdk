#pragma once

#include "core/container_manager.h"
#include "server/server.h"
#include "utils/arg_parser.h"
#include "utils/logger.h"

#include <atomic>
#include <string>

namespace mdk::app
{

using namespace mdk::utils;

class App
{
  public:
    void        Run();
    std::string ProcessCommand(ArgParser& argParser);
    void        Stop();

    static App& GetInstance();

  private:
    App() = default;

    std::string HandleHelp(ArgParser& argParser);
    std::string HandleList(ArgParser& argParser);
    std::string HandleRunRaw(ArgParser& argParser);

    static void DefaultSignalHandler(int);

    std::atomic<bool>      running_{false};
    core::ContainerManager container_manager_;
    server::Server         server_;
};

} // namespace mdk::app
