# GFXL

Rethinking low level access in the graphics space from the ground up, making it more simple and approachable.

## Building
1. Install premake
2. `premake5 gmake2`

## Grammer

```ebnf
# === Lexical tokens ===
identifier     = letter , { letter | digit | "_" } ;
letter         = "a" … "z" | "A" … "Z" | "_" ;
digit          = "0" … "9" ;

intLiteral     = digit , { digit } ;
floatLiteral   = digit , { digit } , "." , digit , { digit } ;
stringLiteral  = '"' , { ? any character except '"' ? } , '"' ;

comment        = "//" , { ? any character except newline ? } ;
blockComment   = "/*" , { ? any sequence not containing "*/" ? } , "*/" ;

keyword        = "if"
               | "assert"
               | "return"
               | "while"
               | "do"
               | "module"
               | "import"
               | "struct"
               | "enum"
               | "var"
               | "const"
               | "defer"
               | "for"
               | "else"
               | "func"
               | "shared"
               | "export"
               | "extern"
               ;

# === Types ===
Type            = SharedType
                | PointerType
                | ArrayListType
                | BasicType
                | FunctionType ;

SharedType      = "shared" , "<" , Type , ">" ;
PointerType     = "*" , Type ;
ArrayListType   = "[]" , Type ;
BasicType       = identifier ;
FunctionType    = "fn" , "(" , [ ParamList ] , ")" , [ ":" , Type ] ;

# === Expressions ===
Expr            = AssignExpr ;

AssignExpr      = ConditionalExpr , [ AssignOp , Expr ] ;
AssignOp        = "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;

ConditionalExpr = LogicOrExpr , [ "?" , Expr , ":" , Expr ] ;

LogicOrExpr     = LogicAndExpr , { "||" , LogicAndExpr } ;
LogicAndExpr    = EqualityExpr , { "&&" , EqualityExpr } ;
EqualityExpr    = RelationalExpr , { ( "==" | "!=" ) , RelationalExpr } ;
RelationalExpr  = ShiftExpr , { ( "<" | ">" | "<=" | ">=" ) , ShiftExpr } ;
ShiftExpr       = AddExpr , { ( "<<" | ">>" ) , AddExpr } ;
AddExpr         = MulExpr , { ( "+" | "-" ) , MulExpr } ;
MulExpr         = UnaryExpr , { ( "*" | "/" | "%" ) , UnaryExpr } ;

UnaryExpr       = [ UnaryOp ] , PrimaryExpr ;
UnaryOp         = "-" | "!" | "*" | "&" ;

PrimaryExpr     = Literal
                | identifier
                | "(" , Expr , ")"
                | "makeShared" , "(" , Expr , ")"
                | CallExpr
                | IndexExpr
                | FieldExpr ;

CallExpr        = PrimaryExpr , "(" , [ ArgList ] , ")" ;
ArgList         = Expr , { "," , Expr } ;

IndexExpr       = PrimaryExpr , "[" , Expr , "]" ;
FieldExpr       = PrimaryExpr , "." , identifier ;

Literal         = intLiteral
                | floatLiteral
                | stringLiteral
                | "true" | "false" | "null" | "error" ;

# === Statements ===
Stmt            = VarDecl
                | ExprStmt
                | IfStmt
                | WhileStmt
                | DoWhileStmt
                | ForStmt
                | ReturnStmt
                | DeferStmt
                | AssertStmt
                | Block
                | ";" ;  # empty statement

VarDecl         = ( "var" | "const" ) , identifier , [ ":" , Type ] , [ "=" , Expr ] , ";" ;

ExprStmt        = Expr , ";" ;

IfStmt          = "if" , ( Capture | "(" , Expr , ")" ) , Block , [ "else" , ( IfStmt | Block ) ] ;
Capture         = "var" , identifier , "=" , Expr ;

WhileStmt       = "while" , "(" , Expr , ")" , Block ;

DoWhileStmt     = "do" , Block , "while" , "(" , Expr , ")" , ";" ;

ForStmt         = "for" , ( Capture | "(" , [ Expr? ] , ";" , [ Expr? ] , ";" , [ Expr? ] , ")" ) , Block ;

ReturnStmt      = "return" , [ Expr ] , ";" ;

DeferStmt       = "defer" , Expr , ";" ;

AssertStmt      = "assert" , "(" , Expr , ")" , ";" ;

Block           = "{" , { Stmt } , "}" ;

# === Function and Module Declarations ===
FuncDecl        = [ "export" | "extern" ] , "fn" , identifier ,
                  "(" , [ ParamList ] , ")" ,
                  [ ":" , Type ] ,
                  ( Block | ";" ) ;

ParamList       = Param , { "," , Param } ;
Param           = identifier , ":" , Type ;

ModuleDecl      = "module" , identifier , ";" ;

ImportDecl      = "import" , stringLiteral , ";" ;

# === Struct and Enum Declarations ===
StructDecl      = "struct" , identifier , StructBody ;
StructBody      = "{" , { VarDecl } , "}" ;

EnumDecl        = "enum" , identifier , "{" , EnumList , "}" ;
EnumList        = identifier , [ "=" , intLiteral ] , { "," , identifier , [ "=" , intLiteral ] } ;

# === TopLevel Plus Global Decl ====
GlobalVarDecl  = identifier , ":=" , Expr , ";" ;
GlobalAssign   = identifier , "=" , Expr , ";" ;

TopLevelDecl   = GlobalVarDecl
               | GlobalAssign
               | FuncDecl
               | StructDecl
               | EnumDecl
               | ImportDecl
               | ModuleDecl ;
Program = { TopLevelDecl } ;

```

