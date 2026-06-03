#include <sstream>

#include "AST.hpp"
#include "Parser.hpp"
#include "Token.hpp"

using namespace std;

Parser::Parser(const vector<Token>& tokens) : tokens_(tokens), current_(0) {}

unique_ptr<ProgramNode> Parser::parseProgram() {
	vector<unique_ptr<StmtNode>> statements;
	while (!isAtEnd()) {
		auto stmt = parseStatement();
		if (stmt) {
			statements.push_back(move(stmt));
		} else {
			synchronize();
		}
	}
	return make_unique<ProgramNode>(move(statements));
}

const vector<string>& Parser::errors() const {
	return errors_;
}

const Token& Parser::currentToken() const {
	return tokens_[current_];
}

bool Parser::isAtEnd() const {
	return currentToken().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
	if (isAtEnd()) {
		return false;
	}
	return currentToken().type == type;
}

bool Parser::checkLexeme(const string& lexeme) const {
	if (isAtEnd()) {
		return false;
	}
	return currentToken().lexeme == lexeme;
}

const Token& Parser::advance() {
	if (!isAtEnd()) {
		current_++;
	}
	return tokens_[current_ - 1];
}

bool Parser::match(TokenType type) {
	if (check(type)) {
		advance();
		return true;
	}
	return false;
}

bool Parser::matchLexeme(const string& lexeme) {
	if (checkLexeme(lexeme)) {
		advance();
		return true;
	}
	return false;
}

const Token& Parser::consume(TokenType type, const string& message) {
	if (check(type)) {
		return advance();
	}
	reportError(currentToken(), message);
	return currentToken();
}

const Token& Parser::consumeLexeme(const string& lexeme, const string& message) {
	if (checkLexeme(lexeme)) {
		return advance();
	}
	reportError(currentToken(), message);
	return currentToken();
}

unique_ptr<StmtNode> Parser::parseStatement() {
	if (check(TokenType::KEYWORD)) {
		const string& kw = currentToken().lexeme;
		if (kw == "if") {
			return parseIfStatement();
		}
		if (kw == "while") {
			return parseWhileStatement();
		}
		if (kw == "for") {
			return parseForStatement();
		}
		if (kw == "input") {
			return parseInputStatement();
		}
		if (kw == "output") {
			return parseOutputStatement();
		}
		if (kw == "return") {
			return parseReturnStatement();
		}
		if (kw == "int" || kw == "float" || kw == "bool" || kw == "void") {
			if (current_ + 2 < tokens_.size() && tokens_[current_ + 2].lexeme == "(") {
				return parseFunctionDecl();
			} else if (kw != "void") {
				return parseVarDecl();
			} else {
				reportError(currentToken(), "Variables cannot have void type.");
				advance();
				return nullptr;
			}
		}
	}

	if (checkLexeme("{")) {
		return parseBlock();
	}

	if (check(TokenType::IDENTIFIER)) {
		return parseAssignment();
	}

	return parseExprStmt();
}

unique_ptr<StmtNode> Parser::parseVarDecl() {
	const Token& typeToken = advance();
	DataType declType = dataTypeFromKeyword(typeToken.lexeme);
	const Token& name = consume(TokenType::IDENTIFIER, "Expected identifier after type.");
	unique_ptr<ExprNode> initializer;

	if (matchLexeme("=")) {
		initializer = parseExpression();
	}

	consumeLexeme(";", "Expected ';' after declaration.");
	return make_unique<VarDeclNode>(declType, name.lexeme, move(initializer));
}

unique_ptr<StmtNode> Parser::parseAssignment() {
	const Token& name = advance();
	if (!matchLexeme("=")) {
		reportError(currentToken(), "Expected '=' after identifier.");
		return nullptr;
	}

	auto value = parseExpression();
	consumeLexeme(";", "Expected ';' after assignment.");
	return make_unique<AssignNode>(name.lexeme, move(value));
}

unique_ptr<StmtNode> Parser::parseExprStmt() {
	auto expr = parseExpression();
	consumeLexeme(";", "Expected ';' after expression.");
	return make_unique<ExprStmtNode>(move(expr));
}

unique_ptr<StmtNode> Parser::parseBlock() {
	consumeLexeme("{", "Expected '{' to start a block.");
	vector<unique_ptr<StmtNode>> statements;

	while (!isAtEnd() && !checkLexeme("}")) {
		auto stmt = parseStatement();
		if (stmt) {
			statements.push_back(move(stmt));
		} else {
			synchronize();
		}
	}

	consumeLexeme("}", "Expected '}' after block.");
	return make_unique<BlockNode>(move(statements));
}

