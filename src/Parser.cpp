#include "Parser.h"
#include <iostream>
#include <stdexcept>
#include <map>
#include <string>
#include <cstdlib>
#include <vector>

// --- Global Precedence Map ---
const std::map<TokenType, Precedence> precedences = {
    {ASSIGN,    ASSIGN_PRECEDENCE},
    {PLUS,      SUM},
    {MINUS,     SUM},
    {ASTERISK,  PRODUCT},
    {SLASH,     PRODUCT}
};

Parser::Parser(Lexer& l) : lexer_(l) {
    // Initialize tokens. Important to get at least two tokens to start lookahead.
    // Ensure lexer is initialized and first token is ready.
    nextToken(); // Sets currentToken_ to the first token.
    nextToken(); // Sets peekToken_ to the second token.

    setupParseFunctions(); // Register parsing functions.
}

std::vector<std::string> Parser::getErrors() const {
    return errors_;
}

constexpr const char* tokenTypeToString(TokenType type) {
    switch (type) {
    case ASSIGN: return "ASSIGN";
    case PLUS: return "PLUS";
    case MINUS: return "MINUS";
    case ASTERISK: return "ASTERISK";
    case SLASH: return "SLASH";
    case INT: return "INT";
    case IDENTIFIER: return "IDENTIFIER";
        // add more cases...
    default: return "UNKNOWN";
    }
}

void Parser::peekError(TokenType type) {
    std::ostringstream msg;
    msg << "Parser error: Expected next token to be " << tokenTypeToString(type)
        << ", got " << tokenTypeToString(peekToken_.type)
        << " instead. (Literal: '" << peekToken_.literal << "')";
    errors_.emplace_back(msg.str());
}
// Helper to check if a token type is a comment.
bool Parser::isCommentToken(TokenType type) const {
    return type == COMMENT_SINGLE_LINE || type == COMMENT_MULTI_LINE;
}

// Advances the token stream. Crucially, if the lexer *returns* comment tokens,
// this `nextToken` *should* skip them so that `currentToken_` and `peekToken_`
// are never comment tokens, UNLESS the AST requires explicit `CommentNode`s.
// Given your `parseTopLevelNode` creates `CommentNode`s, we need `nextToken`
// to advance PAST the token that *was* a comment, so the next iteration of the loop
// can find the next *actual* token.
void Parser::nextToken() {
    // Move the current token to the previously peeked token.
    currentToken_ = peekToken_;
    peekToken_ = lexer_.nextToken();

    while (isCommentToken(peekToken_.type)) {
        peekToken_ = lexer_.nextToken();
    }
}

inline bool Parser::currentTokenIs(TokenType type) const {
    return currentToken_.type == type;
}

inline bool Parser::peekTokenIs(TokenType type) const {
    return peekToken_.type == type;
}

bool Parser::expectPeek(TokenType type) {
    if (peekTokenIs(type)) {
        nextToken(); // Advance if the peeked token matches.
        return true;
    }
    else {
        peekError(type);
        return false;
    }
}

// Parses the entire program.
std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    // Loop until the current token is END_OF_FILE.
    while (currentToken_.type != END_OF_FILE) {
        // Get the next AST node (could be a Statement or a CommentNode).
        std::unique_ptr<ASTNode> node = parseTopLevelNode();

        if (node) {
            // If the node is a Statement, add it to the program's statements.
            if (auto stmt_ptr = dynamic_cast<Statement*>(node.get())) {
                program->AddStatement(std::unique_ptr<Statement>(static_cast<Statement*>(node.release())));
            }
            // If the node is a CommentNode, and you want comments in your AST:
            // You need a way to add it to the Program. Let's assume Program has an AddCommentNode method,
            // or you might store them in a separate list. For now, if not added, they are processed but dropped from the main statement list.
            else if (auto comment_ptr = dynamic_cast<CommentNode*>(node.get())) {
                // If you want comments in the AST, you'd typically add them to a list,
                // perhaps not necessarily as "statements". If they are ignored entirely
                // by the semantic analyzer, then just creating the node and not adding it
                // might be okay, as long as the token stream advances correctly.
                // For this fix, we'll assume they are *not* added to program->statements.
            }
        }
    }
    return program;
}


// Determines the type of the current top-level node.
// If it's a comment token, creates a CommentNode. Otherwise, parses it as a statement.
std::unique_ptr<ASTNode> Parser::parseTopLevelNode() {
    // `currentToken_` holds the token that `nextToken()` from the previous loop iteration provided.
    // Check if this token is a comment.
    if (isCommentToken(currentToken_.type)) {
        // Create a CommentNode AST node from the current token.
        Token commentT = currentToken_; // Copy the token.
        return std::make_unique<CommentNode>(std::move(commentT));
    }
    else {
        // If it's not a comment, try to parse it as a regular statement.
        return parseStatement();
    }
}

// Parses a statement. This function assumes currentToken_ is NOT a comment token.
std::unique_ptr<Statement> Parser::parseStatement() {
    // `currentToken_` is guaranteed not to be a comment token here because
    // `parseProgram` calls `nextToken` which now skips comments,
    // and `parseTopLevelNode` would have returned a CommentNode if it was.

    if (currentTokenIs(PRINT)) {
        return parsePrintStatement();
    }
    else if (currentTokenIs(IDENTIFIER) && peekTokenIs(ASSIGN)) {
        return parseAssignmentStatement();
    }
    else {
        return parseExpressionStatement();
    }
}

std::unique_ptr<Expression> Parser::parseBooleanLiteral() {
    bool val = (currentToken_.type == TRUE);
    auto expr = std::make_unique<BooleanLiteral>(val);
    expr->resolvedType = BOOL;
    nextToken();
    return expr;
}

