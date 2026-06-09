#include "app.h"

#include <iostream>

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
    else if (subcmd->text == std::string_view("run-raw"))
    {
        handle_run_raw();
    }
}

void App::handle_run_raw()
{
    // mdk run-raw <root-fs> <command>
    auto rootfs = arg_parser_.expect(TokenType::Value);
    if (rootfs == std::nullopt)
    {
        LOG_ERROR("Expecting root-fs value");
        return;
    }

    auto command = arg_parser_.expect(TokenType::Value);
    if (command == std::nullopt)
    {
        LOG_ERROR("Expecting command value");
        return;
    }

    container_manager_.create_container(std::string(rootfs->text), std::string(command->text));
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