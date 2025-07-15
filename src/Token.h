#pragma once

#include <string>
#include <map>

enum TokenType {
    ILLEGAL,
    END_OF_FILE,
    IDENTIFIER,
    
    INT,
    FLOAT,
    STRING,
    OCTAL,
    HEX,
    CHAR,
    BOOL,

    ASSIGN,
    PLUS,
    MINUS,
    ASTERISK,
    SLASH,
    SEMICOLON,
    COLON,
    LPAREN,
    RPAREN,

    PRINT,
    TRUE,
    FALSE,

    COMMENT_MULTI_LINE,
    COMMENT_SINGLE_LINE
};
extern const std::map<TokenType, std::string> tokenTypeStrings;

struct Token {
	TokenType type;
	std::string literal;

	std::string toString() const;
};