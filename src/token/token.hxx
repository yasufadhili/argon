#pragma once

#include <map>
#include <string>
#include <utility>
#include <sstream>

namespace token {
  enum class TokenType {
    INVALID,

    IDENTIFIER, TYPE_IDENTIFIER,

    INTEGER, FLOAT, STRING,

    RIGHT_ARROW, LEFT_ARROW,

    COLON, SEMICOLON, COMMA,

    PLUS, MINUS,
    GREATER, LESS,

    LEFT_BRACKET, RIGHT_BRACKET, LEFT_BRACE, RIGHT_BRACE, LEFT_PAREN, RIGHT_PAREN,

    RETURN, FUNCTION, MODULE, IMPORT,

    END_OF_LINE, END_OF_FILE,
  };

  inline std::map<TokenType, std::string> token_type_to_string_map{
    {TokenType::INVALID, "INVALID"},

    {TokenType::IDENTIFIER, "IDENTIFIER"},
    {TokenType::TYPE_IDENTIFIER, "TYPE_IDENTIFIER"},

    {TokenType::INTEGER, "INTEGER"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::STRING, "STRING"},

    {TokenType::RIGHT_ARROW, "RIGHT_ARROW"},
    {TokenType::LEFT_ARROW, "LEFT_ARROW"},

    {TokenType::COLON, "COLON"},
    {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::COMMA, "COMMA"},

    {TokenType::PLUS, "PLUS"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::GREATER, "GREATER"},
    {TokenType::LESS, "LESS"},

    {TokenType::LEFT_BRACKET, "LEFT_BRACKET"},
    {TokenType::RIGHT_BRACKET, "RIGHT_BRACKET"},
    {TokenType::LEFT_BRACE, "LEFT_BRACE"},
    {TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
    {TokenType::LEFT_PAREN, "LEFT_PAREN"},
    {TokenType::RIGHT_PAREN, "RIGHT_PAREN"},

    {TokenType::RETURN, "RETURN"},
    {TokenType::FUNCTION, "FUNCTION"},
    {TokenType::MODULE, "MODULE"},
    {TokenType::IMPORT, "IMPORT"},

    {TokenType::END_OF_LINE, "END_OF_LINE"},
    {TokenType::END_OF_FILE, "END_OF_FILE"},
  };

  inline auto get_token_type_string(const TokenType type) -> std::string {
    return token_type_to_string_map.at(type);
  }

  struct Token {
    TokenType type;
    std::string lexeme;
    size_t line;
    size_t column;
    size_t index;

    Token(const TokenType type, std::string lexeme, const size_t line, const size_t column, const size_t index)
      : type(type), lexeme(std::move(lexeme)), line(line), column(column), index(index) {
    }

    [[nodiscard]] auto to_string() const -> std::string;
  };

  inline auto Token::to_string() const -> std::string {
    std::stringstream ss;
    ss << "( ";
    ss << get_token_type_string(this->type) << ", ";
    ss << this->lexeme << ", ";
    ss << this->line << ":" << this->column;
    ss << " )";

    return ss.str();
  }
}
