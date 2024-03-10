# BEANS interpreter

BEANS is a language supposed to run on a coffee machine. 

Is inherits a few things from BASIC, which makes it easy to parse and interpret even on small devices like ESPs.

```
DEF PRESSURE

: stage_preinfusion
CALL run_preinfusion WITH
  IF PRESSURE > 8 THEN
    GOTO END
  FI
END

: end
CALL purge
```

See further examples in the `tests/` folder (files marked with `x` are supposed to fail).

## Grammar

Newlines and spaces are considered white space and are used to separate tokens. Also note here that variable definitions only happen in the header of the file and are not allowed in the rest of the code.

```EBNF
<program> ::= <header> <code>

<header> ::= <var_decl>*
<var_decl> ::= ("DEF" | "EXTERN") <identifier>

<code> ::= (<label> | <statement>)*
<inner_code> ::= <statement>?
               | "RETURN"
<label> ::= ":" <identifier>
<statement> ::= "IF" <expr> "THEN" <inner_code>* "FI"
          | <func_call>
          | <assign>
          | "GOTO" <identifier>
<func_call> ::= "CALL" <func_name> ("WITH" <inner_code>* "END")?
<expr> ::= <unary> <op> <unary>
<unary> ::= <identifier>
      | <number>
      | "(" <expr> ")"

<assign> ::= <identifier> "=" <unary>

<number> ::= <digit>+ ("." <digit>+)?
<digit> ::= [0-9]
<op> ::= "<" | ">" | "<=" | ">=" | "==" | "+" | "-" | "*" | "/"

<func_name> ::= <identifier>
<identifier> ::= ([a-z] | [A-Z]) ([a-z] | [A-Z] | [0-9] | _)*
```

In addition, multi-line comments (and in turn single-line) are supported using the `/* COMMENT */` syntax borrowed from C.

## TODO

- currently no function calls are supported
