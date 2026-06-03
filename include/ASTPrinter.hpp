#pragma once

#include <vector>

#include "AST.hpp"

class ASTPrinter : public ASTVisitor {
public:
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

private:
    std::vector<bool> branchStack_;

    void printPrefix();
    void printNodeLine(const std::string& label);
    void visitChild(ASTNode* child, bool isLast);
    const char* dataTypeToString(DataType type) const;
};
