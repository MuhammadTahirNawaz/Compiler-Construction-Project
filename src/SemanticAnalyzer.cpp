#include <sstream>

#include "SemanticAnalyzer.hpp"

using namespace std;

SemanticAnalyzer::SemanticAnalyzer() {}

void SemanticAnalyzer::visitProgram(ProgramNode* node) {
	for (const auto& stmt : node->statements) {
		if (stmt) {
			stmt->accept(this);
		}
	}
}

void SemanticAnalyzer::visitVarDecl(VarDeclNode* node) {
	if (!symbolTable_.declare(node->name, node->type, "")) {
		reportError("Duplicate declaration of '" + node->name + "'.");
		return;
	}

	if (node->initializer) {
		node->initializer->accept(this);
		if (node->initializer->inferredType != DataType::UNKNOWN &&
			node->initializer->inferredType != node->type) {
			reportError("Type mismatch in initializer for '" + node->name + "'.");
		}
	}
}

void SemanticAnalyzer::visitAssign(AssignNode* node) {
	const SymbolInfo* info = symbolTable_.lookup(node->name);
	if (!info) {
		reportError("Undeclared variable '" + node->name + "'.");
		return;
	}

	if (node->value) {
		node->value->accept(this);
		if (node->value->inferredType != DataType::UNKNOWN &&
			node->value->inferredType != info->type) {
			reportError("Type mismatch in assignment to '" + node->name + "'.");
		}
	}
}

void SemanticAnalyzer::visitExprStmt(ExprStmtNode* node) {
	if (node->expression) {
		node->expression->accept(this);
	}
}

void SemanticAnalyzer::visitBlock(BlockNode* node) {
	symbolTable_.enterScope();

	for (const auto& stmt : node->statements) {
		if (stmt) {
			stmt->accept(this);
		}
	}

	symbolTable_.exitScope();
}

void SemanticAnalyzer::visitWhileStmt(WhileStmtNode* node) {
	if (node->condition) {
		node->condition->accept(this);
		if (node->condition->inferredType != DataType::BOOL &&
			node->condition->inferredType != DataType::UNKNOWN) {
			reportError("While condition must be boolean.");
		}
	}

	if (node->body) {
		node->body->accept(this);
	}
}

void SemanticAnalyzer::visitIfStmt(IfStmtNode* node) {
	if (node->condition) {
		node->condition->accept(this);
		if (node->condition->inferredType != DataType::BOOL &&
			node->condition->inferredType != DataType::UNKNOWN) {
			reportError("If condition must be boolean.");
		}
	}

	if (node->thenBranch) {
		node->thenBranch->accept(this);
	}
	if (node->elseBranch) {
		node->elseBranch->accept(this);
	}
}

void SemanticAnalyzer::visitInputStmt(InputStmtNode* node) {
	const SymbolInfo* info = symbolTable_.lookup(node->name);
	if (!info) {
		reportError("Undeclared variable '" + node->name + "' in input.");
	}
}

void SemanticAnalyzer::visitOutputStmt(OutputStmtNode* node) {
	if (node->expression) {
		node->expression->accept(this);
		if (node->expression->inferredType == DataType::UNKNOWN) {
			reportError("Invalid expression type in output.");
		}
	}
}

void SemanticAnalyzer::visitForStmt(ForStmtNode* node) {
	symbolTable_.enterScope();

	if (node->initializer) {
		node->initializer->accept(this);
	}

	if (node->condition) {
		node->condition->accept(this);
		if (node->condition->inferredType != DataType::BOOL &&
			node->condition->inferredType != DataType::UNKNOWN) {
			reportError("For condition must be boolean.");
		}
	}

	if (node->postCycle) {
		node->postCycle->accept(this);
	}

	if (node->body) {
		node->body->accept(this);
	}

	symbolTable_.exitScope();
}

void SemanticAnalyzer::visitFunctionDecl(FunctionDeclNode* node) {
	if (functionSignatures_.find(node->name) != functionSignatures_.end()) {
		reportError("Duplicate function declaration of '" + node->name + "'.");
		return;
	}
	FunctionSignature sig;
	sig.returnType = node->returnType;
	for (const auto& p : node->parameters) {
		sig.paramTypes.push_back(p.first);
	}
	functionSignatures_[node->name] = sig;

	// Declare the function in the outer scope
	symbolTable_.declare(node->name, node->returnType, "");

	// Enter new scope for parameters and body
	symbolTable_.enterScope();
	for (const auto& p : node->parameters) {
		if (!symbolTable_.declare(p.second, p.first, "")) {
			reportError("Duplicate parameter name '" + p.second + "' in function '" + node->name + "'.");
		}
	}

	DataType prevRet = currentReturnType_;
	currentReturnType_ = node->returnType;

	if (node->body) {
		node->body->accept(this);
	}

	currentReturnType_ = prevRet;
	symbolTable_.exitScope();
}

