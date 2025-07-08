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
	emit(" push rbp");
	emit(" mov rbp, rsp");

	visit(program_ast);

	emit(" mov rax, 60");
	emit(" xor rdi, rdi");
	emit(" syscall");

	emit("_start:");
	emit(" call main");
	emit(" mov rax, 60");
	emit(" xor rdi, rdi");
	emit(" syscall");

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
	if (const AssignmentStatement* assign_stmt = dynamic_cast<const AssignmentStatement*>(node)) {
		visit(assign_stmt);
	}
	else if (const ExpressionStatement* expr_stmt = dynamic_cast<const ExpressionStatement*>(node)) {
		visit(expr_stmt);
	}
	else {
		error("Unknown statement type encountered during code generation.");
	}
}

void CodeGenerator::visit(const AssignmentStatement* node) {
	// 1. Evaluate the right-hand side expression.
	//    Its result will be left on the stack.
	visit(node->value.get());

	// 2. Pop the result into RAX.
	pop_into_rax(); // Pop value into RAX

	// 3. Store the value from RAX into the variable's memory location.
	std::string var_name = node->identifier->name;
	Symbol* sym = getSymbol(var_name); // Get existing variable
	if (!sym) {
		defineVariable(var_name);
		sym = getSymbol(var_name);
	}

	if (sym) {
		// Store RAX into [rbp + offset]
		std::stringstream ss;
		ss << "  mov QWORD PTR [rbp - " << sym->stackOffset << "], rax";
		emit(ss.str());
	}
	else {
		error("Failed to allocate or find variable: " + var_name);
	}
}

void CodeGenerator::visit(const ExpressionStatement* node) {
	visit(node->expression.get());
	emit("  pop rax"); // Clean up the stack after an expression statement
}

void CodeGenerator::visit(const Expression* node) {
	if (const IntegerLiteral* int_lit = dynamic_cast<const IntegerLiteral*>(node)) {
		visit(int_lit);
	}
	else if (const IdentifierExpr* id_expr = dynamic_cast<const IdentifierExpr*>(node)) {
		visit(id_expr);
	}
	else if (const BinaryExpression* bin_expr = dynamic_cast<const BinaryExpression*>(node)) {
		visit(bin_expr);
	}
	else {
		error("Unknown expression type encountered during code generation.");
	}
}

void CodeGenerator::visit(const IntegerLiteral* node) {
	std::stringstream ss;
	ss << "  push " << node->value; // Push integer literal directly onto stack
	emit(ss.str());
}

void CodeGenerator::visit(const IdentifierExpr* node) {
	std::string var_name = node->name;
	Symbol* sym = getSymbol(var_name);
	if (sym) {
		// Load the variable's value from its memory location onto the stack
		std::stringstream ss;
		ss << "  push QWORD PTR [rbp - " << sym->stackOffset << "]";
		emit(ss.str());
	}
	else {
		error("Undeclared variable: " + var_name + " used.");
		emit("  push 0");
	}
}

void CodeGenerator::visit(const BinaryExpression* node) {
	// 1. Evaluate right operand, push result
	visit(node->right.get());

	// 2. Evaluate left operand, push result
	visit(node->left.get());
	pop_into_rax(); // RAX now has left_result
	emit("  pop rbx"); // RBX now has right_result

	switch (node->op) {
	case PLUS:
		emit("  add rax, rbx"); // RAX = RAX + RBX (left + right)
		break;
	case MINUS:
		emit("  sub rax, rbx"); // RAX = RAX - RBX (left - right)
		break;
	case ASTERISK:
		// Imulq multiplies RAX by operand and stores result in RAX/RDX
		emit("  imul rax, rbx"); // RAX = RAX * RBX (left * right)
		break;
	case SLASH:
		// Integer division: idivq divides RDX:RAX by operand
		// Result (quotient) in RAX, remainder in RDX
		// Clear RDX before division for single operand division
		emit("  xor rdx, rdx"); // Clear RDX
		emit("  idiv rbx");     // RAX = RAX / RBX (left / right)
		// Make sure RAX (dividend) is sign-extended into RDX if needed
		// For positive numbers, xor rdx, rdx is fine.
		// For full sign extension: cqo (convert quad to octal) - uses %rax to %rdx
		break;
	default:
		error("Unsupported binary operator: " + tokenTypeStrings.at(node->op));
		break;
	}

	// Push the result of the operation back onto the stack
	push_from_rax();
}

void CodeGenerator::defineVariable(const std::string& name) {
	if (symbolTable_.count(name) > 0) {
		error("Variable '" + name + "' already declared. (Re-assignment is fine, but re-definition isn't explicit here)");
		return;
	}
	stackOffsetCounter_ += WORD_SIZE; // Allocate 8 bytes for a QWORD (64-bit)
	symbolTable_[name] = { stackOffsetCounter_ }; // Store the offset
}

Symbol* CodeGenerator::getSymbol(const std::string& name) {
	auto it = symbolTable_.find(name);
	if (it != symbolTable_.end()) {
		return &it->second;
	}
	return nullptr;
}

void CodeGenerator::pop_into_rax() {
	emit("  pop rax");
}

void CodeGenerator::push_from_rax() {
	emit("  push rax");
}