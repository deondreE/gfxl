#include "Codegen.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>

const int WORD_SIZE = 8;

CodeGenerator::CodeGenerator() : stackOffsetCounter_(0) {
    assembly_code_ += ".intel_syntax noprefix\n";
    assembly_code_ += ".global _start\n";
    assembly_code_ += ".text\n";
}

std::string CodeGenerator::generate(const Program* program_ast) {
    if (!program_ast) {
        error("Cannot generate code for null program AST.");
        return "";
    }

    emit("main:");
    emit("  push rbp");
    emit("  mov rbp, rsp");

    visit(program_ast);

    emit("  mov rax, 60");
    emit("  xor rdi, rdi");
    emit("  syscall");

    emit("_start:");
    emit("  call main");
    emit("  mov rax, 60");
    emit("  xor rdi, rdi");
    emit("  syscall");

    return assembly_code_;
}

std::vector<std::string> CodeGenerator::getErrors() const {
    return errors_;
}

void CodeGenerator::emit(const std::string& instruction) {
    assembly_code_ += instruction + "\n";
}

void CodeGenerator::error(const std::string& msg) {
    errors_.push_back("Code Generation Error: " + msg);
}

void CodeGenerator::visit(const Program* node) {
    for (const auto& stmt : node->statements) {
        visit(stmt.get());
    }
}

void CodeGenerator::visit(const Statement* node) {
    if (const auto* assign = dynamic_cast<const AssignmentStatement*>(node)) {
        visit(assign);
    }
    else if (const auto* expr = dynamic_cast<const ExpressionStatement*>(node)) {
        visit(expr);
    }
    else if (const auto* print = dynamic_cast<const PrintStatement*>(node)) {
        visit(print);
    }
    else {
        error("Unknown statement type.");
    }
}

void CodeGenerator::visit(const AssignmentStatement* node) {
    visit(node->value.get());
    pop_into_rax();

    std::string var = node->identifier->name;
    Symbol* sym = getSymbol(var);
    if (!sym) {
        defineVariable(var);
        sym = getSymbol(var);
    }

    if (sym) {
        std::stringstream ss;
        ss << "  mov QWORD PTR [rbp - " << sym->stackOffset << "], rax";
        emit(ss.str());
    }
    else {
        error("Failed to resolve symbol: " + var);
    }
}

void CodeGenerator::visit(const ExpressionStatement* node) {
    visit(node->expression.get());
    emit("  pop rax");
}

void CodeGenerator::visit(const PrintStatement* node) {
    visit(node->expression.get());
    emit("  pop rdi");        // Value to print
    emit("  call print_int"); // You must define print_int elsewhere (e.g. in another asm file)
}

void CodeGenerator::visit(const Expression* node) {
    if (const auto* int_lit = dynamic_cast<const IntegerLiteral*>(node)) {
        visit(int_lit);
    }
    else if (const auto* ident = dynamic_cast<const IdentifierExpr*>(node)) {
        visit(ident);
    }
    else if (const auto* bin = dynamic_cast<const BinaryExpression*>(node)) {
        visit(bin);
    }
    else {
        error("Unknown expression type.");
    }
}

void CodeGenerator::visit(const IntegerLiteral* node) {
    std::stringstream ss;
    ss << "  push " << node->value;
    emit(ss.str());
}

void CodeGenerator::visit(const IdentifierExpr* node) {
    Symbol* sym = getSymbol(node->name);
    if (sym) {
        std::stringstream ss;
        ss << "  push QWORD PTR [rbp - " << sym->stackOffset << "]";
        emit(ss.str());
    }
    else {
        error("Undefined variable: " + node->name);
        emit("  push 0");
    }
}

void CodeGenerator::visit(const BinaryExpression* node) {
    visit(node->right.get());
    visit(node->left.get());
    pop_into_rax();
    emit("  pop rbx");

    switch (node->op) {
    case PLUS:
        emit("  add rax, rbx");
        break;
    case MINUS:
        emit("  sub rax, rbx");
        break;
    case ASTERISK:
        emit("  imul rax, rbx");
        break;
    case SLASH:
        emit("  xor rdx, rdx");
        emit("  idiv rbx");
        break;
    default:
        error("Unsupported binary operator.");
        break;
    }

    push_from_rax();
}

void CodeGenerator::defineVariable(const std::string& name) {
    stackOffsetCounter_ += WORD_SIZE;
    symbolTable_[name] = { stackOffsetCounter_ };
}

Symbol* CodeGenerator::getSymbol(const std::string& name) {
    auto it = symbolTable_.find(name);
    return (it != symbolTable_.end()) ? &it->second : nullptr;
}

void CodeGenerator::pop_into_rax() {
    emit("  pop rax");
}

void CodeGenerator::push_from_rax() {
    emit("  push rax");
}
