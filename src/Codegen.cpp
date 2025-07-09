// Codegen.cpp
#include "Codegen.h"
#include <iostream> // For error messages or debug output
#include <stdexcept> // For std::runtime_error
#include <random>    // For std::mt19937_64 and std::uniform_int_distribution
#include <chrono>    // For seeding the random number generator

// External declaration for tokenTypeStrings (defined in Token.cpp)
extern const std::map<TokenType, std::string> tokenTypeStrings;

// --- Static Members and Global Helpers ---
// For generating unique labels in assembly
static std::mt19937_64 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
static std::uniform_int_distribution<long long> dist; // Using default range for simplicity
static long long labelCounter = 0; // Better for sequential labels

std::string generateUniqueLabel(const std::string& prefix) {
    return prefix + std::to_string(labelCounter++);
}

// --- CodeGenerator Implementation ---

CodeGenerator::CodeGenerator() : stackOffsetCounter_(0), targetPlatform_(PLATFORM_UNKNOWN) {
    // Detect platform at compiler's compile-time
#if defined(_WIN32) || defined(_WIN64)
    targetPlatform_ = PLATFORM_WINDOWS_MINGW; // Assume MinGW for simplicity
#elif defined(__linux__)
    targetPlatform_ = PLATFORM_LINUX;
#elif defined(__APPLE__) && defined(__MACH__)
    targetPlatform_ = PLATFORM_MACOS;
#else
    error("Codegen Init: Unsupported host platform detected.");
#endif
}

std::string CodeGenerator::generate(const Program* program_ast) {
    if (!program_ast) {
        error("Code generation received a null AST program.");
        return "";
    }

    // Emit platform-specific boilerplate prologue
    emitMainPrologue();

    // Traverse the AST to generate code
    visitProgram(program_ast);

    // Emit platform-specific boilerplate epilogue
    emitMainEpilogue();

    return ss.str();
}

std::vector<std::string> CodeGenerator::getErrors() const {
    return errors_;
}

void CodeGenerator::error(const std::string& msg) {
    errors_.push_back(msg);
}

void CodeGenerator::emit(const std::string& instruction) {
    ss << "  " << instruction << "\n";
}

void CodeGenerator::emitComment(const std::string& comment) {
    // Use '#' for GNU AS (Linux/MinGW) or ';' for NASM/MASM (MSVC)
    if (targetPlatform_ == PLATFORM_LINUX || targetPlatform_ == PLATFORM_WINDOWS_MINGW || targetPlatform_ == PLATFORM_MACOS) {
        ss << "  # " << comment << "\n";
    }
    // Add else if for other assemblers if you support them.
}

// --- Platform-Specific Assembly Boilerplate ---
void CodeGenerator::emitMainPrologue() {
    if (targetPlatform_ == PLATFORM_LINUX || targetPlatform_ == PLATFORM_MACOS) {
        ss << ".intel_syntax noprefix\n"; // Using Intel syntax
        ss << ".globl main\n";           // Standard entry point for Linux/macOS
        ss << ".text\n";
        ss << "main:\n";
        emit("push rbp");               // Save base pointer
        emit("mov rbp, rsp");           // Set new base pointer
        // Stack alignment for Linux x64: RSP must be 16-byte aligned before a call instruction.
        // `call` pushes 8 bytes, so RSP becomes `N-8`. If N was 16-aligned, N-8 is not.
        // If stackOffsetCounter_ indicates we'll push an odd number of qwords later,
        // we might need an initial push or sub rsp here.
        // For simplicity, we'll assume stack management is handled by `sub rsp` only when defining variables.
    }
    else if (targetPlatform_ == PLATFORM_WINDOWS_MINGW) {
        ss << ".intel_syntax noprefix\n"; // Using Intel syntax
        ss << ".globl main\n";           // MinGW usually uses `main`
        ss << ".text\n";
        ss << "main:\n";
        emit("push rbp");               // Save base pointer
        emit("mov rbp, rsp");           // Set new base pointer
        // Windows x64 calling convention: reserve 32 bytes of "shadow space"
        emit("sub rsp, 32");
    }
    else {
        error("Codegen Init: Cannot emit prologue for unknown platform.");
    }
}

