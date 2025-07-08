#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Token.h"

// ─────────────────── Base node ───────────────────
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

// ─────────────────── Expressions ─────────────────
class Expression : public ASTNode {};

// Integer literal  e.g.  42
class IntegerLiteral : public Expression {
public:
    explicit IntegerLiteral(int val) : value(val) {}
    int value;
};

// Identifier expression  e.g.  foo
class IdentifierExpr : public Expression {
public:
    explicit IdentifierExpr(std::string n) : name(std::move(n)) {}
    std::string name;
};

// Binary expression  e.g.  a + b
class BinaryExpression : public Expression {
public:
    BinaryExpression(std::unique_ptr<Expression> l,
        TokenType                    o,
        std::unique_ptr<Expression> r)
        : left(std::move(l)), op(o), right(std::move(r)) {
    }

    std::unique_ptr<Expression> left;
    TokenType                   op;
    std::unique_ptr<Expression> right;
};

// ─────────────────── Statements ──────────────────
class Statement : public ASTNode {};

// Bare expression used as a statement  e.g.  a + 1;
class ExpressionStatement : public Statement {
public:
    explicit ExpressionStatement(std::unique_ptr<Expression> expr)
        : expression(std::move(expr)) {
    }
    std::unique_ptr<Expression> expression;
};

// Assignment  e.g.  x = 5;
class AssignmentStatement : public Statement {
public:
    AssignmentStatement(std::unique_ptr<IdentifierExpr> id,
        std::unique_ptr<Expression>    val)
        : identifier(std::move(id)), value(std::move(val)) {
    }
    std::unique_ptr<IdentifierExpr> identifier;
    std::unique_ptr<Expression>     value;
};

// print <expr>;
class PrintStatement : public Statement {
public:
    explicit PrintStatement(std::unique_ptr<Expression> expr)
        : expression(std::move(expr)) {
    }
    std::unique_ptr<Expression> expression;
};

// ─────────────────── Program root ────────────────
class Program : public ASTNode {
public:
    void AddStatement(std::unique_ptr<Statement> stmt) {
        statements.emplace_back(std::move(stmt));
    }
    std::vector<std::unique_ptr<Statement>> statements;
};
