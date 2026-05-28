#pragma once

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

    Logger&   logger_;
    ArgParser arg_parser_;
};
} // namespace mdk::app