#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class DataType {
    INT,
    FLOAT,
    BOOL,
    VOID,
    UNKNOWN
};

class ASTVisitor;

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor* visitor) = 0;
};

class StmtNode : public ASTNode {};

class ExprNode : public ASTNode {
public:
    DataType inferredType = DataType::UNKNOWN;
};

class ProgramNode;
class VarDeclNode;
class AssignNode;
class ExprStmtNode;
class BlockNode;
class WhileStmtNode;
class IfStmtNode;
class InputStmtNode;
class OutputStmtNode;
class ForStmtNode;
class FunctionDeclNode;
class ReturnStmtNode;
class FunctionCallNode;
class BinaryExprNode;
class UnaryExprNode;
class LiteralNode;
class IdentifierNode;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visitProgram(ProgramNode* node) = 0;
    virtual void visitVarDecl(VarDeclNode* node) = 0;
    virtual void visitAssign(AssignNode* node) = 0;
    virtual void visitExprStmt(ExprStmtNode* node) = 0;
    virtual void visitBlock(BlockNode* node) = 0;
    virtual void visitWhileStmt(WhileStmtNode* node) = 0;
    virtual void visitIfStmt(IfStmtNode* node) = 0;
    virtual void visitInputStmt(InputStmtNode* node) = 0;
    virtual void visitOutputStmt(OutputStmtNode* node) = 0;
    virtual void visitForStmt(ForStmtNode* node) = 0;
    virtual void visitFunctionDecl(FunctionDeclNode* node) = 0;
    virtual void visitReturnStmt(ReturnStmtNode* node) = 0;
    virtual void visitFunctionCall(FunctionCallNode* node) = 0;
    virtual void visitBinaryExpr(BinaryExprNode* node) = 0;
    virtual void visitUnaryExpr(UnaryExprNode* node) = 0;
    virtual void visitLiteral(LiteralNode* node) = 0;
    virtual void visitIdentifier(IdentifierNode* node) = 0;
};

class ProgramNode : public ASTNode {
public:
    explicit ProgramNode(std::vector<std::unique_ptr<StmtNode>> stmts)
        : statements(std::move(stmts)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitProgram(this); }

    std::vector<std::unique_ptr<StmtNode>> statements;
};

class VarDeclNode : public StmtNode {
public:
    VarDeclNode(DataType declType, std::string ident, std::unique_ptr<ExprNode> init)
        : type(declType), name(std::move(ident)), initializer(std::move(init)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitVarDecl(this); }

    DataType type;
    std::string name;
    std::unique_ptr<ExprNode> initializer;
};

class AssignNode : public StmtNode {
public:
    AssignNode(std::string ident, std::unique_ptr<ExprNode> expr)
        : name(std::move(ident)), value(std::move(expr)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitAssign(this); }

    std::string name;
    std::unique_ptr<ExprNode> value;
};

class ExprStmtNode : public StmtNode {
public:
    explicit ExprStmtNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitExprStmt(this); }

    std::unique_ptr<ExprNode> expression;
};

class BlockNode : public StmtNode {
public:
    explicit BlockNode(std::vector<std::unique_ptr<StmtNode>> stmts)
        : statements(std::move(stmts)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitBlock(this); }

    std::vector<std::unique_ptr<StmtNode>> statements;
};

class WhileStmtNode : public StmtNode {
public:
    WhileStmtNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<StmtNode> stmt)
        : condition(std::move(cond)), body(std::move(stmt)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitWhileStmt(this); }

    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> body;
};

class IfStmtNode : public StmtNode {
public:
    IfStmtNode(std::unique_ptr<ExprNode> cond,
               std::unique_ptr<StmtNode> thenStmt,
               std::unique_ptr<StmtNode> elseStmt)
        : condition(std::move(cond)),
          thenBranch(std::move(thenStmt)),
          elseBranch(std::move(elseStmt)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitIfStmt(this); }

    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> thenBranch;
    std::unique_ptr<StmtNode> elseBranch;
};

class InputStmtNode : public StmtNode {
public:
    explicit InputStmtNode(std::string ident) : name(std::move(ident)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitInputStmt(this); }

    std::string name;
};

class OutputStmtNode : public StmtNode {
public:
    explicit OutputStmtNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitOutputStmt(this); }

    std::unique_ptr<ExprNode> expression;
};

class ForStmtNode : public StmtNode {
public:
    ForStmtNode(std::unique_ptr<StmtNode> init,
                std::unique_ptr<ExprNode> cond,
                std::unique_ptr<StmtNode> post,
                std::unique_ptr<StmtNode> stmt)
        : initializer(std::move(init)),
          condition(std::move(cond)),
          postCycle(std::move(post)),
          body(std::move(stmt)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitForStmt(this); }

    std::unique_ptr<StmtNode> initializer;
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> postCycle;
    std::unique_ptr<StmtNode> body;
};

class FunctionDeclNode : public StmtNode {
public:
    FunctionDeclNode(DataType retType,
                     std::string name,
                     std::vector<std::pair<DataType, std::string>> params,
                     std::unique_ptr<StmtNode> body)
        : returnType(retType),
          name(std::move(name)),
          parameters(std::move(params)),
          body(std::move(body)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitFunctionDecl(this); }

    DataType returnType;
    std::string name;
    std::vector<std::pair<DataType, std::string>> parameters;
    std::unique_ptr<StmtNode> body;
};

class ReturnStmtNode : public StmtNode {
public:
    explicit ReturnStmtNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitReturnStmt(this); }

    std::unique_ptr<ExprNode> expression;
};

class FunctionCallNode : public ExprNode {
public:
    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ExprNode>> args)
        : name(std::move(name)), arguments(std::move(args)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitFunctionCall(this); }

    std::string name;
    std::vector<std::unique_ptr<ExprNode>> arguments;
};

class BinaryExprNode : public ExprNode {
public:
    BinaryExprNode(std::string oper, std::unique_ptr<ExprNode> lhs, std::unique_ptr<ExprNode> rhs)
        : op(std::move(oper)), left(std::move(lhs)), right(std::move(rhs)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitBinaryExpr(this); }

    std::string op;
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
};

class UnaryExprNode : public ExprNode {
public:
    UnaryExprNode(std::string oper, std::unique_ptr<ExprNode> expr)
        : op(std::move(oper)), operand(std::move(expr)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitUnaryExpr(this); }

    std::string op;
    std::unique_ptr<ExprNode> operand;
};

class LiteralNode : public ExprNode {
public:
    explicit LiteralNode(std::string val) : value(std::move(val)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitLiteral(this); }

    std::string value;
};

class IdentifierNode : public ExprNode {
public:
    explicit IdentifierNode(std::string ident) : name(std::move(ident)) {}

    void accept(ASTVisitor* visitor) override { visitor->visitIdentifier(this); }

    std::string name;
};