std::unique_ptr<AssignmentStatement> Parser::parseAssignmentStatement() {
    auto identifier_expr = std::make_unique<IdentifierExpr>(currentToken_.literal);

    if (!expectPeek(ASSIGN)) {
        return nullptr;
    }
    // After expectPeek(ASSIGN), currentToken_ is now the token *after* ASSIGN.
    // This token is guaranteed not to be a comment due to the refined nextToken().
    nextToken(); // Advance past the ASSIGN token correctly.

    std::unique_ptr<Expression> value_expr = parseExpression(LOWEST);
    if (!value_expr) {
        return nullptr;
    }

    // Expect a semicolon. `expectPeek` handles advancing past SEMICOLON and skipping comments.
    if (peekTokenIs(SEMICOLON)) {
        nextToken(); // Consume the semicolon.
    }

    return std::make_unique<AssignmentStatement>(std::move(identifier_expr), std::move(value_expr));
}

std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
    auto expr = parseExpression(LOWEST);
    if (!expr) {
        return nullptr;
    }

    if (peekTokenIs(SEMICOLON)) {
        nextToken(); // Consume the semicolon.
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

    std::unique_ptr<Expression> left_expr = (this->*prefix_fn)();
    if (!left_expr) return nullptr;

    // Loop for infix operators. `peekTokenIs(SEMICOLON) == false` ensures we stop at statement end.
    // `prec < peekPrecedence()` ensures we respect operator precedence.
    while (!peekTokenIs(SEMICOLON) && prec < peekPrecedence()) {
        InfixParseFn infix_fn = nullptr;
        auto infix_it = infixParseFns.find(peekToken_.type);
        if (infix_it != infixParseFns.end()) {
            infix_fn = infix_it->second;
        }

        if (infix_fn == nullptr) {
            // If the next token isn't an infix operator, stop parsing this expression.
            return left_expr;
        }

        // Consume the infix operator token. `nextToken` will skip comments before it.
        nextToken();

        // Call the infix parser. It handles parsing the right-hand side and combining with left_expr.
        left_expr = (this->*infix_fn)(std::move(left_expr));
        if (!left_expr) return nullptr;
    }
    return left_expr;
}


std::unique_ptr<PrintStatement> Parser::parsePrintStatement() {
    // Current token is PRINT. Consume it. `nextToken` skips comments after PRINT.
    nextToken();

    std::unique_ptr<Expression> expr = parseExpression(LOWEST);
    if (!expr) {
        return nullptr;
    }

    // Expect a semicolon. `expectPeek` handles consuming SEMICOLON and skipping comments before it.
    if (peekTokenIs(SEMICOLON)) {
        nextToken();
    }

    return std::make_unique<PrintStatement>(std::move(expr));
}

std::unique_ptr<Expression> Parser::parseIntegerLiteral() {
    const std::string& lit = currentToken_.literal;
    int base = 10;
    if (currentToken_.type == HEX) base = 16;
    else if (currentToken_.type == OCTAL) base = 8; 

    char* endPtr = nullptr;
    long val = std::strtol(lit.c_str(), &endPtr, base);

    auto expr = std::make_unique<IntegerLiteral>(static_cast<int>(val));
    expr->resolvedType = currentToken_.type;
    nextToken();
    return expr;
}

std::unique_ptr<Expression> Parser::parseStringLiteral() {
    auto expr = std::make_unique<StringLiteral>(currentToken_.literal);
    expr->resolvedType = STRING;
    nextToken();
    return expr;
}

std::unique_ptr<Expression> Parser::parseCharLiteral() {
    char c = currentToken_.literal.empty() ? '\0' : currentToken_.literal[0];
    auto expr = std::make_unique<CharLiteral>(c);
    expr->resolvedType = CHAR;
    nextToken();
    return expr;
}

std::unique_ptr<Expression> Parser::parseIdentifier() {
    return std::make_unique<IdentifierExpr>(currentToken_.literal);
}

std::unique_ptr<Expression> Parser::parseGroupedExpression() {
    // Current token is LPAREN. Consume it. `nextToken` skips comments after '('.
    nextToken();

    std::unique_ptr<Expression> expr = parseExpression(LOWEST);
    if (!expr) {
        return nullptr;
    }

    // Expect and consume the closing RPAREN. `expectPeek` handles comments before ')'.
    if (!expectPeek(RPAREN)) {
        return nullptr;
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseInfixExpression(std::unique_ptr<Expression> left_expr) {
    TokenType op_type = currentToken_.type;
    Precedence prec = currentPrecedence();

    // Consume the operator. `nextToken` skips comments after the operator.
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
    registerPrefix(INT, &Parser::parseIntegerLiteral);
    registerPrefix(IDENTIFIER, &Parser::parseIdentifier);
    registerPrefix(LPAREN, &Parser::parseGroupedExpression);
    registerPrefix(TRUE, &Parser::parseBooleanLiteral);
    registerPrefix(FALSE, &Parser::parseBooleanLiteral);
    registerPrefix(STRING, &Parser::parseStringLiteral);
    registerPrefix(CHAR, &Parser::parseCharLiteral);

    registerInfix(PLUS, &Parser::parseInfixExpression);
    registerInfix(MINUS, &Parser::parseInfixExpression);
    registerInfix(ASTERISK, &Parser::parseInfixExpression);
    registerInfix(SLASH, &Parser::parseInfixExpression);
    registerInfix(ASSIGN, &Parser::parseInfixExpression);
}