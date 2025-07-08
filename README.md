# GFXL

Rethinking low level access in the graphics space from the ground up, making it more simple and approachable.

## Building
1. Install premake
2. `premake5 gmake2`

## Grammer

```ebnf
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
               | "do" ;
```