void CodeGenerator::emitMainEpilogue() {
    if (targetPlatform_ == PLATFORM_LINUX || targetPlatform_ == PLATFORM_MACOS) {
        emitComment("Main Epilogue");
        // Deallocate local variables by restoring RSP to RBP's original value
        if (stackOffsetCounter_ < 0) { // If variables were allocated
            emit("add rsp, " + std::to_string(-stackOffsetCounter_)); // Deallocate all local vars
        }
        emit("mov rsp, rbp");           // Restore stack pointer
        emit("pop rbp");                // Restore base pointer
        emit("mov eax, 0");             // Return 0 (success)
        emit("ret");
    }
    else if (targetPlatform_ == PLATFORM_WINDOWS_MINGW) {
        emitComment("Main Epilogue");
        // Deallocate local variables
        if (stackOffsetCounter_ < 0) {
            emit("add rsp, " + std::to_string(-stackOffsetCounter_));
        }
        emit("add rsp, 32");            // Clean up shadow space
        emit("mov rsp, rbp");           // Restore stack pointer
        emit("pop rbp");                // Restore base pointer
        emit("mov eax, 0");             // Return 0 (success)
        emit("ret");
    }
    else {
        error("Codegen Finalize: Cannot emit epilogue for unknown platform.");
    }
}

void CodeGenerator::emitPrintInteger(const std::string& reg) { // reg holds the int value (e.g., "rax")
    emitComment("Call print_int");
    if (targetPlatform_ == PLATFORM_LINUX) {
        emit("mov rdi, " + reg); // First arg in RDI for Linux x64
        // If current stack frame is not 16-byte aligned before the call,
        // we might need to `sub rsp, 8` before the call and `add rsp, 8` after.
        // A simpler way: push RDI, RSI, RDX, RCX, R8, R9 before ANY function call
        // if they are volatile (caller-saved) registers and you need their values.
        // For simplicity now, we assume `print_int` only uses RDI.
        emit("call print_int");
    }
    else if (targetPlatform_ == PLATFORM_WINDOWS_MINGW) {
        emit("mov rcx, " + reg); // First arg in RCX for Windows x64
        // Windows calling convention requires caller to reserve 32 bytes of "shadow space"
        // This is done in the prologue `sub rsp, 32` already.
        // Need to save XMM registers if using float arguments (not applicable here).
        emit("call print_int");
    }
    else if (targetPlatform_ == PLATFORM_MACOS) {
        emit("mov rdi, " + reg); // First arg in RDI for macOS x64 (similar to Linux)
        emit("call _print_int"); // macOS often prepends '_' to C function names
    }
    else {
        error("Codegen PrintInteger: Unsupported platform for print_int.");
    }
}

void CodeGenerator::emitPrintBoolean(const std::string& reg) { // reg holds the boolean value (e.g., "al")
    emitComment("Call print_bool");
    // The `print_bool` helper function (in print_helpers.c) is assumed to take a `bool`
    // which in C is often 1 byte, so we can pass 'al' directly.
    if (targetPlatform_ == PLATFORM_LINUX) {
        emit("mov rdi, " + reg); // First arg in RDI
        emit("call print_bool");
    }
    else if (targetPlatform_ == PLATFORM_WINDOWS_MINGW) {
        emit("mov rcx, " + reg); // First arg in RCX
        emit("call print_bool");
    }
    else if (targetPlatform_ == PLATFORM_MACOS) {
        emit("mov rdi, " + reg); // First arg in RDI
        emit("call _print_bool"); // macOS often prepends '_'
    }
    else {
        error("Codegen PrintBoolean: Unsupported platform for print_bool.");
    }
}

// --- AST Node Dispatchers & Specific Code Generation Functions ---

void CodeGenerator::visitProgram(const Program* node) {
    for (const auto& stmt : node->statements) {
        visitStatement(stmt.get()); // Use .get() to get raw pointer from unique_ptr
    }
}

void CodeGenerator::visitStatement(const Statement* node) {
    if (const AssignmentStatement* assign = dynamic_cast<const AssignmentStatement*>(node)) {
        visitAssignmentStatement(assign);
    }
    else if (const ExpressionStatement* expr = dynamic_cast<const ExpressionStatement*>(node)) {
        visitExpressionStatement(expr);
    }
    else if (const PrintStatement* print = dynamic_cast<const PrintStatement*>(node)) {
        visitPrintStatement(print);
    }
    else {
        error("Unhandled statement type in codegen dispatcher.");
    }
}

