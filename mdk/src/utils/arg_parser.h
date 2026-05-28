#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace mdk::utils
{

enum class TokenType : std::uint8_t
{
    Subcommand,
    Option,
    Flag,
    Value,
};

struct Token
{
    TokenType        type;
    std::string_view text;
};

class ArgParser
{
  public:
    ArgParser(int argc, char** argv);

    [[nodiscard]] std::optional<Token> peek(std::size_t offset) const;
    [[nodiscard]] std::optional<Token> next();

    [[nodiscard]] std::optional<Token> expect(TokenType type);
    [[nodiscard]] bool                 match(TokenType type);

  private:
    void tokenize(int argc, char** argv);

    std::vector<Token> tokens_;
    size_t             current_{0};
};

} // namespace mdk::utils