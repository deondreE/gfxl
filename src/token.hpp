#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <vector>

// Enum for different types of tokens the lexer can recognize.
enum TokenType {
    TOKEN_KEYWORD_PRINT,       // 'print' keyword
    TOKEN_STRING_LITERAL,      // "..." string
    TOKEN_SEMICOLON,           // ';'
    TOKEN_EOF,                 // End of file

    // Error token type (useful for signaling lexical errors)
    TOKEN_ERROR
};

// Represents a single token identified by the lexer.
struct Token {
    TokenType type;        // The type of the token
    std::string lexeme;    // The raw text of the token from the source code
    std::string literal;   // The actual value for literals (e.g., "hello" for a string literal)
    int line;              // Line number where the token starts
    int column;            // Column number where the token starts

    // Constructor for convenience
    Token(TokenType type, std::string lexeme, std::string literal, int line, int column)
        : type(type), lexeme(lexeme), literal(literal), line(line), column(column) {}

    // Default constructor for cases where a token might be initialized without full details
    Token() : type(TOKEN_ERROR), lexeme(""), literal(""), line(0), column(0) {}
};

#endif // TOKEN_HPP