#include "utils/logger.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/un.h>
#include <system_error>
#include <unistd.h>

constexpr std::string_view kServerPath = "/tmp/mdk_daemon.sock";

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: mdk <command> [args...]\n";
        return 1;
    }

    // Combine all arguments into a single command string
    std::string request;

    for (int i = 1; i < argc; ++i)
    {
        if (i > 1)
        {
            request += ' ';
        }

        request += argv[i];
    }

    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        std::cout << "socket() failed";
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;

    std::ranges::copy(kServerPath, address.sun_path);

    if (connect(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
    {
        close(socket_fd);

        std::cout << "connect() failed";
    }

    const auto bytes = write(socket_fd, request.data(), request.size());

    if (bytes < 0)
    {
        close(socket_fd);

        std::cout << "write() failed\n";
    }

    close(socket_fd);

    std::cout << "Command sent: " << request << '\n';
}