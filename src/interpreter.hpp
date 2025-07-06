#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <memory>
#include <iostream>
#include "ast.hpp" // Include our AST node definitions

// Forward declaration of AST nodes for the accept method
class PrintStmt;

class Interpreter {
public:
    // Interprets the given AST statement.
    void interpret(const Stmt& statement);

    // Visitor method for PrintStmt.
    void visitPrintStmt(const PrintStmt& stmt);

    // We need to implement the accept method defined in ast.hpp
    // This allows PrintStmt to call the appropriate visitor method.
    // We define this here to avoid circular includes if ast.hpp needed Interpreter definition.
};

// Implementation of the accept method for PrintStmt from ast.hpp
// This is typically done in the .cpp file of the AST node, but since Interpreter
// needs to be fully defined for the accept method to call its members,
// it's sometimes put after the Interpreter class definition.
// For simplicity and to keep the structure clear for this small example,
// we'll put it here.
inline void PrintStmt::accept(Interpreter& visitor) const {
    visitor.visitPrintStmt(*this);
}

#endif // INTERPRETER_HPP