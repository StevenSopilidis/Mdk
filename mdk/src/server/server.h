#pragma once

#include "utils/logger.h"

#include <string_view>

namespace mdk::server
{

using namespace mdk::utils;

class Server
{
  public:
    Server() = default;
    ~Server();

    Server(const Server&)            = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&)                 = default;
    Server& operator=(Server&&)      = default;
    void    Run();

  private:
    int                               server_fd_;
    static constexpr std::string_view kServerPath{"/tmp/mdk_daemon.sock"};
};
} // namespace mdk::server
