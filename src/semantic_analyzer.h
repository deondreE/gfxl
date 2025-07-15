#pragma once

#include "ast.h"
#include "symbol_table.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(Program& node) = 0;
    virtual void visit(ExpressionStatement& node) = 0;
    virtual void visit(AssignmentStatement& node) = 0;
    virtual void visit(PrintStatement& node) = 0;
    virtual void visit(BooleanLiteral& node) = 0;
    virtual void visit(StringLiteral& node) = 0;
    virtual void visit(CharLiteral& node) = 0;
    virtual void visit(IdentifierExpr& node) = 0;
    virtual void visit(IntegerLiteral& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(CommentNode& node) = 0;
};

class SemanticAnalyzer : public ASTVisitor {
public:
    SemanticAnalyzer() : currentScope(std::make_unique<SymbolTable>()) {}

    const std::vector<std::string>& getErrors() const {
        return errors;
    }

    void analyze(Program& program) {
        // We start in the global scope.
        // No explicit enterScope() here, as constructor already set up the initial scope.
        program.accept(*this);
        // No explicit exitScope() here either, as program analysis finishes in global scope.
    }

    void visit(Program& node) override {
        // No new scope for the program itself unless you want to model a 'main' function scope
        for (const auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }

    void visit(AssignmentStatement& node) override {
        node.value->accept(*this);

        TokenType valueType = node.value->resolvedType;
        SymbolEntry* entry = currentScope->resolve(node.identifier->name);
        if (!entry) {
            if (valueType == ILLEGAL) {
                addError("Semantic Error: Attempting to define variable '" + node.identifier->name + "' with an unresolved type.");
                currentScope->define(node.identifier->name, SYM_VAR, ILLEGAL);
                node.identifier->resolvedType = ILLEGAL;
            }
            else {// <--- HERE!
                currentScope->define(node.identifier->name, SYM_VAR, valueType); 
                node.identifier->resolvedType = valueType;
            }
        }
        else {
            node.identifier->resolvedType = entry->declaredTokenType;

            if (node.identifier->resolvedType != valueType) {
                if(valueType == ILLEGAL) {
                    addError("Semantic Warning: Assignment value for '" + node.identifier->name + "' has an unresolved type. Variable type remains " + tokenTypeStrings.at(node.identifier->resolvedType) + ".");
                }
                else {
                    addError("Semantic Error: Type mismatch in assignment to '" + node.identifier->name + "'. Expected " + tokenTypeStrings.at(node.identifier->resolvedType) + ", but got " + tokenTypeStrings.at(valueType) + ".");
                }

                node.identifier->resolvedType = ILLEGAL;
            }
        }
    }

    void visit(ExpressionStatement& node) override {
        node.expression->accept(*this);
    }

    void visit(PrintStatement& node) override {
        node.expression->accept(*this);

        if (node.expression->resolvedType == ILLEGAL) {
            addError("Semantic Error: PRINT statement argument has an unresolved or invalid type.");
        }
    }

    void visit(StringLiteral& node) override {
        node.resolvedType = STRING;
    }
    
    void visit(CharLiteral& node) override {
        node.resolvedType = CHAR;
    }

    // DO NOTHING
    void visit(CommentNode& node) override {}
    
    void visit(BooleanLiteral& node) override {
        node.resolvedType = BOOL;
    }

    void visit(IdentifierExpr& node) override {
        SymbolEntry* entry = currentScope->resolve(node.name);
        if (!entry) {
            addError("Semantic Error: Undefined variable '" + node.name + "'.");
            node.resolvedType = ILLEGAL;
        }
        else {
            node.resolvedType = entry->declaredTokenType;
        }
    }

    void visit(IntegerLiteral& node) override {
        node.resolvedType = INT;
    }

    void visit(BinaryExpression& node) override {
        node.left->accept(*this);
        node.right->accept(*this);

        TokenType leftType = ILLEGAL;
        if (auto* id_expr = dynamic_cast<IdentifierExpr*>(node.left.get())) {
            leftType = id_expr->resolvedType;
        }
        else if (auto* int_lit = dynamic_cast<IntegerLiteral*>(node.left.get())) {
            leftType = INT;
        }
        else if (auto* bin_expr = dynamic_cast<BinaryExpression*>(node.left.get())) {
            leftType = bin_expr->resolvedType;
        }

        TokenType rightType = ILLEGAL;
        if (auto* id_expr = dynamic_cast<IdentifierExpr*>(node.right.get())) {
            rightType = id_expr->resolvedType;
        }
        else if (auto* int_lit = dynamic_cast<IntegerLiteral*>(node.right.get())) {
            rightType = INT;
        }
        else if (auto* bin_expr = dynamic_cast<BinaryExpression*>(node.right.get())) {
            rightType = bin_expr->resolvedType;
        }

        if (leftType == ILLEGAL || rightType == ILLEGAL) {
            node.resolvedType = ILLEGAL;
        }
        else if (leftType != INT || rightType != INT) {
            addError("Semantic Error: Arithmetic operator '" + tokenTypeStrings.at(node.op) + "' expects integer operands.");
            node.resolvedType = ILLEGAL;
        }
        else {
            node.resolvedType = INT;
        }

        if (node.op == SLASH) {
            if (auto* int_lit = dynamic_cast<IntegerLiteral*>(node.right.get())) {
                if (int_lit->value == 0) {
                    addError("Semantic Error: Division by zero detected.");
                    node.resolvedType = ILLEGAL;
                }
            }
        }
    }
private:
    std::unique_ptr<SymbolTable> currentScope;
    std::vector<std::string> errors;

    void addError(const std::string& msg) {
        errors.push_back(msg);
    }

    void enterScope() {
        currentScope = std::make_unique<SymbolTable>(std::move(currentScope));
    }

    void exitScope() {
        if (currentScope->getOuterPtr()) {
            currentScope = currentScope->popOuterScope();
        }
        else {}
    }
};