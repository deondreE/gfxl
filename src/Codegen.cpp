// Codegen.cpp
#include "Codegen.h"
#include <iostream> // For error messages or debug output
#include <stdexcept> // For std::runtime_error
#include <random>    // For std::mt19937_64 and std::uniform_int_distribution
#include <chrono>    // For seeding the random number generator
#include <string>
#include <map>

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
        // Note: Linux x64 ABI requires RSP to be 16-byte aligned BEFORE a call.
        // `call` pushes 8 bytes, so if RSP is currently 16-byte aligned,
        // after push it becomes 8-byte aligned. The function called must handle alignment.
        // For simplicity, we rely on C library functions often handling this or adjust later if needed.
        // Local variables will be allocated via `sub rsp`.
    }
    else if (targetPlatform_ == PLATFORM_WINDOWS_MINGW) {
        ss << ".intel_syntax noprefix\n"; // Using Intel syntax
        ss << ".globl main\n";           // MinGW usually uses `main`
        ss << ".text\n";
        ss << "main:\n";
        emit("push rbp");               // Save base pointer
        emit("mov rbp, rsp");           // Set new base pointer
        // Windows x64 calling convention: reserve 32 bytes of "shadow space" for caller-saved regs
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
        // We allocated space using `sub rsp`, so we add it back.
        // `stackOffsetCounter_` is negative, so we add its absolute value.
        if (stackOffsetCounter_ < 0) {
            emit("add rsp, " + std::to_string(-stackOffsetCounter_));
        }
        emit("mov rsp, rbp");           // Restore stack pointer to RBP's value
        emit("pop rbp");                // Restore base pointer
        emit("mov eax, 0");             // Standard return code 0 for success in EAX/RAX
        emit("ret");
    }
    else if (targetPlatform_ == PLATFORM_WINDOWS_MINGW) {
        emitComment("Main Epilogue");
        // Deallocate local variables
        if (stackOffsetCounter_ < 0) {
            emit("add rsp, " + std::to_string(-stackOffsetCounter_));
        }
        emit("add rsp, 32");            // Clean up shadow space
        emit("mov rsp, rbp");           // Restore stack pointer to RBP's value
        emit("pop rbp");                // Restore base pointer
        emit("mov eax, 0");             // Return 0 (success)
        emit("ret");
    }
    else {
        error("Codegen Finalize: Cannot emit epilogue for unknown platform.");
    }
}

void CodeGenerator::emitPrintInteger(const std::string& valueReg) {
    emitComment("Call print_int");
    // print_int typically expects the integer value in the first argument register.
    // For Linux/macOS, this is RDI. For Windows, it's RCX.
    std::string argReg = getArgRegister(0);
    emit("mov " + argReg + ", " + getRegisterPart(INT, valueReg)); // Move value to appropriate part of arg register

    // Call the helper function
    if (targetPlatform_ == PLATFORM_MACOS) { // macOS often prepends '_' to C function names
        emit("call _print_int");
    }
    else {
        emit("call print_int");
    }
}

std::string CodeGenerator::getRegisterPart(TokenType type, const std::string& baseReg) const {
    if (baseReg.empty()) return ""; // Should not happen

    if (type == BOOL) {
        if (baseReg.length() > 1 && baseReg[0] == 'r') {
            return baseReg[0] + std::string("l") + baseReg.substr(1); // e.g., "rax" -> "al"
        }
        // Handle cases where baseReg might already be a smaller register (less likely with our setup)
        if (baseReg.length() == 1) return baseReg; // e.g. "a" -> "a" (assuming it's AL)
        return ""; // Fallback
    }

    // For INT, we use the full register (e.g., RAX)
    return baseReg;
}

