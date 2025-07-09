#pragma once

#include <string>
#include <map>

enum TokenType {
	ILLEGAL, // '\0'
	END_OF_FILE, 

	// Indentifier Literals
	IDENTIFIER,
	INT,

	ASSIGN, // = 
	PLUS, // +
	MINUS, // - 
	ASTERISK, // *
	SLASH, // /

	SEMICOLON,
	LPAREN, // (
	RPAREN, // )
	TRUE, // 0
	FALSE, // 1
	PRINT, 
};

extern const std::map<TokenType, std::string> tokenTypeStrings;

struct Token {
	TokenType type;
	std::string literal;

	std::string toString() const;
};