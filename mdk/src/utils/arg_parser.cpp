#include "arg_parser.h"

#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

namespace mdk::utils
{

ArgParser::ArgParser(std::string_view command)
{
    std::istringstream iss{std::string(command)};
    std::vector<std::string> args;
    std::string              token;

    while (iss >> token)
    {
        args.push_back(std::move(token));
    }

    Tokenize(args);
}

ArgParser::ArgParser(std::vector<std::string> args) { Tokenize(args); }

void ArgParser::Tokenize(const std::vector<std::string>& args)
{
    auto seen_subcmd{false};

    for (const auto& arg : args)
    {
        std::string_view s{arg};

        if (s.starts_with("--"))
        {
            tokens_.push_back({TokenType::Option, std::string(s.substr(2))});
        }
        else if (s.starts_with("-") && s.size() > 1)
        {
            tokens_.push_back({TokenType::Flag, std::string(s.substr(1))});
        }
        else if (!seen_subcmd)
        {
            tokens_.push_back({TokenType::Subcommand, arg});
            seen_subcmd = true;
        }
        else
        {
            tokens_.push_back({TokenType::Value, arg});
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
    if (current_ == tokens_.size() || tokens_[current_].type != type)
    {
        return false;
    }

    ++current_;
    return true;
}

} // namespace mdk::utils
