#include "app.h"

namespace mdk::app
{
App::App(int argc, char** argv) : logger_{Logger::getInstance()}, arg_parser_(ArgParser(argc, argv))
{
}

void App::Run()
{
    auto subcmd = arg_parser_.expect(TokenType::Subcommand);
    if (!subcmd.has_value())
    {
        LOG_ERROR("Expected subcmd as first arguemnt");
    }

    if (subcmd->text == std::string_view("help"))
    {
        handle_help();
    }
}

void App::handle_help()
{
    if (arg_parser_.peek(0) != std::nullopt)
    {
        LOG_ERROR("Invalid arguments passed to help");
        return;
    }

    LOG_INFO("MDK HELP....");
}

} // namespace mdk::app