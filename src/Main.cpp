#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "token.h"
#include "parser.h"
#include "ast.h"
#include "Codegen.h"

std::string readFileContent(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return ""; // Return empty string on error
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void printAST(const ASTNode* node, int indent = 0) {
    if (!node) return;
    std::string prefix(indent * 2, ' ');

    if (const Program* prog = dynamic_cast<const Program*>(node)) {
        std::cout << prefix << "Program:\n";
        for (const auto& stmt : prog->statements) {
            printAST(stmt.get(), indent + 1);
        }
    }
    else if (const AssignmentStatement* assign = dynamic_cast<const AssignmentStatement*>(node)) {
        std::cout << prefix << "Assignment:\n";
        std::cout << prefix << "  Identifier: " << assign->identifier->name << "\n";
        std::cout << prefix << "  Value:\n";
        printAST(assign->value.get(), indent + 2);
    }
    else if (const ExpressionStatement* expr_stmt = dynamic_cast<const ExpressionStatement*>(node)) {
        std::cout << prefix << "ExpressionStatement:\n";
        printAST(expr_stmt->expression.get(), indent + 1);
    }
    else if (const BinaryExpression* bin_expr = dynamic_cast<const BinaryExpression*>(node)) {
        std::cout << prefix << "BinaryExpr (Op: " << tokenTypeStrings.at(bin_expr->op) << "):\n";
        std::cout << prefix << "  Left:\n";
        printAST(bin_expr->left.get(), indent + 2);
        std::cout << prefix << "  Right:\n";
        printAST(bin_expr->right.get(), indent + 2);
    }
    else if (const IntegerLiteral* int_lit = dynamic_cast<const IntegerLiteral*>(node)) {
        std::cout << prefix << "IntegerLiteral: " << int_lit->value << "\n";
    }
    else if (const IdentifierExpr* id_expr = dynamic_cast<const IdentifierExpr*>(node)) {
        std::cout << prefix << "IdentifierExpr: " << id_expr->name << "\n";
    }
    else {
        std::cout << prefix << "Unknown AST Node Type\n";
    }
}


int main(int argc, char* argv[]) {
    std::string input_code;
    std::string output_filename = "output.s"; // Default output assembly file

    if (argc > 3) {
        std::cerr << "Usage: " << argv[0] << " [input_filename] [output_assembly_filename (optional)]" << std::endl;
        return 1;
    }
    else if (argc >= 2) {
        std::string input_filename = argv[1];
        input_code = readFileContent(input_filename);
        if (input_code.empty()) {
            return 1;
        }
        if (argc == 3) {
            output_filename = argv[2];
        }
        std::cout << "Processing file: " << input_filename << "\n---\n";
    }
    else {
        std::cerr << "Error: No input file specified.\n";
        std::cerr << "Usage: " << argv[0] << " [input_filename] [output_assembly_filename (optional)]" << std::endl;
        return 1;
    }

    std::cout << input_code << "---\n\n";

    // 1. Lexing phase
    Lexer lexer(input_code);

    // 2. Parsing phase
    Parser parser(lexer);
    std::unique_ptr<Program> program_ast = parser.parseProgram();

    // Check for parsing errors
    std::vector<std::string> parser_errors = parser.getErrors();
    if (!parser_errors.empty()) {
        std::cerr << "Parser Errors:\n";
        for (const std::string& err : parser_errors) {
            std::cerr << "  - " << err << std::endl;
        }
        return 1; // Indicate failure
    }
    else if (!program_ast) {
        std::cerr << "Parsing failed, but no specific errors logged (possibly empty input or fundamental parse error).\n";
        return 1;
    }
    else {
        std::cout << "Parsing successful!\n\n";
        std::cout << "--- Generated AST ---\n";
        printAST(program_ast.get());
        std::cout << "\n---------------------\n\n";
    }

    // 3. Code Generation phase
    CodeGenerator code_gen;
    std::string assembly_output = code_gen.generate(program_ast.get());

    // Check for code generation errors
    std::vector<std::string> codegen_errors = code_gen.getErrors();
    if (!codegen_errors.empty()) {
        std::cerr << "Code Generation Errors:\n";
        for (const std::string& err : codegen_errors) {
            std::cerr << "  - " << err << std::endl;
        }
        return 1;
    }
    else {
        std::cout << "Code Generation successful! Output written to " << output_filename << "\n";
    }

    // Write assembly to file
    std::ofstream output_file(output_filename);
    if (output_file.is_open()) {
        output_file << assembly_output;
        output_file.close();
        std::cout << "\n--- Generated Assembly ---\n" << assembly_output << "------------------------\n";
    }
    else {
        std::cerr << "Error: Could not open output file " << output_filename << " for writing.\n";
        return 1;
    }

    return 0;
}