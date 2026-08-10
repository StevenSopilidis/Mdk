#include "server.h"

#include "app/app.h"
#include "utils/arg_parser.h"

#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace mdk::server
{
Server::~Server()
{
    if (server_fd_ > 0)
    {
        close(server_fd_);
    }
}

void Server::Run()
{
    server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd_ < 0)
    {
        throw std::runtime_error("Could not create unix socket");
    }

    unlink(kServerPath.data());

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, kServerPath.data(), sizeof(addr.sun_path) - 1);

    if (bind(server_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        throw std::runtime_error("Could not bind unix socket");
    }

    if (listen(server_fd_, SOMAXCONN) < 0)
    {
        throw std::runtime_error("Could not listen at unix socket");
    }

    LOG_INFO("Daemon unix server starting");

    while (true)
    {
        auto client_fd = accept(server_fd_, nullptr, nullptr);
        if (client_fd < 0)
        {
            LOG_ERROR("accept() failed");
            continue;
        }

        LOG_INFO("Client connected");

        std::array<char, 256> buffer;

        while (true)
        {
            ssize_t bytes = read(client_fd, buffer.data(), sizeof(buffer));

            if (bytes == 0)
            {
                // Client closed connection
                LOG_INFO("Client disconnected");
                break;
            }

            if (bytes < 0)
            {
                LOG_ERROR("read() failed");
                break;
            }

            auto data = std::string_view(buffer.data(), bytes);

            LOG_INFO("Received request: {}\n", data);

            auto parser = ArgParser(data);

            app::App::GetInstance().ProcessCommand(parser);

            LOG_INFO("Processed request2\n", data);

            memset(buffer.data(), 0, sizeof(buffer));
        }   

        close(client_fd);
    }
}
} // namespace mdk::server