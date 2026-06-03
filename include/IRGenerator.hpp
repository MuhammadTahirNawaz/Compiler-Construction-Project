#pragma once

#include <string>
#include <vector>
#include "AST.hpp"

struct TacInstr {
    std::string op;
    std::string dest;
    std::string src1;
    std::string src2;

    std::string toString() const;
};

class IRGenerator : public ASTVisitor {
public:
    IRGenerator() = default;

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

    const std::vector<TacInstr>& getInstructions() const { return instructions_; }
    void print() const;

private:
    std::vector<TacInstr> instructions_;
    int tempCount_ = 0;
    int labelCount_ = 0;
    std::string lastResult_;

    std::string newTemp();
    std::string newLabel();
};
