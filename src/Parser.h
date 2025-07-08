#pragma once

#include <vector>
#include <string>
#include <memory> 
#include <map>

#include "lexer.h"
#include "token.h"
#include "ast.h"

// Define operator precedences (higher value means higher precedence)
enum Precedence {
    LOWEST = 1,
    SUM = 2, // + -
    PRODUCT = 3, // * /
    ASSIGN_PRECEDENCE = 4 // For handling assignment operator
};

// Map to store operator precedences
extern const std::map<TokenType, Precedence> precedences;

class Parser {
public:
    Parser(Lexer& l); // Takes a Lexer reference

    std::unique_ptr<Program> parseProgram();
    std::vector<std::string> getErrors() const;

private:
    Lexer& lexer_;
    Token currentToken_;
    Token peekToken_; // Lookahead token
    std::vector<std::string> errors_;

    // --- Utility Methods for Token Stream ---
    void nextToken(); // Advances currentToken and fills peekToken
    bool currentTokenIs(TokenType type) const;
    bool peekTokenIs(TokenType type) const;
    bool expectPeek(TokenType type); // Checks peekToken, advances, and logs error if mismatch

    // --- Error Handling ---
    void peekError(TokenType type);

    // --- Main Parsing Methods ---
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<AssignmentStatement> parseAssignmentStatement();
    std::unique_ptr<ExpressionStatement> parseExpressionStatement();

    // --- Expression Parsing (using Operator Precedence Climbing / Pratt Parsing) ---
    std::unique_ptr<Expression> parseExpression(Precedence prec);
    std::unique_ptr<Expression> parseIntegerLiteral();
    std::unique_ptr<Expression> parseIdentifier();
    std::unique_ptr<Expression> parseGroupedExpression();
    std::unique_ptr<PrintStatement> parsePrintStatement();
    std::unique_ptr<Expression> parsePrefixExpression(); // Handles INT, IDENTIFIER, LPAREN prefix
    std::unique_ptr<Expression> parseInfixExpression(std::unique_ptr<Expression> left_expr); // Handles binary ops

    Precedence peekPrecedence() const;
    Precedence currentPrecedence() const;

    using PrefixParseFn = std::unique_ptr<Expression>(Parser::*)();
    using InfixParseFn = std::unique_ptr<Expression>(Parser::*)(std::unique_ptr<Expression>);

    std::map<TokenType, PrefixParseFn> prefixParseFns;
    std::map<TokenType, InfixParseFn> infixParseFns;

    void registerPrefix(TokenType token_type, PrefixParseFn fn);
    void registerInfix(TokenType token_type, InfixParseFn fn);

    void setupParseFunctions(); 
};