#include "lexer.hpp"
#include <iostream> // For error output
#include <cctype>   // For isalpha, isalnum, isdigit

Lexer::Lexer(const std::string& source) : source(source) {}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current; // Reset 'start' to the beginning of the new lexeme
        scanToken();     // Scan the next token
    }

    // Add the End-Of-File token at the very end
    tokens.emplace_back(TOKEN_EOF, "", "", line, current);
    return tokens;
}

bool Lexer::isAtEnd() const {
    return current >= source.length();
}

char Lexer::advance() {
    column++;
    return source[current++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0'; // Null character to indicate end
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

void Lexer::addToken(TokenType type) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, "", line, column - (current - start)); // Adjusted column
}

void Lexer::addToken(TokenType type, const std::string& literal_value) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, literal_value, line, column - (current - start)); // Adjusted column
}

void Lexer::scanToken() {
    char c = advance(); // Get the current character and move forward

    switch (c) {
        case ';':
            addToken(TOKEN_SEMICOLON);
            break;
        case '"':
            string(); // Handle string literals
            break;
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace characters
            break;
        case '\n':
            // Newline character, increment line number and reset column
            line++;
            column = 1;
            break;
        default:
            if (isalpha(c)) { // Start of an identifier or keyword
                identifier();
            } else if (isdigit(c)) { // Start of a number (not used in this minimal example, but for future)
                // number();
                error("Numbers are not supported in this tiny GLX version.");
            }
            else {
                error("Unexpected character: '" + std::string(1, c) + "'");
            }
            break;
    }
}

void Lexer::string() {
    // Consume characters until closing quote or end of file
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
            column = 1; // Reset column for new line
        }
        advance();
    }

    if (isAtEnd()) {
        error("Unterminated string literal.");
        return;
    }

    advance(); // Consume the closing '"'

    // Extract the string value (between the quotes)
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TOKEN_STRING_LITERAL, value);
}

void Lexer::identifier() {
    // Consume alphanumeric characters for the identifier/keyword
    while (isalnum(peek())) {
        advance();
    }

    std::string text = source.substr(start, current - start);

    // Check if the identifier is a reserved keyword
    if (text == "print") {
        addToken(TOKEN_KEYWORD_PRINT);
    } else {
        // If it's not a keyword, it's an unrecognized identifier for now
        // In a fuller language, this would be a user-defined identifier.
        error("Unknown identifier or keyword: '" + text + "'");
    }
}

void Lexer::error(const std::string& message) {
    std::cerr << "[Lexer Error] Line " << line << ", Column " << column << ": " << message << std::endl;
    // In a real compiler, you might store errors and continue, or exit.
    // For this simple example, we'll just print and continue scanning,
    // but the parser will likely fail if tokens are incorrect.
}