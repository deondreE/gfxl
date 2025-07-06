#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <memory> // For std::unique_ptr

// Forward declaration of the Interpreter/Visitor (design pattern for walking AST)
class Interpreter;

// Base class for all AST nodes.
// We'll use a simple visitor pattern here to allow the interpreter to process nodes.
class Stmt {
public:
    virtual ~Stmt() = default; // Virtual destructor for proper cleanup
    virtual void accept(Interpreter& visitor) const = 0; // Pure virtual accept method
};

// Represents a 'print' statement in our language.
class PrintStmt : public Stmt {
public:
    const std::string value; // The string literal to be printed

    PrintStmt(const std::string& val) : value(val) {}

    // Implement the accept method for PrintStmt
    void accept(Interpreter& visitor) const override;
};

#endif // AST_HPP