#include "server/protocol.h"
#include "third-party/nlohmann/json.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: mdk <command> [args...]\n";
        return 1;
    }

    nlohmann::json request = nlohmann::json::array();
    for (int i = 1; i < argc; ++i)
    {
        request.push_back(argv[i]);
    }

    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        std::cerr << "socket() failed: " << strerror(errno) << '\n';
        return 1;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::strncpy(address.sun_path, mdk::server::kServerPath.data(), sizeof(address.sun_path) - 1);

    if (connect(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
    {
        std::cerr << "connect() failed: " << strerror(errno) << '\n';
        close(socket_fd);
        return 1;
    }

    if (!mdk::server::WriteFrame(socket_fd, request.dump()))
    {
        std::cerr << "write() failed\n";
        close(socket_fd);
        return 1;
    }

    auto response = mdk::server::ReadFrame(socket_fd);
    close(socket_fd);

    if (!response)
    {
        std::cerr << "failed to read daemon response\n";
        return 1;
    }

    nlohmann::json parsed;
    try
    {
        parsed = nlohmann::json::parse(*response);
    }
    catch (const nlohmann::json::parse_error&)
    {
        std::cout << *response << '\n';
        return 1;
    }

    std::cout << parsed.dump(2) << '\n';

    if (!parsed.contains("ok") || parsed["ok"] != true)
    {
        return 1;
    }

    return 0;
}
