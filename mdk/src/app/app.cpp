#include "app.h"

#include <iostream>

namespace mdk::app
{
App::App(int argc, char** argv) : logger_{Logger::GetInstance()}, arg_parser_(ArgParser(argc, argv))
{
}

void App::Run()
{
    auto subcmd = arg_parser_.Expect(TokenType::Subcommand);

    if (!subcmd.has_value())
    {
        LOG_ERROR("Expected subcmd as first arguemnt");
    }

    running_.store(true, std::memory_order_release);

    if (subcmd->text == std::string_view("help"))
    {
        HandleHelp();
    }
    else if (subcmd->text == std::string_view("run-raw"))
    {
        HandleRunRaw();
    }

    while (running_.load(std::memory_order_acquire))
    {
    }
}

void App::Stop() { running_.store(false, std::memory_order_acquire); }

void App::HandleRunRaw()
{
    // mdk run-raw <root-fs> <command>
    auto rootfs = arg_parser_.Expect(TokenType::Value);
    if (rootfs == std::nullopt)
    {
        LOG_ERROR("Expecting root-fs value");
        return;
    }

    auto command = arg_parser_.Expect(TokenType::Value);
    if (command == std::nullopt)
    {
        LOG_ERROR("Expecting command value");
        return;
    }

    container_manager_.CreateContainer(std::string(rootfs->text), std::string(command->text));
}

void App::HandleHelp()
{
    if (arg_parser_.Peek(0) != std::nullopt)
    {
        LOG_ERROR("Invalid arguments passed to help");
        return;
    }

    LOG_INFO("MDK HELP....");
}

} // namespace mdk::app