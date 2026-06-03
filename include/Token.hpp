#pragma once

#include <string>

enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    BOOLEAN_LITERAL,
    OPERATOR,
    SEPARATOR,
    END_OF_FILE,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};
