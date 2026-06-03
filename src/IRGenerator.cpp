#include "IRGenerator.hpp"
#include <iostream>

using namespace std;

string TacInstr::toString() const {
    if (op == "LABEL") {
        return dest + ":";
    }
    if (op == "GOTO") {
        return "  goto " + dest;
    }
    if (op == "IF_FALSE_GOTO") {
        return "  ifFalse " + src1 + " goto " + dest;
    }
    if (op == "ASSIGN") {
        return "  " + dest + " = " + src1;
    }
    if (op == "INPUT") {
        return "  input " + dest;
    }
    if (op == "OUTPUT") {
        return "  output " + dest;
    }
    if (op == "RETURN") {
        if (src1.empty()) return "  return";
        return "  return " + src1;
    }
    if (op == "PARAM") {
        return "  param " + dest;
    }
    if (op == "CALL") {
        if (dest.empty()) {
            return "  call " + src1;
        }
        return "  " + dest + " = call " + src1;
    }
    return "  " + dest + " = " + src1 + " " + op + " " + src2;
}

string IRGenerator::newTemp() {
    return "t" + to_string(tempCount_++);
}

string IRGenerator::newLabel() {
    return "L" + to_string(labelCount_++);
}

void IRGenerator::visitProgram(ProgramNode* node) {
    for (const auto& stmt : node->statements) {
        if (stmt) {
            stmt->accept(this);
        }
    }
}

void IRGenerator::visitVarDecl(VarDeclNode* node) {
    if (node->initializer) {
        node->initializer->accept(this);
        instructions_.push_back(TacInstr{"ASSIGN", node->name, lastResult_, ""});
    }
}

void IRGenerator::visitAssign(AssignNode* node) {
    if (node->value) {
        node->value->accept(this);
        instructions_.push_back(TacInstr{"ASSIGN", node->name, lastResult_, ""});
    }
}

void IRGenerator::visitExprStmt(ExprStmtNode* node) {
    if (node->expression) {
        node->expression->accept(this);
    }
}

void IRGenerator::visitBlock(BlockNode* node) {
    for (const auto& stmt : node->statements) {
        if (stmt) {
            stmt->accept(this);
        }
    }
}

void IRGenerator::visitWhileStmt(WhileStmtNode* node) {
    string startLabel = newLabel();
    string endLabel = newLabel();

    instructions_.push_back(TacInstr{"LABEL", startLabel, "", ""});
    if (node->condition) {
        node->condition->accept(this);
        instructions_.push_back(TacInstr{"IF_FALSE_GOTO", endLabel, lastResult_, ""});
    }

    if (node->body) {
        node->body->accept(this);
    }

    instructions_.push_back(TacInstr{"GOTO", startLabel, "", ""});
    instructions_.push_back(TacInstr{"LABEL", endLabel, "", ""});
}

void IRGenerator::visitIfStmt(IfStmtNode* node) {
    string elseLabel = newLabel();
    string endLabel = newLabel();

    if (node->condition) {
        node->condition->accept(this);
        instructions_.push_back(TacInstr{"IF_FALSE_GOTO", elseLabel, lastResult_, ""});
    }

    if (node->thenBranch) {
        node->thenBranch->accept(this);
    }

    if (node->elseBranch) {
        instructions_.push_back(TacInstr{"GOTO", endLabel, "", ""});
    }

    instructions_.push_back(TacInstr{"LABEL", elseLabel, "", ""});

    if (node->elseBranch) {
        node->elseBranch->accept(this);
        instructions_.push_back(TacInstr{"LABEL", endLabel, "", ""});
    }
}

void IRGenerator::visitInputStmt(InputStmtNode* node) {
    instructions_.push_back(TacInstr{"INPUT", node->name, "", ""});
}

void IRGenerator::visitOutputStmt(OutputStmtNode* node) {
    if (node->expression) {
        node->expression->accept(this);
        instructions_.push_back(TacInstr{"OUTPUT", lastResult_, "", ""});
    }
}

void IRGenerator::visitForStmt(ForStmtNode* node) {
    if (node->initializer) {
        node->initializer->accept(this);
    }

    string startLabel = newLabel();
    string endLabel = newLabel();

    instructions_.push_back(TacInstr{"LABEL", startLabel, "", ""});
    if (node->condition) {
        node->condition->accept(this);
        instructions_.push_back(TacInstr{"IF_FALSE_GOTO", endLabel, lastResult_, ""});
    }

    if (node->body) {
        node->body->accept(this);
    }

    if (node->postCycle) {
        node->postCycle->accept(this);
    }

    instructions_.push_back(TacInstr{"GOTO", startLabel, "", ""});
    instructions_.push_back(TacInstr{"LABEL", endLabel, "", ""});
}

void IRGenerator::visitFunctionDecl(FunctionDeclNode* node) {
    instructions_.push_back(TacInstr{"LABEL", "FUNC_" + node->name, "", ""});
    if (node->body) {
        node->body->accept(this);
    }
    // implicit return for safety
    instructions_.push_back(TacInstr{"RETURN", "", "", ""});
}

void IRGenerator::visitReturnStmt(ReturnStmtNode* node) {
    string retVal = "";
    if (node->expression) {
        node->expression->accept(this);
        retVal = lastResult_;
    }
    instructions_.push_back(TacInstr{"RETURN", "", retVal, ""});
}

void IRGenerator::visitFunctionCall(FunctionCallNode* node) {
    vector<string> args;
    for (const auto& arg : node->arguments) {
        if (arg) {
            arg->accept(this);
            args.push_back(lastResult_);
        }
    }
    for (const auto& argName : args) {
        instructions_.push_back(TacInstr{"PARAM", argName, "", ""});
    }
    string temp = newTemp();
    instructions_.push_back(TacInstr{"CALL", temp, node->name, ""});
    lastResult_ = temp;
}

void IRGenerator::visitBinaryExpr(BinaryExprNode* node) {
    if (!node->left || !node->right) return;

    node->left->accept(this);
    string lhs = lastResult_;

    node->right->accept(this);
    string rhs = lastResult_;

    string temp = newTemp();
    instructions_.push_back(TacInstr{node->op, temp, lhs, rhs});
    lastResult_ = temp;
}

void IRGenerator::visitUnaryExpr(UnaryExprNode* node) {
    if (!node->operand) return;

    node->operand->accept(this);
    string operand = lastResult_;

    string temp = newTemp();
    instructions_.push_back(TacInstr{node->op, temp, operand, ""});
    lastResult_ = temp;
}

void IRGenerator::visitLiteral(LiteralNode* node) {
    lastResult_ = node->value;
}

void IRGenerator::visitIdentifier(IdentifierNode* node) {
    lastResult_ = node->name;
}

void IRGenerator::print() const {
    for (const auto& inst : instructions_) {
        cout << inst.toString() << "\n";
    }
}