unique_ptr<StmtNode> Parser::parseWhileStatement() {
	consumeLexeme("while", "Expected 'while'.");
	consumeLexeme("(", "Expected '(' after 'while'.");
	auto condition = parseExpression();
	consumeLexeme(")", "Expected ')' after condition.");
	auto body = parseStatement();
	return make_unique<WhileStmtNode>(move(condition), move(body));
}

unique_ptr<StmtNode> Parser::parseIfStatement() {
	consumeLexeme("if", "Expected 'if'.");
	consumeLexeme("(", "Expected '(' after 'if'.");
	auto condition = parseExpression();
	consumeLexeme(")", "Expected ')' after condition.");
	auto thenBranch = parseStatement();
	unique_ptr<StmtNode> elseBranch;
	if (check(TokenType::KEYWORD) && currentToken().lexeme == "else") {
		advance();
		elseBranch = parseStatement();
	}
	return make_unique<IfStmtNode>(move(condition), move(thenBranch), move(elseBranch));
}

unique_ptr<StmtNode> Parser::parseInputStatement() {
	consumeLexeme("input", "Expected 'input'.");
	const Token& name = consume(TokenType::IDENTIFIER, "Expected identifier after input.");
	consumeLexeme(";", "Expected ';' after input statement.");
	return make_unique<InputStmtNode>(name.lexeme);
}

unique_ptr<StmtNode> Parser::parseOutputStatement() {
	consumeLexeme("output", "Expected 'output'.");
	auto expr = parseExpression();
	consumeLexeme(";", "Expected ';' after output statement.");
	return make_unique<OutputStmtNode>(move(expr));
}

unique_ptr<StmtNode> Parser::parseForStatement() {
	consumeLexeme("for", "Expected 'for'.");
	consumeLexeme("(", "Expected '(' after 'for'.");

	// 1. Initializer
	unique_ptr<StmtNode> initializer;
	if (check(TokenType::KEYWORD) && (currentToken().lexeme == "int" || currentToken().lexeme == "float" || currentToken().lexeme == "bool")) {
		initializer = parseVarDecl();
	} else if (!checkLexeme(";")) {
		initializer = parseAssignment();
	} else {
		consumeLexeme(";", "Expected ';'.");
	}

	// 2. Condition
	unique_ptr<ExprNode> condition;
	if (!checkLexeme(";")) {
		condition = parseExpression();
	}
	consumeLexeme(";", "Expected ';' after condition.");

	// 3. Post-cycle expression / assignment (without trailing ;)
	unique_ptr<StmtNode> post;
	if (!checkLexeme(")")) {
		if (check(TokenType::IDENTIFIER)) {
			const Token& name = advance();
			consumeLexeme("=", "Expected '=' in post-cycle assignment.");
			auto value = parseExpression();
			post = make_unique<AssignNode>(name.lexeme, move(value));
		} else {
			auto expr = parseExpression();
			post = make_unique<ExprStmtNode>(move(expr));
		}
	}
	consumeLexeme(")", "Expected ')' after post-cycle statement.");

	auto body = parseStatement();
	return make_unique<ForStmtNode>(move(initializer), move(condition), move(post), move(body));
}

unique_ptr<StmtNode> Parser::parseFunctionDecl() {
	const Token& typeToken = advance();
	DataType retType = dataTypeFromKeyword(typeToken.lexeme);
	const Token& name = consume(TokenType::IDENTIFIER, "Expected function name.");
	consumeLexeme("(", "Expected '(' after function name.");

	vector<pair<DataType, string>> params;
	if (!checkLexeme(")")) {
		do {
			if (!check(TokenType::KEYWORD)) {
				reportError(currentToken(), "Expected parameter type.");
				break;
			}
			const Token& pTypeTok = advance();
			DataType pType = dataTypeFromKeyword(pTypeTok.lexeme);
			if (pType == DataType::VOID || pType == DataType::UNKNOWN) {
				reportError(pTypeTok, "Invalid parameter type.");
			}
			const Token& pName = consume(TokenType::IDENTIFIER, "Expected parameter name.");
			params.push_back({pType, pName.lexeme});
		} while (matchLexeme(","));
	}
	consumeLexeme(")", "Expected ')' after parameter list.");

	auto body = parseBlock();
	return make_unique<FunctionDeclNode>(retType, name.lexeme, move(params), move(body));
}

