#pragma once

#include <cstdint>
#include <optional>
#include <string>
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
    TokenType   type;
    std::string text;
};

class ArgParser
{
  public:
    explicit ArgParser(std::string_view command);
    explicit ArgParser(std::vector<std::string> args);

    [[nodiscard]] std::optional<Token> Peek(std::size_t offset) const;
    [[nodiscard]] std::optional<Token> Next();

    [[nodiscard]] std::optional<Token> Expect(TokenType type);
    [[nodiscard]] bool                 Match(TokenType type);

  private:
    void Tokenize(const std::vector<std::string>& args);

    std::vector<Token> tokens_;
    std::size_t        current_{0};
};

} // namespace mdk::utils
