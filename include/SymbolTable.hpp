#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "AST.hpp"

struct SymbolInfo {
    DataType type = DataType::UNKNOWN;
    std::string value;
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();

    bool declare(const std::string& name, DataType type, const std::string& value);
    bool assign(const std::string& name, const std::string& value);
    const SymbolInfo* lookup(const std::string& name) const;

private:
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes_;
};
