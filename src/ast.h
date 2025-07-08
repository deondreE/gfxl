#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Token.h"

// Forward declarations for mutually recursive types (e.g., Program needs Statement)
class Program;
class Statement;
class Expression;
class IntegerLiteral;
class IdentifierExpr;
class BinaryExpression;
class ExpressionStatement;
class AssignmentStatement;

class ASTNode {
public:
	virtual ~ASTNode() = default;
};

class Expression : public ASTNode {};

class IntegerLiteral : public Expression {
public:
    int value;
    IntegerLiteral(int val) : value(val) {}
};

class IdentifierExpr : public Expression {
public:
    std::string name;
    IdentifierExpr(const std::string& n) : name(n) {}
};

class BinaryExpression : public Expression {
public:
    std::unique_ptr<Expression> left;
    TokenType op; // Token type for the operator (PLUS, MINUS, etc.)
    std::unique_ptr<Expression> right;

    BinaryExpression(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(o), right(std::move(r)) {
    }
};

class Statement : public ASTNode {};

class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;
    ExpressionStatement(std::unique_ptr<Expression> expr) : expression(std::move(expr)) {}
};

class AssignmentStatement : public Statement {
public:
    std::unique_ptr<IdentifierExpr> identifier; // The variable being assigned to
    std::unique_ptr<Expression> value;          // The expression whose result is assigned

    AssignmentStatement(std::unique_ptr<IdentifierExpr> id, std::unique_ptr<Expression> val)
        : identifier(std::move(id)), value(std::move(val)) {
    }
};

class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Statement>> statements;

    void AddStatement(std::unique_ptr<Statement> stmt) {
        statements.push_back(std::move(stmt));
    }
};