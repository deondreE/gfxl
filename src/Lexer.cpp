#include "Lexer.h"
#include <cctype>

const std::map<TokenType, std::string> tokenTypeStrings = {
    {ILLEGAL, "ILLEGAL"},
    {END_OF_FILE, "EOF"},
    {IDENTIFIER, "IDENTIFIER"},
    {INT, "INT"},
    {ASSIGN, "ASSIGN"},
    {PLUS, "PLUS"},
    {MINUS, "MINUS"},
    {ASTERISK, "ASTERISK"},
    {SLASH, "SLASH"},
    {SEMICOLON, "SEMICOLON"},
    {LPAREN, "LPAREN"},
    {RPAREN, "RPAREN"}
};

std::string Token::toString() const {
    return "Token { Type: " + tokenTypeStrings.at(type) + ", Literal: '" + literal + "' }";
}

Lexer::Lexer(const std::string& input) 
: input_(input), position_(0), readPosition_(0), ch_(0) {
    readChar();
}

Token Lexer::nextToken() {
    skipWhitespace();

    Token token;

    switch (ch_) {
    case '=':
        token = { ASSIGN, "=" };
        break;
    case '+':
        token = { PLUS, "+" };
        break;
    case '-':
        token = { MINUS, "-" };
        break;
    case '*':
        token = { ASTERISK, "*" };
        break;
    case '/':
        token = { SLASH, "/" };
        break;
    case ';':
        token = { SEMICOLON, ";" };
        break;
    case '(':
        token = { LPAREN, "(" };
        break;
    case ')':
        token = { RPAREN, ")" };
        break;
    case 0: // NULL Char EOF
        token = { END_OF_FILE, "" };
        break;
    default:
        if (isLetter(ch_)) {
            token.literal = readIdentifier();
            token.type = IDENTIFIER;
            return token;
        }
        else if (isDigit(ch_)) {
            token.literal = readNumber();
            token.type = INT;
            return token;
        }
        else {
            token = { ILLEGAL, std::string(1, ch_) };
        }
        break;
    }

    readChar();
    return token;
}

std::string Lexer::readIdentifier() {
	int startPosition = position_;	
	while (isLetter(ch_) || isDigit(ch_)) {
		readChar();
	}

	return input_.substr(startPosition, position_ - startPosition);
}

void Lexer::readChar() {
    if (readPosition_ >= input_.length()) {
        ch_ = 0;
    }
    else {
        ch_ = input_[readPosition_];
    }
    position_ = readPosition_;
    readPosition_++;
}

char Lexer::peekChar() {
    if (readPosition_ >= input_.length()) {
        return 0;
    }
    else {
        return input_[readPosition_];
    }
}

std::string Lexer::readNumber() {
    int startPosition = position_;
    while (isDigit(ch_)) {
        readChar();
    }
    return input_.substr(startPosition, position_ - startPosition);
}

void Lexer::skipWhitespace() {
    while (std::isspace(static_cast<unsigned char>(ch_))) {
        readChar(); 
    }
}

bool Lexer::isLetter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

bool Lexer::isDigit(char ch) {
    return ch >= '0' && ch <= '9';
}
