#include "arg_parser.h"

namespace mdk::utils
{
ArgParser::ArgParser(int argc, char** argv) { tokenize(argc, argv); }

void ArgParser::tokenize(int argc, char** argv)
{
    auto seen_subcmd{false};

    for (std::size_t i{1}; i < argc; i++)
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

std::optional<Token> ArgParser::peek(std::size_t offset) const
{
    if (current_ + offset >= tokens_.size())
    {
        return std::nullopt;
    }

    return tokens_.at(current_ + offset);
}

std::optional<Token> ArgParser::next()
{
    if (current_ == tokens_.size())
    {
        return std::nullopt;
    }

    return tokens_[current_++];
}

std::optional<Token> ArgParser::expect(TokenType type)
{
    if (current_ == tokens_.size() || tokens_[current_].type != type)
    {
        return std::nullopt;
    }

    return tokens_[current_++];
}

bool ArgParser::match(TokenType type)
{
    if (current_ == tokens_.size())
    {
        return false;
    }

    return tokens_[current_++].type == type;
}

} // namespace mdk::utils