void CodeGenerator::visitAssignmentStatement(const AssignmentStatement* node) {
    emitComment("Assignment: " + node->identifier->name);

    // 1. Generate code for the right-hand side expression
    visitExpression(node->value.get()); // Result of expression is in RAX/AL

    // Get the resolved type of the value (from semantic analysis)
    TokenType valueType = node->value->resolvedType;
    std::string reg_to_move = getRegName(valueType, "rax");

    // 2. Define or retrieve the variable's stack location
    CodegenSymbol* symbol = getSymbol(node->identifier->name);
    if (!symbol) { // If variable not yet defined in codegen's symbol table
        // This means it's the first assignment to a new variable.
        // Semantic analyzer should have caught undefined uses before this.
        defineVariable(node->identifier->name, valueType);
        symbol = getSymbol(node->identifier->name); // Retrieve the newly defined symbol
    }

    if (!symbol) { // Should not happen if defineVariable works correctly
        error("Internal Codegen Error: Could not define/retrieve symbol for '" + node->identifier->name + "'.");
        return;
    }

    // 3. Store the value from RAX/AL into the variable's stack location
    emit("mov " + getRegSize(valueType) + " ptr [rbp" + std::to_string(symbol->stackOffset) + "], " + reg_to_move);
}

void CodeGenerator::visitExpressionStatement(const ExpressionStatement* node) {
    emitComment("Expression Statement");
    visitExpression(node->expression.get()); // Evaluate the expression, result left in RAX/AL
    // The result is discarded as it's a bare expression statement.
}

void CodeGenerator::visitPrintStatement(const PrintStatement* node) {
    emitComment("Print Statement");
    visitExpression(node->expression.get()); // Evaluate expression to print (result in RAX/AL)

    TokenType exprType = node->expression->resolvedType;

    if (exprType == INT) {
        emitPrintInteger("rax");
    }
    else if (exprType == BOOL) {
        emitPrintBoolean("al"); // Use AL as it contains the 0/1 boolean value
    }
    else {
        error("Codegen Error: Attempting to print an unsupported type (TokenType: " + tokenTypeStrings.at(exprType) + ").");
    }
}

void CodeGenerator::visitExpression(const Expression* node) {
    if (const IntegerLiteral* int_lit = dynamic_cast<const IntegerLiteral*>(node)) {
        visitIntegerLiteral(int_lit);
    }
    else if (const BooleanLiteral* bool_lit = dynamic_cast<const BooleanLiteral*>(node)) {
        visitBooleanLiteral(bool_lit);
    }
    else if (const IdentifierExpr* id_expr = dynamic_cast<const IdentifierExpr*>(node)) {
        visitIdentifierExpr(id_expr);
    }
    else if (const BinaryExpression* bin_expr = dynamic_cast<const BinaryExpression*>(node)) {
        visitBinaryExpression(bin_expr);
    }
    else {
        error("Unhandled expression type in codegen dispatcher.");
    }
}

void CodeGenerator::visitIntegerLiteral(const IntegerLiteral* node) {
    emitComment("Integer Literal: " + std::to_string(node->value));
    emit("mov rax, " + std::to_string(node->value)); // Load integer into RAX
}

void CodeGenerator::visitBooleanLiteral(const BooleanLiteral* node) {
    emitComment("Boolean Literal: " + node->value ? "true" : "false");
    emit("mov al, " + std::to_string(node->value ? 1 : 0)); // Load 1 for true, 0 for false into AL
    emit("movzx rax, al"); // Zero-extend AL to RAX (for consistency in expressions expecting 64-bit)
}

void CodeGenerator::visitIdentifierExpr(const IdentifierExpr* node) {
    emitComment("Identifier: " + node->name);
    CodegenSymbol* symbol = getSymbol(node->name);
    if (!symbol) {
        error("Codegen Error: Undefined variable used '" + node->name + "'. (Should be caught by semantic analysis)");
        return;
    }
    // Load the value from stack into RAX/AL based on the symbol's type
    emit("mov " + getRegName(symbol->type, "rax") + ", " + getRegSize(symbol->type) + " ptr [rbp" + std::to_string(symbol->stackOffset) + "]");
}

