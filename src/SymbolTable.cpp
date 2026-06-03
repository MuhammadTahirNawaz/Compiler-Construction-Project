#include "SymbolTable.hpp"

using namespace std;

SymbolTable::SymbolTable() {
    // Start with a global scope
    enterScope();
}

void SymbolTable::enterScope() {
    scopes_.push_back(unordered_map<string, SymbolInfo>());
}

void SymbolTable::exitScope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

bool SymbolTable::declare(const string& name, DataType type, const string& value) {
    if (scopes_.empty()) {
        return false;
    }
    // Check for duplicate declaration in the current scope only
    auto& currentScope = scopes_.back();
    if (currentScope.find(name) != currentScope.end()) {
        return false;
    }
    currentScope[name] = SymbolInfo{type, value};
    return true;
}

bool SymbolTable::assign(const string& name, const string& value) {
    // Look up the symbol from the most nested scope to the outermost
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto search = it->find(name);
        if (search != it->end()) {
            search->second.value = value;
            return true;
        }
    }
    return false;
}

const SymbolInfo* SymbolTable::lookup(const string& name) const {
    // Look up the symbol from the most nested scope to the outermost
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto search = it->find(name);
        if (search != it->end()) {
            return &(search->second);
        }
    }
    return nullptr;
}
