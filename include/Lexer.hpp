#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Token.hpp"

class Lexer {
public:
    explicit Lexer(const std::string& filePath, std::size_t bufferSize = 4096);

    std::vector<Token> tokenize();

private:
    std::string filePath_;
    std::size_t bufferSize_;
};
