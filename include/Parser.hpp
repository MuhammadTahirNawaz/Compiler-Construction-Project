#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AST.hpp"
#include "Token.hpp"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<ProgramNode> parseProgram();
    const std::vector<std::string>& errors() const;

private:
    const std::vector<Token>& tokens_;
    std::size_t current_;
    std::vector<std::string> errors_;

    const Token& currentToken() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool checkLexeme(const std::string& lexeme) const;
    const Token& advance();
    bool match(TokenType type);
    bool matchLexeme(const std::string& lexeme);
    const Token& consume(TokenType type, const std::string& message);
    const Token& consumeLexeme(const std::string& lexeme, const std::string& message);

    std::unique_ptr<StmtNode> parseStatement();
    std::unique_ptr<StmtNode> parseVarDecl();
    std::unique_ptr<StmtNode> parseAssignment();
    std::unique_ptr<StmtNode> parseExprStmt();
    std::unique_ptr<StmtNode> parseBlock();
    std::unique_ptr<StmtNode> parseWhileStatement();
    std::unique_ptr<StmtNode> parseIfStatement();
    std::unique_ptr<StmtNode> parseInputStatement();
    std::unique_ptr<StmtNode> parseOutputStatement();
    std::unique_ptr<StmtNode> parseForStatement();
    std::unique_ptr<StmtNode> parseFunctionDecl();
    std::unique_ptr<StmtNode> parseReturnStatement();

    std::unique_ptr<ExprNode> parseExpression();
    std::unique_ptr<ExprNode> parseLogicalOr();
    std::unique_ptr<ExprNode> parseLogicalAnd();
    std::unique_ptr<ExprNode> parseComparison();
    std::unique_ptr<ExprNode> parseAdditive();
    std::unique_ptr<ExprNode> parseMultiplicative();
    std::unique_ptr<ExprNode> parseUnary();
    std::unique_ptr<ExprNode> parsePrimary();

    DataType dataTypeFromKeyword(const std::string& lexeme) const;
    void reportError(const Token& token, const std::string& message);
    void synchronize();
};
