# GFXL

Rethinking low level access in the graphics space from the ground up, making it more simple and approachable.

## Building
1. Install premake
2. `premake5 gmake2`

## Grammer

identifier     = letter , { letter | digit | "_" } ;
letter         = "a" … "z" | "A" … "Z" | "_" ;
digit          = "0" … "9" ;

intLiteral     = digit , { digit } ;
floatLiteral   = digit , { digit } , "." , digit , { digit } ;
stringLiteral  = '"' , { ? any char except '"' ? } , '"' ;

comment        = "//"
blockComment   = "/*" , { ? not "*/" ? } , "*/" ;
nestedBlockComment = "/* /* */"
AssignOp :: = '=' | ':=' | '-=' | '*=' | '/='

Keywords:
    const func struct if else elif for ret break continue defer

Operators:
    = == != < > <= >= + - / % && || ! & | << >> += -= /= ++ -- ->


