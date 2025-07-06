#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "token.hpp" // Just for displaying token types in debug output
#include "ast.hpp"   // Just for showing AST conceptually in debug output

// Helper to print token types for debugging
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_KEYWORD_PRINT: return "KEYWORD_PRINT";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.glx>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'\n";
        return 1;
    }

    // Read the entire file content into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    std::cout << "--- GLX Compiler/Interpreter (Tiny Version) ---\n";
    std::cout << "Source file: " << filename << "\n";
    std::cout << "------------------------------------------------\n\n";

    // --- LEXICAL ANALYSIS ---
    std::cout << "Phase 1: Lexical Analysis (Scanning)\n";
    std::cout << "-------------------------------------\n";
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    bool lexer_has_errors = false;
    for (const auto& token : tokens) {
        std::cout << "  Token { Type: " << tokenTypeToString(token.type)
                  << ", Lexeme: '" << token.lexeme
                  << "', Literal: '" << token.literal
                  << "', Line: " << token.line
                  << ", Column: " << token.column << " }\n";
        if (token.type == TOKEN_ERROR) {
            lexer_has_errors = true;
        }
    }
    if (lexer_has_errors) {
        std::cerr << "\nLexical analysis completed with errors. Aborting.\n";
        return 1;
    }
    std::cout << "\nLexical analysis complete. " << tokens.size() << " tokens found.\n\n";

    // --- SYNTACTIC ANALYSIS ---
    std::cout << "Phase 2: Syntactic Analysis (Parsing)\n";
    std::cout << "-------------------------------------\n";
    Parser parser(tokens);
    std::unique_ptr<Stmt> ast_root = parser.parse(); // Our simple AST root is just one statement

    if (!ast_root) {
        std::cerr << "\nParsing failed. Aborting.\n";
        return 1;
    }
    std::cout << "Parsing complete. AST conceptually built.\n\n";
    // In a real scenario, you might have an AST pretty-printer here to visualize.
    // For this simple case, we know it's a PrintStmt.

    // --- INTERPRETATION / CODE EXECUTION ---
    std::cout << "Phase 3: Interpretation (Execution)\n";
    std::cout << "-----------------------------------\n";
    Interpreter interpreter;
    // Execute the AST. The interpreter will dispatch to the correct visitor method.
    interpreter.interpret(*ast_root);

    std::cout << "\n------------------------------------------------\n";
    std::cout << "GLX program execution complete.\n";

    return 0;
}