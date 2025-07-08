#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "Token.h"
#include "ast.h"

struct Symbol {
	int stackOffset;
};

class CodeGenerator
{
public:
	CodeGenerator();

	std::string generate(const Program* program_ast);
	std::vector<std::string> getErrors() const;

private:
    std::vector<std::string> errors_;
    std::string assembly_code_;
    std::map<std::string, Symbol> symbolTable_; // Stores variable names and their stack locations
    int stackOffsetCounter_; // Tracks the next available stack slot for new variables

    // Helper to add assembly instructions
    void emit(const std::string& instruction);
    void error(const std::string& msg);

    // --- AST Node Visitors (Recursive functions to generate code for specific nodes) ---
    void visit(const Program* node);
    void visit(const Statement* node); // Virtual dispatch for statements
    void visit(const AssignmentStatement* node);
    void visit(const ExpressionStatement* node);

    void visit(const Expression* node); // Virtual dispatch for expressions
    void visit(const IntegerLiteral* node);
    void visit(const IdentifierExpr* node);
    void visit(const BinaryExpression* node);

    // --- Symbol Table Management ---
    void defineVariable(const std::string& name);
    Symbol* getSymbol(const std::string& name);

    // --- Assembly Utilities ---
    void pop_into_rax();
    void push_from_rax();
};