void CodeGenerator::emitPrintBoolean(const std::string& valueReg) {
    emitComment("Call print_bool");
    // print_bool expects a boolean (0 or 1), usually passed as a byte.
    // We need to get the specific byte register (e.g., 'al' from 'rax').
    std::string argReg = getArgRegister(0);
    emit("mov " + argReg + ", " + getRegisterPart(BOOL, valueReg)); // Move the byte value to arg register

    if (targetPlatform_ == PLATFORM_MACOS) {
        emit("call _print_bool");
    }
    else {
        emit("call print_bool");
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

    // 1. Generate code for the right-hand side expression.
    // The result will be in RAX (or AL zero-extended to RAX).
    visitExpression(node->value.get());
    TokenType valueType = node->value->resolvedType;

    // 2. Ensure variable is defined in our codegen symbol table and on the stack.
    CodegenSymbol* symbol = getSymbol(node->identifier->name);
    if (!symbol) {
        // This is the first time we're seeing this variable in codegen.
        // Define it on the stack. Semantic analysis should have guaranteed it's valid.
        defineVariable(node->identifier->name, valueType); // This also updates stackOffsetCounter_
        symbol = getSymbol(node->identifier->name);       // Get the newly defined symbol
    }
    else {
        // If it's already defined, ensure its type matches (though sema should check this).
        // For simplicity, we allow re-assignment to potentially different types if the language supported it,
        // but here we'll assume types are static and should match or be compatible.
        // We don't need to re-allocate stack space.
    }

    if (!symbol) { // Defensive check
        error("Internal Codegen Error: Failed to get symbol for '" + node->identifier->name + "' after definition.");
        return;
    }

    // 3. Store the value from RAX/AL into the variable's stack location.
    // Use appropriate register part and memory size.
    emit("mov " + getRegSize(valueType) + " ptr [rbp" + std::to_string(symbol->stackOffset) + "], " + getRegisterPart(valueType, "rax"));
}

void CodeGenerator::visitExpressionStatement(const ExpressionStatement* node) {
    emitComment("Expression Statement");
    visitExpression(node->expression.get()); // Evaluate the expression, result left in RAX/AL
    // The result is discarded as it's a bare expression statement.
}

void CodeGenerator::visitPrintStatement(const PrintStatement* node) {
    emitComment("Print Statement");
    // 1. Generate code to evaluate the expression to be printed.
    // The result will be in RAX (or AL zero-extended to RAX).
    visitExpression(node->expression.get());

    TokenType exprType = node->expression->resolvedType;

    // 2. Call the appropriate print helper based on the expression's resolved type.
    if (exprType == INT) {
        emitPrintInteger("rax"); // Pass the integer value in RAX
    }
    else if (exprType == BOOL) {
        emitPrintBoolean("rax"); // Pass the boolean value (in AL, zero-extended to RAX)
    }
    else {
        error("Attempting to print an unsupported type (TokenType: " + tokenTypeStrings.at(exprType) + ").");
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
    // Load 1 for true, 0 for false into AL, then zero-extend to RAX for consistency.
    emit("mov al, " + std::to_string(node->value ? 1 : 0));
    emit("movzx rax, al"); // Zero-extend AL to RAX
}

void CodeGenerator::visitIdentifierExpr(const IdentifierExpr* node) {
    emitComment("Identifier: " + node->name);
    CodegenSymbol* symbol = getSymbol(node->name);
    if (!symbol) {
        // This indicates a serious semantic analysis failure if not caught earlier.
        error("Codegen Error: Undefined variable used '" + node->name + "'.");
        return;
    }

    // Load the value from the variable's stack location into RAX/AL based on its type.
    emit("mov " + getRegSize(symbol->type) + " ptr [rbp" + std::to_string(symbol->stackOffset) + "], " + getRegisterPart(symbol->type, "rax"));
}

void CodeGenerator::visitBinaryExpression(const BinaryExpression* node) {
    emitComment("Binary Expression: " + tokenTypeStrings.at(node->op));

    // Evaluate right operand first, its result will be in RAX (or AL zero-extended)
    visitExpression(node->right.get());
    TokenType rightType = node->right->resolvedType;

    // Push the right operand's value onto the stack to preserve it
    emit("push rax");
    stackOffsetCounter_ += 8; // Account for the push on the stack

    // Evaluate left operand, its result will be in RAX (or AL zero-extended)
    visitExpression(node->left.get());
    TokenType leftType = node->left->resolvedType;

    // Pop the right operand into RBX (or BL for boolean operations)
    emit("pop rbx");
    stackOffsetCounter_ -= 8; // Account for the pop

    // Determine the correct register parts for operation based on type
    std::string leftReg = getRegisterPart(leftType, "rax");
    std::string rightReg = getRegisterPart(rightType, "rbx");
    TokenType resultType = node->resolvedType; // This is the type of the expression's result

    // Perform the operation. The result is expected to be in RAX (or AL zero-extended).
    switch (node->op) {
    case PLUS:
        emit("add " + getRegisterPart(INT, "rax") + ", " + getRegisterPart(INT, "rbx"));
        break;
    case MINUS:
        emit("sub " + getRegisterPart(INT, "rax") + ", " + getRegisterPart(INT, "rbx"));
        break;
    case ASTERISK:
        // For signed multiplication, IMUL is used.
        // `imul rbx` will multiply RAX by RBX, result in RAX.
        emit("imul " + getRegisterPart(INT, "rbx"));
        break;
    case SLASH:
        // For signed division: CQO extends RAX into RDX:RAX.
        // Then RDX:RAX is divided by the operand (RBX).
        // Quotient goes to RAX, remainder to RDX.
        emit("cqo"); // Sign-extend RAX into RDX:RAX
        emit("idiv " + getRegisterPart(INT, "rbx")); // Divide RDX:RAX by RBX
        break;
    default:
        error("Unhandled binary operator in code generation: " + tokenTypeStrings.at(node->op));
        break;
    }
    // The result of the operation is now in RAX (or AL zero-extended to RAX if applicable).
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

std::string CodeGenerator::getArgRegister(int argIndex) const {
    if (targetPlatform_ == PLATFORM_LINUX || targetPlatform_ == PLATFORM_MACOS) {
        // Linux/macOS x64 ABI: RDI, RSI, RDX, RCX, R8, R9
        if (argIndex == 0) return "rdi";
        if (argIndex == 1) return "rsi";
        if (argIndex == 2) return "rdx";
        if (argIndex == 3) return "rcx";
        if (argIndex == 4) return "r8";
        if (argIndex == 5) return "r9";
    }
    else if (targetPlatform_ == PLATFORM_WINDOWS_MINGW) {
        // Windows x64 ABI: RCX, RDX, R8, R9
        if (argIndex == 0) return "rcx";
        if (argIndex == 1) return "rdx";
        if (argIndex == 2) return "r8";
        if (argIndex == 3) return "r9";
    }
    // Fallback if we need more registers or unsupported platform
    return "";
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