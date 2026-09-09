#include "protocol.h"

#include <arpa/inet.h>
#include <cerrno>
#include <unistd.h>

namespace mdk::server
{
namespace
{

bool WriteAll(int fd, const char* data, std::size_t n)
{
    std::size_t off = 0;
    while (off < n)
    {
        const ssize_t written = ::write(fd, data + off, n - off);
        if (written < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return false;
        }
        if (written == 0)
        {
            return false;
        }
        off += static_cast<std::size_t>(written);
    }
    return true;
}

bool ReadAll(int fd, char* data, std::size_t n)
{
    std::size_t off = 0;
    while (off < n)
    {
        const ssize_t got = ::read(fd, data + off, n - off);
        if (got < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return false;
        }
        if (got == 0)
        {
            return false;
        }
        off += static_cast<std::size_t>(got);
    }
    return true;
}

} // namespace

bool WriteFrame(int fd, std::string_view payload)
{
    if (payload.size() > kMaxFrameBytes)
    {
        return false;
    }

    const std::uint32_t n = htonl(static_cast<std::uint32_t>(payload.size()));
    if (!WriteAll(fd, reinterpret_cast<const char*>(&n), sizeof(n)))
    {
        return false;
    }

    if (payload.empty())
    {
        return true;
    }

    return WriteAll(fd, payload.data(), payload.size());
}

std::optional<std::string> ReadFrame(int fd)
{
    std::uint32_t n_be = 0;
    if (!ReadAll(fd, reinterpret_cast<char*>(&n_be), sizeof(n_be)))
    {
        return std::nullopt;
    }

    const std::uint32_t n = ntohl(n_be);
    if (n > kMaxFrameBytes)
    {
        return std::nullopt;
    }

    std::string payload(n, '\0');
    if (n > 0 && !ReadAll(fd, payload.data(), n))
    {
        return std::nullopt;
    }

    return payload;
}

} // namespace mdk::server