void SemanticAnalyzer::visitReturnStmt(ReturnStmtNode* node) {
	DataType retExprType = DataType::VOID;
	if (node->expression) {
		node->expression->accept(this);
		retExprType = node->expression->inferredType;
	}
	if (retExprType != currentReturnType_) {
		reportError("Type mismatch: function return type does not match return expression.");
	}
}

void SemanticAnalyzer::visitFunctionCall(FunctionCallNode* node) {
	auto it = functionSignatures_.find(node->name);
	if (it == functionSignatures_.end()) {
		reportError("Undeclared function '" + node->name + "'.");
		node->inferredType = DataType::UNKNOWN;
		return;
	}

	const FunctionSignature& sig = it->second;
	if (node->arguments.size() != sig.paramTypes.size()) {
		reportError("Function '" + node->name + "' expects " +
					to_string(sig.paramTypes.size()) + " arguments, but " +
					to_string(node->arguments.size()) + " were provided.");
	} else {
		for (size_t i = 0; i < node->arguments.size(); ++i) {
			if (node->arguments[i]) {
				node->arguments[i]->accept(this);
				if (node->arguments[i]->inferredType != sig.paramTypes[i] &&
					node->arguments[i]->inferredType != DataType::UNKNOWN) {
					reportError("Type mismatch in argument " + to_string(i + 1) +
								" of call to '" + node->name + "'.");
				}
			}
		}
	}
	node->inferredType = sig.returnType;
}

void SemanticAnalyzer::visitBinaryExpr(BinaryExprNode* node) {
	if (!node->left || !node->right) {
		return;
	}

	node->left->accept(this);
	node->right->accept(this);

	DataType leftType = node->left->inferredType;
	DataType rightType = node->right->inferredType;
	const string& op = node->op;

	if (op == "&&" || op == "||") {
		if (leftType != DataType::BOOL || rightType != DataType::BOOL) {
			reportError("Logical operator requires boolean operands.");
		}
		node->inferredType = DataType::BOOL;
		return;
	}

	if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
		if (leftType != rightType) {
			reportError("Comparison operands must have the same type.");
		}
		node->inferredType = DataType::BOOL;
		return;
	}

	if (op == "+" || op == "-" || op == "*" || op == "/") {
		if (!isNumeric(leftType) || !isNumeric(rightType)) {
			reportError("Arithmetic operator requires numeric operands.");
			node->inferredType = DataType::UNKNOWN;
			return;
		}
		node->inferredType = (leftType == DataType::FLOAT || rightType == DataType::FLOAT)
								 ? DataType::FLOAT
								 : DataType::INT;
		return;
	}

	node->inferredType = DataType::UNKNOWN;
}

void SemanticAnalyzer::visitUnaryExpr(UnaryExprNode* node) {
	if (!node->operand) {
		return;
	}

	node->operand->accept(this);
	if (node->op == "!") {
		if (node->operand->inferredType != DataType::BOOL) {
			reportError("Logical negation requires a boolean operand.");
		}
		node->inferredType = DataType::BOOL;
		return;
	}

	if (node->op == "-") {
		if (!isNumeric(node->operand->inferredType)) {
			reportError("Unary '-' requires a numeric operand.");
			node->inferredType = DataType::UNKNOWN;
			return;
		}
		node->inferredType = node->operand->inferredType;
		return;
	}

	node->inferredType = DataType::UNKNOWN;
}

void SemanticAnalyzer::visitLiteral(LiteralNode* node) {
	node->inferredType = inferLiteralType(node->value);
}

void SemanticAnalyzer::visitIdentifier(IdentifierNode* node) {
	const SymbolInfo* info = symbolTable_.lookup(node->name);
	if (!info) {
		reportError("Undeclared variable '" + node->name + "'.");
		node->inferredType = DataType::UNKNOWN;
	} else {
		node->inferredType = info->type;
	}
}

const vector<string>& SemanticAnalyzer::errors() const {
	return errors_;
}

DataType SemanticAnalyzer::inferLiteralType(const string& value) const {
	if (value == "true" || value == "false") {
		return DataType::BOOL;
	}

	if (value.find('.') != string::npos) {
		return DataType::FLOAT;
	}

	return DataType::INT;
}

bool SemanticAnalyzer::isNumeric(DataType type) const {
	return type == DataType::INT || type == DataType::FLOAT;
}

void SemanticAnalyzer::reportError(const string& message) {
	errors_.push_back(message);
}

