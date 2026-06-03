# Mini-Compiler (C++)

A lightweight, hand-written compiler for a C-like language, built using C++17.

- **Lexer**: Custom double-buffered, multi-threaded regex token scanner.
- **Parser**: LL(1) recursive-descent parser producing a structured AST.
- **Semantic Analyzer**: Type verification, scope handling, and symbol table.
- **AST Printer**: Custom visitor pattern visualization for code structures.
- **Target**: Restricted C syntax featuring conditionals, loops, and functions.
- **Usage**: Run `./mini-compiler <source_file.c>` to compile and inspect.