unique_ptr<StmtNode> Parser::parseReturnStatement() {
	consumeLexeme("return", "Expected 'return'.");
	unique_ptr<ExprNode> expr;
	if (!checkLexeme(";")) {
		expr = parseExpression();
	}
	consumeLexeme(";", "Expected ';' after return value.");
	return make_unique<ReturnStmtNode>(move(expr));
}

unique_ptr<ExprNode> Parser::parseExpression() {
	return parseLogicalOr();
}

unique_ptr<ExprNode> Parser::parseLogicalOr() {
	auto expr = parseLogicalAnd();
	while (matchLexeme("||")) {
		string op = tokens_[current_ - 1].lexeme;
		auto right = parseLogicalAnd();
		expr = make_unique<BinaryExprNode>(op, move(expr), move(right));
	}
	return expr;
}

unique_ptr<ExprNode> Parser::parseLogicalAnd() {
	auto expr = parseComparison();
	while (matchLexeme("&&")) {
		string op = tokens_[current_ - 1].lexeme;
		auto right = parseComparison();
		expr = make_unique<BinaryExprNode>(op, move(expr), move(right));
	}
	return expr;
}

unique_ptr<ExprNode> Parser::parseComparison() {
	auto expr = parseAdditive();
	while (matchLexeme("<") || matchLexeme(">") || matchLexeme("<=") || matchLexeme(">=") ||
			matchLexeme("==") || matchLexeme("!=")) {
		string op = tokens_[current_ - 1].lexeme;
		auto right = parseAdditive();
		expr = make_unique<BinaryExprNode>(op, move(expr), move(right));
	}
	return expr;
}

unique_ptr<ExprNode> Parser::parseAdditive() {
	auto expr = parseMultiplicative();
	while (matchLexeme("+") || matchLexeme("-")) {
		string op = tokens_[current_ - 1].lexeme;
		auto right = parseMultiplicative();
		expr = make_unique<BinaryExprNode>(op, move(expr), move(right));
	}
	return expr;
}

unique_ptr<ExprNode> Parser::parseMultiplicative() {
	auto expr = parseUnary();
	while (matchLexeme("*") || matchLexeme("/")) {
		string op = tokens_[current_ - 1].lexeme;
		auto right = parseUnary();
		expr = make_unique<BinaryExprNode>(op, move(expr), move(right));
	}
	return expr;
}

unique_ptr<ExprNode> Parser::parseUnary() {
	if (matchLexeme("-") || matchLexeme("!")) {
		string op = tokens_[current_ - 1].lexeme;
		auto right = parseUnary();
		return make_unique<UnaryExprNode>(op, move(right));
	}
	return parsePrimary();
}

unique_ptr<ExprNode> Parser::parsePrimary() {
	if (match(TokenType::INT_LITERAL) || match(TokenType::FLOAT_LITERAL) || match(TokenType::BOOLEAN_LITERAL)) {
		return make_unique<LiteralNode>(tokens_[current_ - 1].lexeme);
	}

	if (match(TokenType::IDENTIFIER)) {
		string name = tokens_[current_ - 1].lexeme;
		if (matchLexeme("(")) {
			vector<unique_ptr<ExprNode>> args;
			if (!checkLexeme(")")) {
				do {
					args.push_back(parseExpression());
				} while (matchLexeme(","));
			}
			consumeLexeme(")", "Expected ')' after function arguments.");
			return make_unique<FunctionCallNode>(name, move(args));
		}
		return make_unique<IdentifierNode>(name);
	}

	if (matchLexeme("(")) {
		auto expr = parseExpression();
		consumeLexeme(")", "Expected ')' after expression.");
		return expr;
	}

	reportError(currentToken(), "Unexpected token in expression.");
	return make_unique<LiteralNode>("0");
}

DataType Parser::dataTypeFromKeyword(const string& lexeme) const {
	if (lexeme == "int") {
		return DataType::INT;
	}
	if (lexeme == "float") {
		return DataType::FLOAT;
	}
	if (lexeme == "bool") {
		return DataType::BOOL;
	}
	if (lexeme == "void") {
		return DataType::VOID;
	}
	return DataType::UNKNOWN;
}

void Parser::reportError(const Token& token, const string& message) {
	ostringstream oss;
	oss << "[" << token.line << ":" << token.column << "] " << message;
	errors_.push_back(oss.str());
}

void Parser::synchronize() {
	advance();
	while (!isAtEnd()) {
		if (tokens_[current_ - 1].lexeme == ";") {
			return;
		}

		if (check(TokenType::KEYWORD)) {
			return;
		}
		advance();
	}
}
