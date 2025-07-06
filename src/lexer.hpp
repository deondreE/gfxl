#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "token.hpp" // Include our Token definition

class Lexer {
public:
    // Constructor takes the source code string.
    Lexer(const std::string& source);

    // Scans the entire source code and returns a vector of tokens.
    std::vector<Token> scanTokens();

private:
    const std::string& source; // Reference to the source code string.
    std::vector<Token> tokens; // The list of tokens being built.

    // Change int to std::string::size_type for indices and potentially line/column
    // This removes the signed/unsigned comparison warnings.
    std::string::size_type start = 0;   // Index of the first character of the current lexeme.
    std::string::size_type current = 0; // Index of the character currently being considered.
    std::string::size_type line = 1;    // Current line number in the source code.
    std::string::size_type column = 1;  // Current column number in the source code.

    // Helper methods for scanning:
    bool isAtEnd() const;   // Checks if the end of the source has been reached.
    char advance();         // Consumes the current character and returns it.
    char peek() const;      // Looks at the current character without consuming it.
    char peekNext() const;  // Looks at the next character after the current without consuming.

    void addToken(TokenType type);                  // Adds a token without a specific literal value.
    void addToken(TokenType type, const std::string& literal_value); // Adds a token with a literal value.

    void scanToken();       // Scans a single token based on the current character.
    void string();          // Handles scanning string literals.
    void identifier();      // Handles scanning identifiers and keywords.
    void number();          // Handles scanning number literals (not used in current minimal example, but good to have).

    // Error reporting
    void error(const std::string& message);
};

#endif // LEXER_HPP