void CodeGenerator::visitBinaryExpression(const BinaryExpression* node) {
    emitComment("Binary Expression: " + tokenTypeStrings.at(node->op));

    // Evaluate right operand first
    visitExpression(node->right.get()); // Result in RAX/AL
    emit("push rax");                   // Push RAX (or AL zero-extended to RAX) to stack

    // Evaluate left operand
    visitExpression(node->left.get());  // Result in RAX/AL

    // Pop right operand into RBX/BL
    emit("pop rbx");

    TokenType leftType = node->left->resolvedType;
    TokenType rightType = node->right->resolvedType;
    TokenType resultType = node->resolvedType; // Type of the result of this expression

    std::string opRegL_name = getRegName(leftType, "rax");
    std::string opRegR_name = getRegName(rightType, "rbx");
    std::string resultReg_name = getRegName(resultType, "rax"); // Final result should be in appropriate part of RAX

    // All arithmetic operators are assumed to operate on INT, and result in INT
    switch (node->op) {
    case PLUS:
        emit("add " + opRegL_name + ", " + opRegR_name);
        break;
    case MINUS:
        emit("sub " + opRegL_name + ", " + opRegR_name);
        break;
    case ASTERISK:
        // imul can take one operand (multiplies RAX by operand)
        emit("imul " + opRegR_name);
        break;
    case SLASH:
        // For signed division: CQO extends RAX into RDX:RAX
        emit("cqo");
        emit("idiv " + opRegR_name); // Divides RDX:RAX by RBX, quotient in RAX, remainder in RDX
        break;
    default:
        error("Codegen Error: Unhandled binary operator in code generation: " + tokenTypeStrings.at(node->op));
        break;
    }
    // Result of the operation is already in RAX (or AL extended to RAX), ready for next step.
}

// --- Symbol Table Management for CodeGen ---

void CodeGenerator::defineVariable(const std::string& name, TokenType type) {
    if (symbolTable_.count(name)) {
        // This case should ideally be caught by semantic analysis.
        error("Internal Codegen Error: Variable '" + name + "' redefined in codegen symbol table.");
        return;
    }
    // Allocate 8 bytes for a variable, regardless of its logical size (byte for bool, qword for int).
    // This simplifies stack offsets, ensuring all variable slots are 8 bytes,
    // which is also typically good for alignment.
    stackOffsetCounter_ -= 8;
    symbolTable_[name] = { stackOffsetCounter_, type };
    emit("sub rsp, 8"); // Allocate space on the stack for the new variable
}

CodegenSymbol* CodeGenerator::getSymbol(const std::string& name) {
    auto it = symbolTable_.find(name);
    if (it != symbolTable_.end()) {
        return &it->second;
    }
    return nullptr; // Not found
}

// --- Assembly Register & Size Utilities ---

std::string CodeGenerator::getRegSize(TokenType type) const {
    if (type == INT) return "qword"; // 64-bit
    if (type == BOOL) return "byte"; // 8-bit
    return "qword"; // Default or error case for safety
}

std::string CodeGenerator::getRegName(TokenType type, const std::string& baseReg) const {
    // Determine the correct register name based on type and desired base register
    // E.g., if baseReg is "rax" and type is BOOL, return "al"
    // If baseReg is "rdi" and type is BOOL, return "dil" (Linux) or "cl" (Windows)

    if (baseReg == "rax") {
        if (type == INT) return "rax";
        if (type == BOOL) return "al";
    }
    if (baseReg == "rbx") { // Used for second operand in binary ops
        if (type == INT) return "rbx";
        if (type == BOOL) return "bl";
    }
    // For function call arguments (RDI/RSI/RDX/RCX/R8/R9 etc.)
    if (targetPlatform_ == PLATFORM_LINUX || targetPlatform_ == PLATFORM_MACOS) {
        if (baseReg == "rdi") {
            if (type == INT) return "rdi";
            if (type == BOOL) return "dil";
        }
        if (baseReg == "rsi") { // Second arg (not used here yet)
            if (type == INT) return "rsi";
            if (type == BOOL) return "sil";
        }
        // ... add rdx, rcx, r8, r9 if needed
    }
    else if (targetPlatform_ == PLATFORM_WINDOWS_MINGW) {
        if (baseReg == "rcx") { // First arg (Windows x64)
            if (type == INT) return "rcx";
            if (type == BOOL) return "cl";
        }
        if (baseReg == "rdx") { // Second arg (Windows x64)
            if (type == INT) return "rdx";
            if (type == BOOL) return "dl";
        }
        // ... add r8, r9 if needed
    }

    // error("Codegen Error: Unhandled register alias for type " + tokenTypeStrings.at(type) + " with base " + baseReg);
    return baseReg;
}

// --- Stack Manipulation Helpers (Not actively used in this version, but can be added back) ---
// void CodeGenerator::push_value_from_rax(TokenType type) {
//     if (type == INT) { emit("push rax"); }
//     else if (type == BOOL) { emit("push rax"); } // Push 8 bytes, but only AL is relevant
//     stackOffsetCounter_ -= 8;
// }

// void CodeGenerator::pop_value_into_rax(TokenType type) {
//     if (type == INT) { emit("pop rax"); }
//     else if (type == BOOL) { emit("pop rax"); } // Pop 8 bytes, use AL
//     stackOffsetCounter_ += 8;
// }