#include "parser.hpp"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::unique_ptr<Stmt> Parser::parse() {
    // For this simple language, we expect exactly one statement followed by EOF.
    // In a real language, this would be a loop parsing multiple statements.
    if (isAtEnd() || peek().type == TOKEN_EOF) {
        std::cerr << "[Parser Error] Empty program." << std::endl;
        return nullptr;
    }
    std::unique_ptr<Stmt> program_stmt = statement();
    if (peek().type != TOKEN_EOF) {
        error(peek(), "Expected end of file after statement.");
        return nullptr; // Parsing failed due to extra tokens
    }
    return program_stmt;
}

// Attempts to parse a general statement.
std::unique_ptr<Stmt> Parser::statement() {
    if (match(TOKEN_KEYWORD_PRINT)) {
        return printStatement();
    }
    // If it's not a recognized statement type, report an error.
    error(peek(), "Expected a 'print' statement.");
    return nullptr;
}

// Parses a 'print' statement.
// Grammar: print_statement ::= "print" STRING_LITERAL SEMICOLON
std::unique_ptr<PrintStmt> Parser::printStatement() {
    // After 'print', we expect a string literal.
    if (!check(TOKEN_STRING_LITERAL)) {
        error(peek(), "Expected a string literal after 'print'.");
        return nullptr;
    }
    Token stringToken = advance(); // Consume the string literal token

    // After the string literal, we expect a semicolon.
    if (!match(TOKEN_SEMICOLON)) {
        error(peek(), "Expected ';' after print statement.");
        return nullptr;
    }

    // Return a new PrintStmt AST node.
    return std::make_unique<PrintStmt>(stringToken.literal);
}

// Helper methods:

// Consumes the current token and returns it.
Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

// Looks at the current token without consuming it.
Token Parser::peek() const {
    return tokens[current];
}

// Looks at the previously consumed token.
Token Parser::previous() const {
    return tokens[current - 1];
}

// Checks if the end of the token stream has been reached.
bool Parser::isAtEnd() const {
    return peek().type == TOKEN_EOF;
}

// Checks if the current token is of the specified type without consuming it.
bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

// Consumes the current token if it matches the given type. Returns true on match, false otherwise.
bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

// Reports a parsing error.
void Parser::error(const Token& token, const std::string& message) {
    if (token.type == TOKEN_EOF) {
        std::cerr << "[Parser Error] Line " << token.line << ", Column " << token.column << " at end: " << message << std::endl;
    } else {
        std::cerr << "[Parser Error] Line " << token.line << ", Column " << token.column << " at '" << token.lexeme << "': " << message << std::endl;
    }
    // In a real compiler, you'd have more sophisticated error recovery or throw an exception.
    // For this example, printing and returning nullptr is sufficient.
}