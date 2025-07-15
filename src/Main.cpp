#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <typeinfo>

#include "Lexer.h"
#include "Token.h"
#include "Parser.h"
#include "AST.h"
#include "semantic_analyzer.h"
#include "Codegen.h"

extern const std::map<TokenType, std::string> tokenTypeStrings;

// Read entire file into a string
std::string readFileContent(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Print AST to a given output stream with indentation
void printAST(std::ostream& os, const ASTNode* node, int indent = 0) {
    if (!node) return;
    std::string prefix(indent * 2, ' ');

    if (auto prog = dynamic_cast<const Program*>(node)) {
        os << prefix << "Program:\n";
        for (const auto& stmt : prog->statements) {
            printAST(os, stmt.get(), indent + 1);
        }
    }
    else if (auto assign = dynamic_cast<const AssignmentStatement*>(node)) {
        os << prefix << "Assignment:\n";
        os << prefix << "  Identifier: "
            << assign->identifier->name
            << " (Resolved: "
            << tokenTypeStrings.at(assign->identifier->resolvedType)
            << ")\n";
        os << prefix << "  Value:\n";
        printAST(os, assign->value.get(), indent + 2);
    }
    else if (auto expr_stmt = dynamic_cast<const ExpressionStatement*>(node)) {
        os << prefix << "ExpressionStatement (Resolved: "
            << tokenTypeStrings.at(expr_stmt->expression->resolvedType)
            << "):\n";
        printAST(os, expr_stmt->expression.get(), indent + 1);
    }
    else if (auto print_stmt = dynamic_cast<const PrintStatement*>(node)) {
        os << prefix << "PrintStatement (Arg: "
            << tokenTypeStrings.at(print_stmt->expression->resolvedType)
            << "):\n";
        printAST(os, print_stmt->expression.get(), indent + 1);
    }
    else if (auto bin_expr = dynamic_cast<const BinaryExpression*>(node)) {
        os << prefix << "BinaryExpr (Op: "
            << tokenTypeStrings.at(bin_expr->op)
            << ", Resolved: "
            << tokenTypeStrings.at(bin_expr->resolvedType)
            << "):\n";
        os << prefix << "  Left:\n";
        printAST(os, bin_expr->left.get(), indent + 2);
        os << prefix << "  Right:\n";
        printAST(os, bin_expr->right.get(), indent + 2);
    }
    else if (auto int_lit = dynamic_cast<const IntegerLiteral*>(node)) {
        os << prefix << "IntegerLiteral: " << int_lit->value
            << " (Resolved: "
            << tokenTypeStrings.at(int_lit->resolvedType)
            << ")\n";
    }
    else if (auto bool_lit = dynamic_cast<const BooleanLiteral*>(node)) {
        os << prefix << "BooleanLiteral: "
            << (bool_lit->value ? "true" : "false")
            << " (Resolved: "
            << tokenTypeStrings.at(bool_lit->resolvedType)
            << ")\n";
    }
    else if (auto str_lit = dynamic_cast<const StringLiteral*>(node)) {
        os << prefix << "StringLiteral: \"" << str_lit->value
            << "\" (Resolved: "
            << tokenTypeStrings.at(str_lit->resolvedType)
            << ")\n";
    }
    else if (auto char_lit = dynamic_cast<const CharLiteral*>(node)) {
        os << prefix << "CharLiteral: '" << char_lit->value
            << "' (Resolved: "
            << tokenTypeStrings.at(char_lit->resolvedType)
            << ")\n";
    }
    else if (auto id_expr = dynamic_cast<const IdentifierExpr*>(node)) {
        os << prefix << "IdentifierExpr: " << id_expr->name
            << " (Resolved: "
            << tokenTypeStrings.at(id_expr->resolvedType)
            << ")\n";
    }
    else {
        os << prefix << "Unknown AST Node ("
            << typeid(*node).name() << ")\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0]
            << " [input_file] [output_asm_file (optional)]\n";
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_asm = (argc == 3 ? argv[2] : "output.s");

    // Read source
    std::string source = readFileContent(input_filename);
    if (source.empty()) return 1;

    std::cout << "Processing " << input_filename << " ...\n\n";
    std::cout << source << "\n---\n\n";

    // Lexing & Parsing
    Lexer lexer(source);
    Parser parser(lexer);
    auto program_ast = parser.parseProgram();

    if (!parser.getErrors().empty()) {
        std::cerr << "Parser Errors:\n";
        for (auto& e : parser.getErrors()) {
            std::cerr << "  - " << e << "\n";
        }
        return 1;
    }
    std::cout << "Parsing successful.\n\n";

    // Semantic Analysis
    SemanticAnalyzer sema;
    sema.analyze(*program_ast);
    if (!sema.getErrors().empty()) {
        std::cerr << "Semantic Errors:\n";
        for (auto& e : sema.getErrors()) {
            std::cerr << "  - " << e << "\n";
        }
        return 1;
    }
    std::cout << "Semantic analysis successful.\n\n";

    // Write AST to file
    {
        std::ofstream ast_file("ast.txt");
        if (!ast_file.is_open()) {
            std::cerr << "Error: Could not open ast.txt for writing.\n";
            return 1;
        }
        printAST(ast_file, program_ast.get());
    }
    std::cout << "AST written to ast.txt\n\n";

    // Code Generation
    CodeGenerator codegen;
    std::string asm_out = codegen.generate(program_ast.get());
    if (!codegen.getErrors().empty()) {
        std::cerr << "Codegen Errors:\n";
        for (auto& e : codegen.getErrors()) {
            std::cerr << "  - " << e << "\n";
        }
        return 1;
    }
    std::cout << "Code generation successful. Writing to "
        << output_asm << "\n";

    std::ofstream out_file(output_asm);
    if (!out_file.is_open()) {
        std::cerr << "Error: Could not open " << output_asm
            << " for writing.\n";
        return 1;
    }
    out_file << asm_out;
    return 0;
}