#include "Lexer.h"
#include <cctype>

const std::map<TokenType, std::string> tokenTypeStrings = {
    {ILLEGAL, "ILLEGAL"},
    {END_OF_FILE, "EOF"},
    {IDENTIFIER, "IDENTIFIER"},
    {INT, "INT"},
    {BOOL, "BOOL"},
    {ASSIGN, "ASSIGN"},
    {PLUS, "PLUS"},
    {MINUS, "MINUS"},
    {ASTERISK, "ASTERISK"},
    {SLASH, "SLASH"},
    {SEMICOLON, "SEMICOLON"},
    {LPAREN, "LPAREN"},
    {RPAREN, "RPAREN"},
    {PRINT, "PRINT"},
    {TRUE, "TRUE"},
    {FALSE, "FALSE"}
};

std::string Token::toString() const {
    std::string typeStr;
    auto it = tokenTypeStrings.find(type);
    if (it != tokenTypeStrings.end()) {
        typeStr = it->second;
    }
    else {
        typeStr = "UNKNOWN_TOKEN_TYPE";
    }
    return "Token(Type: " + typeStr + ", Literal: \"" + literal + "\")";
}

Lexer::Lexer(const std::string& input) 
: input_(input), position_(0), readPosition_(0), ch_(0) {
    readChar();
}

Token Lexer::nextToken() {
    skipWhitespace();

    Token token;

    // --- Identifiers or keywords ------------------------------------------
    if (isLetter(ch_)) {
        std::string literal = readIdentifier();

        TokenType type; // Declare the TokenType variable

        if (literal == "print") {
            type = PRINT;
        }
        else if (literal == "true") {
            type = TRUE;
        }
        else if (literal == "false") {
            type = FALSE;
        }
        else {
            type = IDENTIFIER;
        }
        return { type, literal };
    }

    // --- Numeric literals --------------------------------------------------
    if (isDigit(ch_)) {
        std::string literal = readNumber();            // advances position_
        return { INT, literal };
    }

    // --- Single‑character tokens ------------------------------------------
    switch (ch_) {
    case '=':  token = { ASSIGN, "=" };     break;
    case '+':  token = { PLUS, "+" };       break;
    case '-':  token = { MINUS, "-" };      break;
    case '*':  token = { ASTERISK, "*" };   break;
    case '/':  token = { SLASH, "/" };      break;
    case ';':  token = { SEMICOLON, ";" };  break;
    case '(':  token = { LPAREN, "(" };     break;
    case ')':  token = { RPAREN, ")" };     break;
    case 0:    token = { END_OF_FILE, "" }; break; // NUL ⇒ EOF
    default:   token = { ILLEGAL, std::string(1, ch_) }; break;
    }

    readChar();  // advance past the single‑character token
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
