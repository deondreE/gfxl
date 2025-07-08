#include "parser.h"
#include <iostream> // For debugging or simple error output

// --- Global Precedence Map ---
const std::map<TokenType, Precedence> precedences = {
    {ASSIGN,    ASSIGN_PRECEDENCE}, // Lowest for math, but higher than general lowest
    {PLUS,      SUM},
    {MINUS,     SUM},
    {ASTERISK,  PRODUCT},
    {SLASH,     PRODUCT},
    {LPAREN,    LOWEST} // Parentheses don't have a precedence in this context, they're handled by parsePrefix
};

Parser::Parser(Lexer& l) : lexer_(l) {
    nextToken();
    nextToken();

    setupParseFunctions();
}

std::vector<std::string> Parser::getErrors() const {
    return errors_;
}

void Parser::peekError(TokenType type) {
    std::string msg = "Expected next token to be " + tokenTypeStrings.at(type) +
        ", got " + tokenTypeStrings.at(peekToken_.type) +
        " instead. (Literal: '" + peekToken_.literal + "')";
    errors_.push_back(msg);
}

void Parser::nextToken() {
    currentToken_ = peekToken_;
    peekToken_ = lexer_.nextToken();
}

bool Parser::currentTokenIs(TokenType type) const {
    return currentToken_.type == type;
}

bool Parser::peekTokenIs(TokenType type) const {
    return peekToken_.type == type;
}

bool Parser::expectPeek(TokenType type) {
    if (peekTokenIs(type)) {
        nextToken();
        return true;
    }
    else {
        peekError(type);
        return false;
    }
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    while (currentToken_.type != END_OF_FILE) {
        std::unique_ptr<Statement> stmt = parseStatement();
        if (stmt) {
            program->AddStatement(std::move(stmt));
        }
        nextToken();
    }
    return program;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    if (currentTokenIs(IDENTIFIER) && peekTokenIs(ASSIGN)) {
        return parseAssignmentStatement();
    }
    else {
        return parseExpressionStatement();
    }
}

std::unique_ptr<AssignmentStatement> Parser::parseAssignmentStatement() {
    auto identifier_expr = std::make_unique<IdentifierExpr>(currentToken_.literal);

    if (!expectPeek(ASSIGN)) { // Expect '='
        return nullptr;
    }
    nextToken();

    std::unique_ptr<Expression> value_expr = parseExpression(LOWEST);
    if (!value_expr) {
        return nullptr;
    }

    if (peekTokenIs(SEMICOLON)) {
        nextToken();
    }

    return std::make_unique<AssignmentStatement>(std::move(identifier_expr), std::move(value_expr));
}


std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
    auto expr = parseExpression(LOWEST);
    if (!expr) {
        return nullptr;
    }

    if (peekTokenIs(SEMICOLON)) {
        nextToken();
    }

    return std::make_unique<ExpressionStatement>(std::move(expr));
}

Precedence Parser::peekPrecedence() const {
    auto it = precedences.find(peekToken_.type);
    if (it != precedences.end()) {
        return it->second;
    }
    return LOWEST;
}

Precedence Parser::currentPrecedence() const {
    auto it = precedences.find(currentToken_.type);
    if (it != precedences.end()) {
        return it->second;
    }
    return LOWEST;
}

std::unique_ptr<Expression> Parser::parseExpression(Precedence prec) {
    PrefixParseFn prefix_fn = nullptr;
    auto prefix_it = prefixParseFns.find(currentToken_.type);
    if (prefix_it != prefixParseFns.end()) {
        prefix_fn = prefix_it->second;
    }

    if (prefix_fn == nullptr) {
        errors_.push_back("No prefix parse function for " + tokenTypeStrings.at(currentToken_.type) +
            " (" + currentToken_.literal + ") found.");
        return nullptr;
    }

    std::unique_ptr<Expression> left_expr = (this->*prefix_fn)(); // Call prefix parser
    if (!left_expr) return nullptr;

    while (peekTokenIs(SEMICOLON) == false && prec < peekPrecedence()) {
        InfixParseFn infix_fn = nullptr;
        auto infix_it = infixParseFns.find(peekToken_.type);
        if (infix_it != infixParseFns.end()) {
            infix_fn = infix_it->second;
        }

        if (infix_fn == nullptr) {
            return left_expr; // No infix function found, stop
        }

        nextToken(); // Consume the infix operator token

        // Call the infix parser with the left_expr
        left_expr = (this->*infix_fn)(std::move(left_expr));
        if (!left_expr) return nullptr;
    }

    return left_expr;
}


std::unique_ptr<Expression> Parser::parseIntegerLiteral() {
    try {
        int val = std::stoi(currentToken_.literal);
        return std::make_unique<IntegerLiteral>(val);
    }
    catch (const std::out_of_range& oor) {
        errors_.push_back("Integer literal " + currentToken_.literal + " out of range.");
        return nullptr;
    }
    catch (const std::invalid_argument& ia) {
        errors_.push_back("Could not parse " + currentToken_.literal + " as integer.");
        return nullptr;
    }
}

std::unique_ptr<Expression> Parser::parseIdentifier() {
    return std::make_unique<IdentifierExpr>(currentToken_.literal);
}

std::unique_ptr<Expression> Parser::parseGroupedExpression() {
    nextToken(); // Consume the '('

    std::unique_ptr<Expression> expr = parseExpression(LOWEST);
    if (!expr) {
        return nullptr;
    }

    if (!expectPeek(RPAREN)) { 
        return nullptr;
    }
    return expr;
}

// Handles binary operations (e.g., +, -, *, /)
std::unique_ptr<Expression> Parser::parseInfixExpression(std::unique_ptr<Expression> left_expr) {
    TokenType op_type = currentToken_.type;
    Precedence prec = currentPrecedence();

    nextToken();

    std::unique_ptr<Expression> right_expr = parseExpression(prec);
    if (!right_expr) {
        return nullptr;
    }

    return std::make_unique<BinaryExpression>(std::move(left_expr), op_type, std::move(right_expr));
}

void Parser::registerPrefix(TokenType token_type, PrefixParseFn fn) {
    prefixParseFns[token_type] = fn;
}

void Parser::registerInfix(TokenType token_type, InfixParseFn fn) {
    infixParseFns[token_type] = fn;
}

void Parser::setupParseFunctions() {
    // Prefix parsers
    registerPrefix(INT, &Parser::parseIntegerLiteral);
    registerPrefix(IDENTIFIER, &Parser::parseIdentifier);
    registerPrefix(LPAREN, &Parser::parseGroupedExpression);

    registerInfix(PLUS, &Parser::parseInfixExpression);
    registerInfix(MINUS, &Parser::parseInfixExpression);
    registerInfix(ASTERISK, &Parser::parseInfixExpression);
    registerInfix(SLASH, &Parser::parseInfixExpression);
    registerInfix(ASSIGN, &Parser::parseInfixExpression);
}
