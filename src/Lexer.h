#pragma once

#include <string>
#include "Token.h"    // brings in Token, TokenType, and tokenTypeStrings

class Lexer {
public:
    explicit Lexer(const std::string& input);
    Token nextToken();

private:
    std::string input_;
    size_t      position_;      // current char index
    size_t      readPosition_;  // next char index
    char        ch_;            // current char under examination

    // Advance by one character (or set ch_ = 0 at EOF)
    void advance();

    // Peek ahead 'ahead' characters; returns 0 at EOF
    char peek(size_t ahead = 0) const;

    // Skip whitespace and both kinds of comments
    void skipIgnorable();

    // Skip until end-of-line or EOF (assumes ch_ == '#')
    void skipSinglelineComment();

    // Skip until closing ### (assumes ch_,peek(0),peek(1) == '#','\#','\#')
    void skipMultilineComment();

    // Map an identifier string to either IDENTIFIER or a keyword token
    TokenType lookupIdent(const std::string& lit) const;

    std::string readString();
    std::string readCharLiteral();
};