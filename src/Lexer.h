#pragma once

#include <string>
#include "Token.h"

class Lexer
{
public:
	Lexer(const std::string& input);

	Token nextToken();

private:
	std::string input_;
	int position_;
	int readPosition_;
	char ch_;

	void readChar();
	char peekChar();
	std::string readIdentifier();
	std::string readNumber();	
	void skipWhitespace();

	bool isLetter(char ch);
	bool isDigit(char ch);
};

