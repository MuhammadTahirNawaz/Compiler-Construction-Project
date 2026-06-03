#include <iostream>
#include <string>

#include "ASTPrinter.hpp"

using namespace std;

namespace {
const char* kColorNode = "\033[1;36m";
const char* kColorAccent = "\033[1;33m";
const char* kColorReset = "\033[0m";
}

void ASTPrinter::printPrefix() {
    for (size_t i = 0; i < branchStack_.size(); ++i) {
        if (i + 1 == branchStack_.size()) {
            cout << (branchStack_[i] ? "`-- " : "|-- ");
        } else {
            cout << (branchStack_[i] ? "    " : "|   ");
        }
    }
}

void ASTPrinter::printNodeLine(const string& label) {
    printPrefix();
    cout << kColorNode << label << kColorReset << "\n";
}

void ASTPrinter::visitChild(ASTNode* child, bool isLast) {
    if (!child) {
        return;
    }
    branchStack_.push_back(isLast);
    child->accept(this);
    branchStack_.pop_back();
}

const char* ASTPrinter::dataTypeToString(DataType type) const {
    switch (type) {
        case DataType::INT:
            return "int";
        case DataType::FLOAT:
            return "float";
        case DataType::BOOL:
            return "bool";
        case DataType::VOID:
            return "void";
        default:
            return "unknown";
    }
}

void ASTPrinter::visitProgram(ProgramNode* node) {
    printNodeLine("Program");
    size_t total = 0;
    for (const auto& stmt : node->statements) {
        if (stmt) {
            ++total;
        }
    }
    size_t index = 0;
    for (const auto& stmt : node->statements) {
        if (stmt) {
            ++index;
            visitChild(stmt.get(), index == total);
        }
    }
}

void ASTPrinter::visitVarDecl(VarDeclNode* node) {
    string label = "VarDecl: " + node->name + " : " + dataTypeToString(node->type);
    printNodeLine(label);
    visitChild(node->initializer.get(), true);
}

void ASTPrinter::visitAssign(AssignNode* node) {
    string label = "Assign: " + node->name;
    printNodeLine(label);
    visitChild(node->value.get(), true);
}

void ASTPrinter::visitExprStmt(ExprStmtNode* node) {
    printNodeLine("ExprStmt");
    visitChild(node->expression.get(), true);
}

void ASTPrinter::visitBlock(BlockNode* node) {
    printNodeLine("Block");
    size_t total = 0;
    for (const auto& stmt : node->statements) {
        if (stmt) {
            ++total;
        }
    }
    size_t index = 0;
    for (const auto& stmt : node->statements) {
        if (stmt) {
            ++index;
            visitChild(stmt.get(), index == total);
        }
    }
}

void ASTPrinter::visitWhileStmt(WhileStmtNode* node) {
    printNodeLine("WhileStmt");
    bool hasCondition = node->condition != nullptr;
    bool hasBody = node->body != nullptr;
    if (hasCondition) {
        visitChild(node->condition.get(), !hasBody);
    }
    if (hasBody) {
        visitChild(node->body.get(), true);
    }
}

void ASTPrinter::visitIfStmt(IfStmtNode* node) {
    printNodeLine("IfStmt");
    bool hasCondition = node->condition != nullptr;
    bool hasThen = node->thenBranch != nullptr;
    bool hasElse = node->elseBranch != nullptr;
    if (hasCondition) {
        visitChild(node->condition.get(), !hasThen && !hasElse);
    }
    if (hasThen) {
        visitChild(node->thenBranch.get(), !hasElse);
    }
    if (hasElse) {
        visitChild(node->elseBranch.get(), true);
    }
}

void ASTPrinter::visitInputStmt(InputStmtNode* node) {
    string label = "InputStmt: " + node->name;
    printNodeLine(label);
}

void ASTPrinter::visitOutputStmt(OutputStmtNode* node) {
    printNodeLine("OutputStmt");
    visitChild(node->expression.get(), true);
}

void ASTPrinter::visitForStmt(ForStmtNode* node) {
    printNodeLine("ForStmt");
    bool hasInit = node->initializer != nullptr;
    bool hasCond = node->condition != nullptr;
    bool hasPost = node->postCycle != nullptr;
    bool hasBody = node->body != nullptr;

    if (hasInit) {
        visitChild(node->initializer.get(), !hasCond && !hasPost && !hasBody);
    }
    if (hasCond) {
        visitChild(node->condition.get(), !hasPost && !hasBody);
    }
    if (hasPost) {
        visitChild(node->postCycle.get(), !hasBody);
    }
    if (hasBody) {
        visitChild(node->body.get(), true);
    }
}

void ASTPrinter::visitFunctionDecl(FunctionDeclNode* node) {
    string paramsStr;
    for (size_t i = 0; i < node->parameters.size(); ++i) {
        if (i > 0) paramsStr += ", ";
        paramsStr += string(dataTypeToString(node->parameters[i].first)) + " " + node->parameters[i].second;
    }
    string label = "FunctionDecl: " + node->name + "(" + paramsStr + ") -> " + dataTypeToString(node->returnType);
    printNodeLine(label);
    if (node->body) {
        visitChild(node->body.get(), true);
    }
}

void ASTPrinter::visitReturnStmt(ReturnStmtNode* node) {
    printNodeLine("ReturnStmt");
    if (node->expression) {
        visitChild(node->expression.get(), true);
    }
}

void ASTPrinter::visitFunctionCall(FunctionCallNode* node) {
    string label = "FunctionCall: " + node->name;
    printNodeLine(label);
    size_t total = node->arguments.size();
    for (size_t i = 0; i < total; ++i) {
        if (node->arguments[i]) {
            visitChild(node->arguments[i].get(), i + 1 == total);
        }
    }
}

void ASTPrinter::visitBinaryExpr(BinaryExprNode* node) {
    string label = string("BinaryExpr: ") + kColorAccent + node->op + kColorReset;
    printNodeLine(label);
    bool hasLeft = node->left != nullptr;
    bool hasRight = node->right != nullptr;
    if (hasLeft) {
        visitChild(node->left.get(), !hasRight);
    }
    if (hasRight) {
        visitChild(node->right.get(), true);
    }
}

void ASTPrinter::visitUnaryExpr(UnaryExprNode* node) {
    string label = string("UnaryExpr: ") + kColorAccent + node->op + kColorReset;
    printNodeLine(label);
    visitChild(node->operand.get(), true);
}

void ASTPrinter::visitLiteral(LiteralNode* node) {
    string label = string("Literal: ") + node->value;
    printNodeLine(label);
}

void ASTPrinter::visitIdentifier(IdentifierNode* node) {
    string label = string("Identifier: ") + node->name;
    printNodeLine(label);
}
