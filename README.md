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


## Semantic

```
/* Declare variable called `old_pressure` All variables are floats */
DEF old_pressure

/* An external variable that has a deeper meaning for the interpreter.
 * Note that these must be declared before use. They can otherwise be used
 * like normally declared variables.
 */
EXTERN PRESSURE

/* A simple assign expression */
var = 2.0
var = PRESSURE
var = ( var + 1 )

/* A simple if clause similar to other languages */
IF var < 2 THEN
  /* some code */
FI

/* This is a label definition.
 * These can only exit at the top level and are not able to jump into IF clauses or similar.
 * 
 * NOTE: There's a whitespace there
 */
: label

/* goto to the label. One can exit function calls or if clauses with these */
GOTO label

/* A simple function call. 
 * As of rn one cannot define these, but instead the interpreter will call the appropriate
 * function as mentioned in their lookup table. Stages, button presses, notification and more 
 * can be easily realized using these.
 */
CALL my_func

/* For functions that run in a loop (and support this), one can use the WITH ... END
 * keywords to execute code after each loop iteration.
 * This can for example be used to defining exit conditions, tracking variables and more.
 */
CALL my_stage WITH
  /* some code */
END
```


## TODO

- currently no function calls are supported
- implement parameters for function calls
- extern variable definitions
