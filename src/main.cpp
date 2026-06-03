#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "ASTPrinter.hpp"
#include "IRGenerator.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include "Token.hpp"

using namespace std;

namespace {
const string kHeaderLine = "========================================";
const string kDividerLine = "----------------------------------------";
const string kSuccessLine = "****************************************";

string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD:
            return "KEYWORD";
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::INT_LITERAL:
            return "INT_LITERAL";
        case TokenType::FLOAT_LITERAL:
            return "FLOAT_LITERAL";
        case TokenType::BOOLEAN_LITERAL:
            return "BOOLEAN_LITERAL";
        case TokenType::OPERATOR:
            return "OPERATOR";
        case TokenType::SEPARATOR:
            return "SEPARATOR";
        case TokenType::END_OF_FILE:
            return "END_OF_FILE";
        case TokenType::ERROR:
            return "ERROR";
    }
    return "UNKNOWN";
}
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: mini-compiler <source-file>\n";
        return 1;
    }

    Lexer lexer(argv[1]);
    vector<Token> tokens = lexer.tokenize();

    cout << kHeaderLine << "\n";
    cout << "[>>> MINI COMPILER REPORT <<<]\n";
    cout << kHeaderLine << "\n\n";

    cout << "[PHASE 1] >> LEXICAL ANALYSIS\n";
    cout << kDividerLine << "\n";

    const int kTypeWidth = 18;
    const int kLexemeWidth = 24;
    const int kLocationWidth = 12;

    cout << "+" << string(kTypeWidth + 2, '-')
         << "+" << string(kLexemeWidth + 2, '-')
         << "+" << string(kLocationWidth + 2, '-') << "+\n";
    cout << "| " << left << setw(kTypeWidth) << "TOKEN TYPE"
         << " | " << left << setw(kLexemeWidth) << "LEXEME"
         << " | " << left << setw(kLocationWidth) << "LOCATION" << " |\n";
    cout << "+" << string(kTypeWidth + 2, '-')
         << "+" << string(kLexemeWidth + 2, '-')
         << "+" << string(kLocationWidth + 2, '-') << "+\n";

    for (const auto& token : tokens) {
        string lexeme = token.lexeme.empty() ? "-" : token.lexeme;
        string location = to_string(token.line) + ":" + to_string(token.column);
        cout << "| " << left << setw(kTypeWidth) << tokenTypeToString(token.type)
             << " | " << left << setw(kLexemeWidth) << lexeme
             << " | " << left << setw(kLocationWidth) << location << " |\n";
    }

    cout << "+" << string(kTypeWidth + 2, '-')
         << "+" << string(kLexemeWidth + 2, '-')
         << "+" << string(kLocationWidth + 2, '-') << "+\n\n";

    Parser parser(tokens);
    unique_ptr<ProgramNode> astRoot = parser.parseProgram();
    if (!parser.errors().empty()) {
        cout << ">> [PHASE 2] SYNTAX ANALYSIS FAILED\n";
        cout << kDividerLine << "\n";
        for (const auto& err : parser.errors()) {
            cout << "- " << err << "\n";
        }
        return 1;
    }

    cout << "[PHASE 2 & 3] >> SYNTAX & SEMANTIC ANALYSIS\n";
    cout << kDividerLine << "\n";
    ASTPrinter printer;
    astRoot->accept(&printer);
    cout << kDividerLine << "\n";

    SemanticAnalyzer analyzer;
    astRoot->accept(&analyzer);
    if (!analyzer.errors().empty()) {
        cout << ">> [PHASE 3] SEMANTIC ANALYSIS FAILED\n";
        cout << kDividerLine << "\n";
        for (const auto& err : analyzer.errors()) {
            cout << "- " << err << "\n";
        }
        return 1;
    }

    cout << "[PHASE 4] >> INTERMEDIATE CODE GENERATION (TAC)\n";
    cout << kDividerLine << "\n";
    IRGenerator irGen;
    astRoot->accept(&irGen);
    irGen.print();
    cout << kDividerLine << "\n\n";

    cout << "Lexer -> Parser -> Semantic Analyzer -> IR Generator pipeline executed successfully!" << "\n\n";
    cout << kSuccessLine << "\n";
    cout << "SUCCESS: Pipeline executed cleanly!\n";
    cout << kSuccessLine << "\n";
    return 0;
}
