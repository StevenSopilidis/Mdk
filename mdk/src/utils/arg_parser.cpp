#include "arg_parser.h"

#include "utils/logger.h"

#include <string_view>
#include <vector>

namespace mdk::utils
{
ArgParser::ArgParser(std::string_view command)
{
    // parse received data into arc, argv equivelant
    std::istringstream iss((std::string(command)));

    std::vector<std::string> args;
    std::string              token;

    while (iss >> token)
    {
        args.push_back(std::move(token));
    }

    std::vector<char*> argv;
    argv.reserve(args.size());

    for (auto& arg : args)
    {
        argv.push_back(arg.data());
    }

    int argc = static_cast<int>(argv.size());

    Tokenize(argc, argv.data());
}

void ArgParser::Tokenize(int argc, char** argv)
{
    auto seen_subcmd{false};

    for (std::size_t i{0}; i < argc; i++)
    {
        std::string_view s{argv[i]};

        if (s.starts_with("--"))
        {
            tokens_.push_back({TokenType::Option, s.substr(2)});
        }
        else if (s.starts_with("-"))
        {
            tokens_.push_back({TokenType::Flag, s.substr(1)});
        }
        else if (!seen_subcmd)
        {
            tokens_.push_back({TokenType::Subcommand, s});
            seen_subcmd = true;
        }
        else
        {
            tokens_.push_back({TokenType::Value, s});
        }
    }
}

std::optional<Token> ArgParser::Peek(std::size_t offset) const
{
    if (current_ + offset >= tokens_.size())
    {
        return std::nullopt;
    }

    return tokens_.at(current_ + offset);
}

std::optional<Token> ArgParser::Next()
{
    if (current_ == tokens_.size())
    {
        return std::nullopt;
    }

    return tokens_[current_++];
}

std::optional<Token> ArgParser::Expect(TokenType type)
{
    if (current_ == tokens_.size() || tokens_[current_].type != type)
    {
        return std::nullopt;
    }

    return tokens_[current_++];
}

bool ArgParser::Match(TokenType type)
{
    if (current_ == tokens_.size())
    {
        return false;
    }

    return tokens_[current_++].type == type;
}

} // namespace mdk::utils