#include "ast.h"
#include "semantic_analyzer.h"

void Program::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void IntegerLiteral::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void IdentifierExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void BinaryExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void BooleanLiteral::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void ExpressionStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void AssignmentStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void PrintStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void CommentNode::accept(ASTVisitor& visitor) { visitor.visit(*this); }