# GFXL

Rethinking low level access in the graphics space from the ground up, making it more simple and approachable.

## Building
1. Install premake
2. `premake5 gmake2`

## Grammer

```abnf
program = *( statement / function_definition / module_definition / struct_definition / typedef_definition / comment / asm_block / glsl_block )

statement = ( global_variable_definition / scoped_variable_definition / assignment_statement / print_statement / expression_statement ) ";"

global_variable_definition = identifier ":=" expression
scoped_variable_definition = identifier "=" expression
assignment_statement = identifier "=" expression ; This rule is now for re-assignments or initial assignments in a local scope if not explicitly defined with a type.
print_statement = "print" "(" expression ")"
expression_statement = expression

expression = ( conditional_expression / binary_expression / unary_expression / primary_expression / lambda_expression / reference_expression )

conditional_expression = logical_or_expression [ "?" expression ":" expression ]
binary_expression = ( logical_or_expression / logical_and_expression / equality_expression / relational_expression / additive_expression / multiplicative_expression )
logical_or_expression = logical_and_expression *( "||" logical_and_expression )
logical_and_expression = equality_expression *( "&&" equality_expression )
equality_expression = relational_expression *( ( "==" / "!=" ) relational_expression )
relational_expression = additive_expression *( ( "<" / ">" / "<=" / ">=" ) additive_expression )
additive_expression = multiplicative_expression *( ( "+" / "-" ) multiplicative_expression )
multiplicative_expression = unary_expression *( ( "*" / "/" / "%" ) unary_expression )

unary_expression = ( ( "+" / "-" / "!" / "*" ) unary_expression / call_expression / member_access_expression / primary_expression )
call_expression = identifier "(" [ argument_list ] ")"
argument_list = expression *( "," expression )
member_access_expression = primary_expression "->" identifier

primary_expression = ( literal / identifier / "(" expression ")" )

lambda_expression = "[" "]" "(" [ parameter_list ] ")" "{" *( statement ) "}"
reference_expression = "ref" "<" identifier ">"

literal = ( integer_literal / hexadecimal_literal / octal_literal / float_literal / string_literal / char_literal / boolean_literal / "nullptr" )
integer_literal = DIGIT+
hexadecimal_literal = "0x" ( DIGIT / "A" / "B" / "C" / "D" / "E" / "F" / "a" / "b" / "c" / "d" / "e" / "f" )+
binary_literal = "0" DIGIT* ; Leading zero for octal
float_literal = DIGIT+ "." DIGIT+
string_literal = DQUOTE *( character ) DQUOTE
char_literal = SQUOTE character SQUOTE
boolean_literal = ( "true" / "false" )

type = ( primitive_type / custom_type / pointer_type / const_type )
primitive_type = ( "int" / "float" / "string" / "bool" / "u32" / "u16" / "u8" / "char" )
custom_type = identifier
pointer_type = "*" type
const_type = "const" "<" type ">"

typedef_definition = "typedef" identifier "{" ( identifier ":" type *( "," identifier ":" type ) ) "}"

function_definition = [ "private" ] "fn" identifier "(" [ parameter_list ] ")" [ ":" type ] ( "{" *( statement ) "}" / ";" )
parameter_list = parameter *( "," parameter )
parameter = identifier ":" type

module_definition = "module" DQUOTE identifier DQUOTE "{" *( module_item ) "}"
module_item = ( module_variable_declaration / module_function_declaration )
module_variable_declaration = ( "shared" / "unique" / "move" / "make_shared" / "make_unique" ) "<" type ">" [ identifier ] ";"
module_function_declaration = "fn" identifier "(" [ parameter_list ] ")" [ ":" type ] ( "{" *( statement ) "}" / ";" )

struct_definition = "struct" identifier "{" *( struct_member ) "}"
struct_member = ( struct_variable_declaration / defer_statement / struct_inline_assignment )
struct_variable_declaration = type identifier [ "=" expression ] ";" ; Explicitly typed variable in a struct
defer_statement = "defer" identifier ";"
struct_inline_assignment = identifier ":=" expression ";" ; For internal struct 'global' like variables

identifier = ( ALPHA / "_" ) *( ALPHA / DIGIT / "_" )

comment = ( single_line_comment / multi_line_comment )
single_line_comment = "#" *( character ) EOL
multi_line_comment = "###" *( character ) "###"

asm_block = "asm_" "{" *( character ) "}"
glsl_block = "glsl_" "{" *( character ) "}"

; Basic Lexical Elements
ALPHA = %x41-5A / %x61-7A   ; A-Z / a-z
DIGIT = %x30-39            ; 0-9
DQUOTE = %x22              ; "
SQUOTE = %x27              ; '
EOL = %x0A                 ; Newline
```

