#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>

#include "Token.h"
#include "ast.h"

struct CodegenSymbol {
	int stackOffset;
    TokenType type;
};

enum TargetPlatform {
    PLATFORM_UNKNOWN,
    PLATFORM_LINUX,
    PLATFORM_WINDOWS_MINGW,
    PLATFORM_MACOS,
};

class CodeGenerator
{
public:
	CodeGenerator();

	std::string generate(const Program* program_ast);
	std::vector<std::string> getErrors() const;

private:
    mutable std::vector<std::string> errors_;
    std::stringstream ss;
    std::map<std::string, CodegenSymbol> symbolTable_; // Stores variable names and their stack locations
    int stackOffsetCounter_; // Tracks the next available stack slot for new variables
    TargetPlatform targetPlatform_;

    void error(const std::string& msg);

    // Helper to add assembly instructions
    void emit(const std::string& instruction);
    void emitComment(const std::string& comment);

    // --- Platform-Specific Assembly Boilerplate ---
    void emitMainPrologue();
    void emitMainEpilogue();
    void emitPrintInteger(const std::string& reg); // Pass register holding integer (e.g., "rax")
    void emitPrintBoolean(const std::string& reg); // Pass register holding 0/1 boolean (e.g., "al")

    void visitProgram(const Program* node);

    // --- AST Node Visitors (Recursive functions to generate code for specific nodes) ---
    void visitStatement(const Statement* node); // Dispatcher for generic Statement*
    void visitAssignmentStatement(const AssignmentStatement* node);
    void visitExpressionStatement(const ExpressionStatement* node);
    void visitPrintStatement(const PrintStatement* node);

    void visitExpression(const Expression* node); // Dispatcher for generic Expression*
    void visitIntegerLiteral(const IntegerLiteral* node);
    void visitBooleanLiteral(const BooleanLiteral* node);
    void visitIdentifierExpr(const IdentifierExpr* node);
    void visitBinaryExpression(const BinaryExpression* node);


    void defineVariable(const std::string& name, TokenType type);
    CodegenSymbol* getSymbol(const std::string& name);

    std::string getRegSize(TokenType type) const; // Added const
    std::string getRegName(TokenType type, const std::string& baseReg) const;

    void visit(BooleanLiteral& node);

    // --- Assembly Utilities ---
    void push_value_from_rax(TokenType type);
    void pop_value_into_rax(TokenType type);
};

