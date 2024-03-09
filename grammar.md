# Grammar 

```
<program> ::= <header> <code>

<header> ::= <var_decl>*
<var_decl> ::= "DEF" <identifier>

<code> ::= (<label> | <statement>)*
<inner_code> ::= <statement>?
               | "RETURN"
<label> ::= ":" <identifier>
<statement> ::= "IF" <expr> "THEN" <inner_code>* "FI"
          | <func_call>
          | <assign>
          | "GOTO" <identifier>
<func_call> ::= "CALL" <func_name> "()" ("WITH" <inner_code>* "END")?
<expr> ::= <unary> <op> <unary>
<unary> ::= <identifier>
      | <number>
      | "(" <expr> ")"

<assign> ::= <identifier> "=" <unary>

<number> ::= <digit>+ ("." <digit>+)?
<digit> ::= [0-9]
<op> ::= "<" | ">" | "<=" | ">=" | "=="

<func_name> ::= <identifier>
<identifier> ::= ([a-z] | [A-Z]) ([a-z] | [A-Z] | [0-9])*

newlines and spaces are considered whitespace and are used to seperate tokens
```

This ought to be enough for our simple language. Example that it should parse

```
DEF var1
DEF var2

: stage1
CALL runStage1 WITH
  IF PRESSURE > 8 THEN
    RETURN
  FI
END
```
