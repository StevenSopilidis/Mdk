#include "server.h"

#include "app/app.h"
#include "server/protocol.h"
#include "utils/arg_parser.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "third-party/nlohmann/json.hpp"

namespace mdk::server
{
namespace
{

nlohmann::json ErrorJson(std::string message)
{
    return {{"ok", false}, {"error", std::move(message)}};
}

nlohmann::json ParseRequest(const std::string& payload, std::vector<std::string>& args)
{
    nlohmann::json parsed;
    try
    {
        parsed = nlohmann::json::parse(payload);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        return ErrorJson(std::string("invalid request json: ") + e.what());
    }

    if (!parsed.is_array() || parsed.empty())
    {
        return ErrorJson("request must be a non-empty JSON array of strings");
    }

    args.clear();
    args.reserve(parsed.size());
    for (const auto& item : parsed)
    {
        if (!item.is_string())
        {
            return ErrorJson("request array entries must be strings");
        }
        args.push_back(item.get<std::string>());
    }

    return {};
}

} // namespace

Server::~Server()
{
    const int fd = server_fd_.exchange(-1, std::memory_order_acq_rel);
    if (fd >= 0)
    {
        unlink(kServerPath.data());
        close(fd);
    }
}

void Server::Shutdown()
{
    const int fd = server_fd_.load(std::memory_order_acquire);
    if (fd >= 0)
    {
        ::shutdown(fd, SHUT_RDWR);
    }
}

void Server::Run(std::atomic<bool>& running)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        throw std::runtime_error("Could not create unix socket");
    }

    unlink(kServerPath.data());

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, kServerPath.data(), sizeof(addr.sun_path) - 1);

    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        close(fd);
        throw std::runtime_error("Could not bind unix socket");
    }

    if (listen(fd, SOMAXCONN) < 0)
    {
        unlink(kServerPath.data());
        close(fd);
        throw std::runtime_error("Could not listen at unix socket");
    }

    server_fd_.store(fd, std::memory_order_release);

    LOG_INFO("Daemon unix server starting");

    while (running.load(std::memory_order_acquire))
    {
        const int client_fd = accept(fd, nullptr, nullptr);
        if (client_fd < 0)
        {
            if (!running.load(std::memory_order_acquire) || errno == EBADF || errno == EINVAL)
            {
                break;
            }
            if (errno == EINTR)
            {
                continue;
            }
            LOG_ERROR("accept() failed: {}", strerror(errno));
            continue;
        }

        LOG_INFO("Client connected");

        auto payload = ReadFrame(client_fd);
        if (!payload)
        {
            LOG_ERROR("failed to read request frame");
            close(client_fd);
            continue;
        }

        LOG_INFO("Received request: {}", *payload);

        std::vector<std::string> args;
        auto                     parse_error = ParseRequest(*payload, args);

        std::string response;
        if (!parse_error.is_null())
        {
            response = parse_error.dump();
        }
        else
        {
            auto parser = mdk::utils::ArgParser(std::move(args));
            response    = app::App::GetInstance().ProcessCommand(parser);
        }

        if (!WriteFrame(client_fd, response))
        {
            LOG_ERROR("failed to write response frame");
        }

        close(client_fd);
    }

    const int owned = server_fd_.exchange(-1, std::memory_order_acq_rel);
    if (owned >= 0)
    {
        unlink(kServerPath.data());
        close(owned);
    }
}
} // namespace mdk::server
