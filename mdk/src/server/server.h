#pragma once

#include "utils/logger.h"

#include <atomic>
#include <string_view>

namespace mdk::server
{

class Server
{
  public:
    Server() = default;
    ~Server();

    Server(const Server&)            = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&)                 = delete;
    Server& operator=(Server&&)      = delete;

    void Run(std::atomic<bool>& running);
    void Shutdown();

  private:
    std::atomic<int> server_fd_{-1};
};

} // namespace mdk::server
