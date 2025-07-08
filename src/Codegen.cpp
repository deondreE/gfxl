#include "Codegen.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>

const int WORD_SIZE = 8;

CodeGenerator::CodeGenerator() : stackOffsetCounter_(0) { 
    assembly_code_ += ".text\n";
}

std::string CodeGenerator::generate(const Program* program_ast) {
    if (!program_ast) {
        error("Cannot generate code for null program AST.");
        return "";
    }

    emit("  .globl main");
    emit("  .globl print_int");
    emit("main:");
    emit("  pushq %rbp");
    emit("  movq %rsp, %rbp");

    visit(program_ast);

    emit("  movq $0, %rax");  // return 0
    emit("  leave");
    emit("  ret");

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
        ss << "  movq %rax, -" << sym->stackOffset << "(%rbp)";
        emit(ss.str());
    } else {
        error("Failed to resolve symbol: " + var);
    }
}

void CodeGenerator::visit(const ExpressionStatement* node) {
    visit(node->expression.get());
    emit("  popq %rax");
}

void CodeGenerator::visit(const PrintStatement* node) {
    visit(node->expression.get());
    emit("  popq %rdi");
    emit("  call print_int");
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
    ss << "  pushq $" << node->value;
    emit(ss.str());
}

void CodeGenerator::visit(const IdentifierExpr* node) {
    Symbol* sym = getSymbol(node->name);
    if (sym) {
        std::stringstream ss;
        ss << "  pushq -" << sym->stackOffset << "(%rbp)";
        emit(ss.str());
    } else {
        error("Undefined variable: " + node->name);
        emit("  pushq $0");
    }
}

void CodeGenerator::visit(const BinaryExpression* node) {
    visit(node->right.get());
    visit(node->left.get());
    emit("  popq %rbx");
    emit("  popq %rax");

    switch (node->op) {
    case PLUS:
        emit("  addq %rbx, %rax");
        break;
    case MINUS:
        emit("  subq %rbx, %rax");
        break;
    case ASTERISK:
        emit("  imulq %rbx, %rax");
        break;
    case SLASH:
        emit("  xorq %rdx, %rdx");
        emit("  idivq %rbx");
        break;
    default:
        error("Unsupported binary operator.");
        break;
    }

    push_from_rax();
}

void CodeGenerator::defineVariable(const std::string& name) {
    stackOffsetCounter_ -= WORD_SIZE;
    symbolTable_[name] = { -stackOffsetCounter_ };
}

Symbol* CodeGenerator::getSymbol(const std::string& name) {
    auto it = symbolTable_.find(name);
    return (it != symbolTable_.end()) ? &it->second : nullptr;
}

void CodeGenerator::pop_into_rax() {
    emit("  popq %rax");
}

void CodeGenerator::push_from_rax() {
    emit("  pushq %rax");
}
