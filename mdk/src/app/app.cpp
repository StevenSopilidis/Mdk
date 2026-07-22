#include "app.h"

#include "core/signal_handler.h"
#include "server/server.h"

#include <csignal>
#include <iostream>

using namespace mdk::server;

namespace mdk::app
{

App& App::GetInstance()
{
    static App app;
    return app;
}

void App::Run()
{
    core::InstallSignalHandler<SIGTERM>(DefaultSignalHandler);
    core::InstallSignalHandler<SIGINT>(DefaultSignalHandler);

    running_ = true;

    LOG_INFO("Started MDK DEAMON");

    main_loop_thread_ = std::jthread(
        [&]()
        {
            while (running_.load(std::memory_order_acquire))
            {
            }
        });
}

void App::Stop() { running_.store(false, std::memory_order_acquire); }

void App::ProcessCommand(int argc, char** argv)
{
    auto argParser = ArgParser(argc, argv);

    auto subcmd = argParser.Expect(TokenType::Subcommand);

    if (!subcmd.has_value())
    {
        LOG_ERROR("Expected subcmd as first arguemnt");
    }

    running_.store(true, std::memory_order_release);

    if (subcmd->text == std::string_view("help"))
    {
        HandleHelp(argParser);
    }
    else if (subcmd->text == std::string_view("run-raw"))
    {
        HandleRunRaw(argParser);
    }
}

void App::HandleRunRaw(ArgParser& argParser)
{
    // mdk run-raw <root-fs> <command>
    auto rootfs = argParser.Expect(TokenType::Value);
    if (rootfs == std::nullopt)
    {
        LOG_ERROR("Expecting root-fs value");
        return;
    }

    auto command = argParser.Expect(TokenType::Value);
    if (command == std::nullopt)
    {
        LOG_ERROR("Expecting command value");
        return;
    }

    container_manager_.CreateContainer(std::string(rootfs->text), std::string(command->text));
}

void App::HandleHelp(ArgParser& argParser)
{
    if (argParser.Peek(0) != std::nullopt)
    {
        LOG_ERROR("Invalid arguments passed to help");
        return;
    }

    LOG_INFO("MDK HELP....");
}

void App::DefaultSignalHandler(int sig)
{
    LOG_INFO("Received signal {}", sig);
    App::GetInstance().running_.store(false, std::memory_order_release);
}

} // namespace mdk::app