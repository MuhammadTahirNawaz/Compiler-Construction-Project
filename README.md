# Mini-Compiler (C++17)

A mini-compiler for a restricted C-like language, built for CSC303L Compiler Construction. The project integrates lexical analysis, syntax analysis, semantic analysis, and symbol table management.

## Build
Use a C++17 compiler. Example with g++:

```
g++ -std=c++17 -Iinclude src/main.cpp src/Lexer.cpp src/Parser.cpp src/SemanticAnalyzer.cpp src/ASTPrinter.cpp -o mini-compiler
```

## Run
```
./mini-compiler tests/valid/basic_assign.c
```

## Project Layout
- include/: Shared interfaces (Token, AST, SymbolTable)
- src/: Compiler phases (lexer, parser, semantic analyzer)
- tests/: Valid and invalid test programs

## Security Notes
- Do not commit secrets or private data.
- Keep build outputs and local configs out of git; review .gitignore before committing.

## Progress Log
- 2026-06-03: Added double-buffered, multi-threaded lexer with regex-based tokenization and token stream output in main.
- 2026-06-03: Added LL(1) recursive-descent parser skeleton with AST node types and expression precedence parsing.
- 2026-06-03: Added semantic analyzer with scoped symbol tracking and basic type checking, wired into main pipeline.
- 2026-06-03: Added AST block/while nodes, parser support for blocks and while, and nested scope handling in semantic analysis.
- 2026-06-03: Added AST printer visitor for visual debugging and wired it into the pipeline.
- 2026-06-03: Added valid and invalid test programs for syntax, type mismatch, and scope errors.
- 2026-06-03: Added if/else AST nodes, parser support, printer output, and semantic checks.
- 2026-06-03: Added if/else test cases, including a semantic error for non-boolean conditions.
- 2026-06-03: Added basic_assign test, updated build command, and added logical operator precedence in parser.
- 2026-06-03: Updated main output banners to ASCII-safe phase labels.
- 2026-06-03: Removed emoji markers from error banners for terminal compatibility.

