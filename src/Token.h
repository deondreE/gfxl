#pragma once

#include <string>
#include <map>

enum TokenType {
	ILLEGAL, // '\0'
	END_OF_FILE, 

	// Indentifier Literals
	IDENTIFIER,
	INT,
	BOOL,

	ASSIGN, // = 
	PLUS, // +
	MINUS, // - 
	ASTERISK, // *
	SLASH, // /

	SEMICOLON,
	LPAREN, // (
	RPAREN, // )
	PRINT, 

	TRUE,
	FALSE,
};

extern const std::map<TokenType, std::string> tokenTypeStrings;

struct Token {
	TokenType type;
	std::string literal;

	std::string toString() const;
};