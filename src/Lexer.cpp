// Lexer.cpp
#include "Lexer.h"
#include <cctype>

const std::map<TokenType, std::string> tokenTypeStrings = {
    {ILLEGAL,            "ILLEGAL"},
    {END_OF_FILE,        "EOF"},
    {IDENTIFIER,         "IDENTIFIER"},
    {INT,                "INT"},
    {BOOL,               "BOOL"},
    {FLOAT,              "FLOAT"},
    {STRING,             "STRING"},
    {CHAR,               "CHAR"},
    {ASSIGN,             "ASSIGN"},
    {COLON,              "COLON"},
    {OCTAL,              "OCTAL"},
    {HEX,                "HEX"},
    {PLUS,               "PLUS"},
    {MINUS,              "MINUS"},
    {ASTERISK,           "ASTERISK"},
    {SLASH,              "SLASH"},
    {SEMICOLON,          "SEMICOLON"},
    {LPAREN,             "LPAREN"},
    {RPAREN,             "RPAREN"},
    {PRINT,              "PRINT"},
    {TRUE,               "TRUE"},
    {FALSE,              "FALSE"},
    {COMMENT_MULTI_LINE, "COMMENT_MULTI_LINE"},
    {COMMENT_SINGLE_LINE,"COMMENT_SINGLE_LINE"}
};

std::string Token::toString() const {
    auto it = tokenTypeStrings.find(type);
    const std::string& typeStr = (it != tokenTypeStrings.end())
        ? it->second
        : "UNKNOWN_TOKEN_TYPE";
    return "Token(Type: " + typeStr + ", Literal: \"" + literal + "\")";
}

Lexer::Lexer(const std::string& input)
    : input_(input), position_(0), readPosition_(0), ch_(0)
{
    advance();
}

// Advance one char, setting ch_ = next character or 0 at EOF
void Lexer::advance() {
    if (readPosition_ < input_.size()) {
        ch_ = input_[readPosition_++];
    }
    else {
        ch_ = 0;
    }
    position_ = readPosition_ - 1;
}

// Peek ahead N characters, returning 0 on EOF
char Lexer::peek(size_t ahead) const {
    size_t pos = readPosition_ + ahead;
    return (pos < input_.size()) ? input_[pos] : char{ 0 };
}

// Skip whitespace, single-line (#…) and multi-line (###…###) comments
void Lexer::skipIgnorable() {
    while (true) {
        // whitespace
        while (std::isspace(static_cast<unsigned char>(ch_))) {
            advance();
        }
        // multiline comment
        if (ch_ == '#' && peek(0) == '#' && peek(1) == '#') {
            skipMultilineComment();
            continue;
        }
        // single-line comment
        if (ch_ == '#') {
            skipSinglelineComment();
            continue;
        }
        break;
    }
}

Token Lexer::nextToken() {
    skipIgnorable();

    // -- string literal
    if (ch_ == '"') {
        std::string lit = readString();
        return { STRING, lit };
    }

    // ? char literal
    if (ch_ == '\'') {
        std::string lit = readCharLiteral();
        return { CHAR, lit };
    }


    // identifier or keyword
    if (std::isalpha(static_cast<unsigned char>(ch_)) || ch_ == '_') {
        size_t start = position_;
        while (std::isalnum(static_cast<unsigned char>(ch_)) || ch_ == '_') {
            advance();
        }
        std::string lit = input_.substr(start, position_ - start);
        return { lookupIdent(lit), lit };
    }

    // Numeric literal: hex, ocatal, int, float
    if (std::isdigit(static_cast<unsigned char>(ch_))) {
        if (ch_ = '0' && peek() == 'x' || peek() == 'X') {
            size_t start = position_;
            advance();
            advance();
            while (std::isdigit(static_cast<unsigned char>(ch_))) {
                advance();
            }
            std::string lit = input_.substr(start, position_ - start);

            return { HEX, lit };
        }

        size_t start = position_;
        while (std::isdigit(static_cast<unsigned char>(ch_))) {
            advance();
        }

        // do we have a float
        if (ch_ == '.' && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
            while (std::isdigit(static_cast<unsigned char>(ch_))) {
                advance();
            }
            std::string lit = input_.substr(start, position_ - start);
            return { FLOAT, lit };
        }

        std::string lit = input_.substr(start, position_ - start);
        if (lit.size() > 1 && lit[0] == '0') {
            return { OCTAL, lit };
        } 
        
        return { INT, lit };
    }

    // single-character tokens
    Token tok;
    switch (ch_) {
    case '=': tok = { ASSIGN,    "=" }; break;
    case '+': tok = { PLUS,      "+" }; break;
    case '-': tok = { MINUS,     "-" }; break;
    case '*': tok = { ASTERISK,  "*" }; break;
    case '/': tok = { SLASH,     "/" }; break;
    case ';': tok = { SEMICOLON, ";" }; break;
    case '(': tok = { LPAREN,    "(" }; break;
    case ')': tok = { RPAREN,    ")" }; break;
    case ':': tok = { COLON,     ":" }; break;
    case  0: tok = { END_OF_FILE, "" }; break;
    default:  tok = { ILLEGAL, std::string(1, ch_) }; break;
    }
    advance();
    return tok;
}

TokenType Lexer::lookupIdent(const std::string& lit) const {
    if (lit == "print") return PRINT;
    else if (lit == "true")  return TRUE;
    else if (lit == "false") return FALSE;
    else                      return IDENTIFIER;
}

std::string Lexer::readString() {
    advance();
    size_t start = position_;
    while (ch_ != '"' && ch_ != 0) {
        advance();
    }

    std::string str = input_.substr(start, position_ - start);
    advance();
    return str;
}

std::string Lexer::readCharLiteral() {
    advance();

    if (ch_ == 0) return "";
    char c = ch_;
    advance(); 

    if (ch_ != '\'') {

    }
    advance();
    return std::string(1, c);
}

void Lexer::skipSinglelineComment() {
    // assume ch_ == '#'
    while (ch_ != '\n' && ch_ != 0) {
        advance();
    }
    if (ch_ == '\n') {
        advance();
    }
}

void Lexer::skipMultilineComment() {
    // consume opening ###
    advance(); advance(); advance();
    while (ch_ != 0) {
        if (ch_ == '#' && peek(0) == '#' && peek(1) == '#') {
            // consume closing ###
            advance(); advance(); advance();
            break;
        }
        advance();
    }
}