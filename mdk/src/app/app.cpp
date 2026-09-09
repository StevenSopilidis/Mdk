#include "app.h"

#include "core/signal_handler.h"
#include "third-party/nlohmann/json.hpp"

#include <csignal>
#include <optional>
#include <utility>
#include <vector>

namespace mdk::app
{
namespace
{

nlohmann::json OkJson(nlohmann::json fields = nlohmann::json::object())
{
    fields["ok"] = true;
    return fields;
}

nlohmann::json ErrJson(std::string message)
{
    return {{"ok", false}, {"error", std::move(message)}};
}

} // namespace

App& App::GetInstance()
{
    static App app;
    return app;
}

void App::Run()
{
    core::InstallSignalHandler<SIGTERM>(DefaultSignalHandler);
    core::InstallSignalHandler<SIGINT>(DefaultSignalHandler);

    running_.store(true, std::memory_order_release);
    container_manager_.LaunchReapThread();

    LOG_INFO("Started MDK DAEMON");

    server_.Run(running_);

    container_manager_.Stop();
}

void App::Stop()
{
    running_.store(false, std::memory_order_release);
    server_.Shutdown();
    container_manager_.Stop();
}

std::string App::ProcessCommand(ArgParser& argParser)
{
    auto subcmd = argParser.Expect(TokenType::Subcommand);

    if (!subcmd.has_value())
    {
        LOG_ERROR("Expected subcommand as first argument");
        return ErrJson("Expected subcommand as first argument").dump();
    }

    if (subcmd->text == "help")
    {
        return HandleHelp(argParser);
    }
    if (subcmd->text == "run-raw")
    {
        return HandleRunRaw(argParser);
    }
    if (subcmd->text == "list")
    {
        return HandleList(argParser);
    }

    LOG_ERROR("Unknown subcommand {}", subcmd->text);
    return ErrJson("Unknown subcommand: " + subcmd->text).dump();
}

std::string App::HandleRunRaw(ArgParser& argParser)
{
    auto rootfs = argParser.Expect(TokenType::Value);
    if (rootfs == std::nullopt)
    {
        LOG_ERROR("Expecting root-fs value");
        return ErrJson("Expecting root-fs value").dump();
    }

    std::vector<std::string> argv;
    while (auto tok = argParser.Next())
    {
        if (tok->type != TokenType::Value)
        {
            return ErrJson("Unexpected argument: " + tok->text).dump();
        }
        argv.push_back(std::move(tok->text));
    }

    if (argv.empty())
    {
        LOG_ERROR("Expecting command value");
        return ErrJson("Expecting command value").dump();
    }

    auto created = container_manager_.CreateContainer(rootfs->text, argv);
    if (!created)
    {
        return ErrJson("Container creation failed").dump();
    }

    return OkJson({{"id", created->id}, {"pid", created->pid}, {"command", argv}}).dump();
}

std::string App::HandleList(ArgParser& argParser)
{
    if (argParser.Peek(0) != std::nullopt)
    {
        LOG_ERROR("Invalid arguments passed to list");
        return ErrJson("Invalid arguments passed to list").dump();
    }

    const auto containers =
        container_manager_.GetContainers() | std::views::values |
        std::views::transform(
            [](const auto& container)
            {
                return "id=" + std::to_string(container->get_id()) +
                       " pid=" + std::to_string(container->get_pid()) + " process_state=" +
                       std::to_string(static_cast<int>(container->process_state()));
            }) |
        std::ranges::to<std::vector<std::string>>();

    return OkJson({{"containers", containers}}).dump();
}

std::string App::HandleHelp(ArgParser& argParser)
{
    if (argParser.Peek(0) != std::nullopt)
    {
        LOG_ERROR("Invalid arguments passed to help");
        return ErrJson("Invalid arguments passed to help").dump();
    }

    return OkJson({{"usage", "mdk <command> [args...]"},
                   {"commands",
                    nlohmann::json::array({"help", "run-raw <rootfs> <command> [args...]"})}})
        .dump();
}

void App::DefaultSignalHandler(int)
{
    auto& app = App::GetInstance();
    app.running_.store(false, std::memory_order_release);
    app.server_.Shutdown();
}

} // namespace mdk::app
