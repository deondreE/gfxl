#include "interpreter.hpp"

// The main entry point for interpretation.
// Uses dynamic_cast to determine the concrete type of the statement
// and calls the appropriate visitor method.
void Interpreter::interpret(const Stmt& statement) {
    // This is the "dispatch" part of the visitor pattern.
    // The AST node calls back into the interpreter with its specific type.
    statement.accept(*this);
}

// Implementation for visiting a PrintStmt.
void Interpreter::visitPrintStmt(const PrintStmt& stmt) {
    std::cout << stmt.value << std::endl;
}