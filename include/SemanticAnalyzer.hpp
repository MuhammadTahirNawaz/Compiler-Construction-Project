#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "AST.hpp"
#include "SymbolTable.hpp"

class SemanticAnalyzer : public ASTVisitor {
public:
    SemanticAnalyzer();

    void visitProgram(ProgramNode* node) override;
    void visitVarDecl(VarDeclNode* node) override;
    void visitAssign(AssignNode* node) override;
    void visitExprStmt(ExprStmtNode* node) override;
    void visitBlock(BlockNode* node) override;
    void visitWhileStmt(WhileStmtNode* node) override;
    void visitIfStmt(IfStmtNode* node) override;
    void visitInputStmt(InputStmtNode* node) override;
    void visitOutputStmt(OutputStmtNode* node) override;
    void visitForStmt(ForStmtNode* node) override;
    void visitFunctionDecl(FunctionDeclNode* node) override;
    void visitReturnStmt(ReturnStmtNode* node) override;
    void visitFunctionCall(FunctionCallNode* node) override;
    void visitBinaryExpr(BinaryExprNode* node) override;
    void visitUnaryExpr(UnaryExprNode* node) override;
    void visitLiteral(LiteralNode* node) override;
    void visitIdentifier(IdentifierNode* node) override;

    const std::vector<std::string>& errors() const;

private:
    struct FunctionSignature {
        DataType returnType;
        std::vector<DataType> paramTypes;
    };

    SymbolTable symbolTable_;
    std::vector<std::string> errors_;
    std::unordered_map<std::string, FunctionSignature> functionSignatures_;
    DataType currentReturnType_ = DataType::UNKNOWN;

    DataType inferLiteralType(const std::string& value) const;
    bool isNumeric(DataType type) const;
    void reportError(const std::string& message);
};

