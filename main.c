#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/queue.h>

#define CHECK_WHITESPACE(VAR) (*VAR == '\n' || *VAR == ' ' || *VAR == '\0')
#define IS_BOOL_OP(T) \
  (T == TOKEN_LESS || T == TOKEN_GREATER || T == TOKEN_LEQ || T == TOKEN_GEQ || T == TOKEN_EQ)
#define IS_OPERATOR(T) \
  (IS_BOOL_OP(T) || T == TOKEN_ADD || T == TOKEN_SUB || T == TOKEN_MUL || T == TOKEN_DIV)
#define IS_UNARY(T) ( T == TOKEN_IDENTIFIER || T == TOKEN_DIGIT || T == TOKEN_EXPR_BEGIN)

#define reterr(code, format, ...)             \
  do {                                        \
    printf("ERR: " format "\n" __VA_OPT__(,) __VA_ARGS__); \
    return code;                              \
  } while (0)

#if 0
#define trace_asm(format, ...) \
  printf("INST: " format "\n" __VA_OPT__(,) __VA_ARGS__)
#else
#define trace_asm(format, ...) 
#endif

enum Tokens {
  TOKEN_DEF = 0,  // DEF
  TOKEN_RETURN,   // RETURN
  TOKEN_IF,       // IF
  TOKEN_IF_THEN,  // THEN
  TOKEN_IF_END,   // FI
  TOKEN_GOTO,     // GOTO
  TOKEN_CALL,     // CALL
  TOKEN_WITH,     // WITH
  TOKEN_WITH_END, // END
  TOKEN_EXPR_BEGIN, // (
  TOKEN_EXPR_END, // )
  TOKEN_ASSIGN,   // =
  TOKEN_LESS,     // <
  TOKEN_GREATER,  // >
  TOKEN_LEQ,      // <=
  TOKEN_GEQ,      // >=
  TOKEN_EQ,       // ==
  TOKEN_ADD,      // +
  TOKEN_SUB,      // -
  TOKEN_MUL,      // *
  TOKEN_DIV,      // /
  TOKEN_LABEL,    // label
  TOKEN_DIGIT,    // [0-9]+(.[0-9]+)?
  TOKEN_IDENTIFIER,
} __attribute__ ((packed));

struct TokenPair {
  enum Tokens token;
  uint16_t id;
} __attribute__ ((packed));

struct identifier_entry {
  char* name;
  LIST_ENTRY(identifier_entry) entries;
};

struct token_entry {
  enum Tokens token;

  union {
    const struct identifier_entry *id;
    double fvalue;
  };

  LIST_ENTRY(token_entry) entries;
};

struct label_entry {
  const struct token_entry *position;
  const struct identifier_entry *id;
  LIST_ENTRY(label_entry) entries;
};

struct Variable {
  float value;
  const struct identifier_entry *id;
};

LIST_HEAD(tokenhead, token_entry);
LIST_HEAD(identifierhead, identifier_entry);
LIST_HEAD(labelhead, label_entry);


char* read_next_token(char* program, char* chars) {
  char* start, * end;
  for (start = program; CHECK_WHITESPACE(start); start++) {
    if (*start == '\0')
      return NULL;
  }

  for (end = start; !CHECK_WHITESPACE(end); end++);

  memcpy(chars, start, end - start);
  chars[end - start] = '\0';

  return end;
}

const struct identifier_entry* find_id_by_name(const struct identifierhead *head, const char* name) {
  struct identifier_entry *e;
  LIST_FOREACH(e, head, entries) {
    if (!strcmp(e->name, name))
      return e;
  }
  return NULL;
}

const struct label_entry* find_label_by_id(const struct labelhead *head, const struct identifier_entry *id) {
  struct label_entry *e;
  LIST_FOREACH(e, head, entries) {
    if (e->id == id)
      return e;
  }
  return NULL;
}

int lexer(char* program, struct tokenhead *tokens, struct identifierhead *ids) {
  LIST_INIT(tokens);
  LIST_INIT(ids);

  char name[100];
  char* idx = program;
  
  struct token_entry *l = NULL, *e = NULL;
  while ((idx = read_next_token(idx, name)) != NULL) {
    l = e;
    e = calloc(1, sizeof(*e));

    if (!strcmp(name, "DEF"))
      e->token = TOKEN_DEF;
    else if (!strcmp(name, "RETURN"))
      e->token = TOKEN_RETURN;
    else if (!strcmp(name, "IF"))
      e->token = TOKEN_IF;
    else if (!strcmp(name, "THEN"))
      e->token = TOKEN_IF_THEN;
    else if (!strcmp(name, "FI"))
      e->token = TOKEN_IF_END;
    else if (!strcmp(name, "GOTO"))
      e->token = TOKEN_GOTO;
    else if (!strcmp(name, "CALL"))
      e->token = TOKEN_CALL;
    else if (!strcmp(name, "WITH"))
      e->token = TOKEN_WITH;
    else if (!strcmp(name, "END"))
      e->token = TOKEN_WITH_END;
    else if (!strcmp(name, "("))
      e->token = TOKEN_EXPR_BEGIN;
    else if (!strcmp(name, ")"))
      e->token = TOKEN_EXPR_END;
    else if (!strcmp(name, "="))
      e->token = TOKEN_ASSIGN;
    else if (!strcmp(name, "<"))
      e->token = TOKEN_LESS;
    else if (!strcmp(name, ">"))
      e->token = TOKEN_GREATER;
    else if (!strcmp(name, "<="))
      e->token = TOKEN_LEQ;
    else if (!strcmp(name, ">="))
      e->token = TOKEN_GEQ;
    else if (!strcmp(name, "=="))
      e->token = TOKEN_EQ;
    else if (!strcmp(name, "+"))
      e->token = TOKEN_ADD;
    else if (!strcmp(name, "-"))
      e->token = TOKEN_SUB;
    else if (!strcmp(name, "*"))
      e->token = TOKEN_MUL;
    else if (!strcmp(name, "/"))
      e->token = TOKEN_DIV;
    else if (':' == name[0]) {
      e->token = TOKEN_LABEL;
    } else if ('0' <= name[0] && name[0] <= '9') {
      e->token = TOKEN_DIGIT;
      e->fvalue = atof(name);
    } else if ('A' <= name[0] && name[0] <= 'z') {
      e->token = TOKEN_IDENTIFIER;

      const struct identifier_entry *id = find_id_by_name(ids, name);
      if (!id) {
        struct identifier_entry *tid = malloc(sizeof(*tid));
        tid->name = malloc(strlen(name));
        memcpy(tid->name, name, strlen(name));
        LIST_INSERT_HEAD(ids, tid, entries);
        id = tid;
      }

      e->id = id;
    } else {
      printf("Unknown symbol: %s", name);
      return -1;
    }
    
    if (l == NULL)
      LIST_INSERT_HEAD(tokens, e, entries);
    else
      LIST_INSERT_AFTER(l, e, entries);
  }

  return 0;
}

int preprocess_tokens(const struct tokenhead *tokens, struct labelhead *labels, size_t *num_vars) {
  const struct token_entry *e;

  *num_vars = 0;
  LIST_INIT(labels);

  LIST_FOREACH(e, tokens, entries) {
    if (e->token == TOKEN_LABEL) {
      const struct identifier_entry *id = LIST_NEXT(e, entries)->id;
      if (find_label_by_id(labels, id) != NULL)
        reterr(-1, "redeclaration of label %s", id->name);

      struct label_entry *l = calloc(1, sizeof(*l));
      l->position = e;
      l->id = id;

      LIST_INSERT_HEAD(labels, l, entries);
    } else if (e->token == TOKEN_DEF) {
      *num_vars += 1;
    }
  }

  return 0;
}

void print_tokens(struct tokenhead *tokens) {
  printf("Printing tokens\n");
  struct token_entry *np;
  size_t i = 0;
  LIST_FOREACH(np, tokens, entries) {
    i++;
    printf("[%3zu] - %i", i, np->token);
    if (np->token == TOKEN_IDENTIFIER)
      printf(" -> %s (%p)", np->id->name, np->id);
    else if (np->token == TOKEN_DIGIT)
      printf(" -> %f", np->fvalue);
    printf("\n");
  }
}

struct Variable* get_variable(struct Variable *variables, size_t var_len, const struct identifier_entry *id) {
  for (size_t i = 0; i < var_len; i++) {
    if (variables[i].id == id)
      return &variables[i];
  }
  return NULL;
}

int parse_expr(const struct token_entry **entry, struct Variable *variables, size_t var_len, bool *is_bool, float *value) {
  if ((*entry)->token == TOKEN_EXPR_BEGIN)
    *entry = LIST_NEXT(*entry, entries);

  float tvalue = *is_bool = 0;
  float buffer;

  enum Tokens op = 0;

  for (size_t i = 0; i < 3; i++) {
    if (i == 1) {
      if (*entry == NULL || !IS_OPERATOR((*entry)->token))
        reterr(-1, "expected operator in expr");
      op = (*entry)->token;
    } else {
      // parse expression
      if ((*entry) == NULL || !IS_UNARY((*entry)->token))
        reterr(-1, "expected unary for <unary> in expr");
      
      if ((*entry)->token == TOKEN_IDENTIFIER)
        buffer = get_variable(variables, var_len, (*entry)->id)->value;
      else if ((*entry)->token == TOKEN_DIGIT)
        buffer = (*entry)->fvalue;
      else { // must be a (
        if (parse_expr(entry, variables, var_len, is_bool, &buffer))
          return -1;

        if (*entry == NULL || (*entry)->token != TOKEN_EXPR_END)
          reterr(-2, "missing )");
      }
    }

    if (i == 0) {
      tvalue = buffer;
    } else if (i == 2) {
      *is_bool = IS_BOOL_OP(op);

      if (op == TOKEN_ADD)
        *value = tvalue + buffer;
      else if (op == TOKEN_SUB)
        *value = tvalue - buffer;
      else if (op == TOKEN_MUL)
        *value = tvalue * buffer;
      else if (op == TOKEN_DIV)
        *value = tvalue / buffer;
      else if (op == TOKEN_LESS)
        *value = tvalue < buffer;
      else if (op == TOKEN_GREATER)
        *value = tvalue > buffer;
      else if (op == TOKEN_LEQ)
        *value = tvalue <= buffer;
      else if (op == TOKEN_GEQ)
        *value = tvalue >= buffer;
      else if (op == TOKEN_EQ)
        *value = fabs(tvalue - buffer) < 0.00001;
      else
        reterr(-2, "Unknown op");
    }

    *entry = LIST_NEXT(*entry, entries);
  }

  return 0;
}

int interpreter(const struct tokenhead *tokens, const struct labelhead *labels, size_t var_num) {
  const struct token_entry *e = LIST_FIRST(tokens), *next;

  struct Variable variables[var_num];  // statically allocate variables
  size_t initialised_vars = 0;

  int rpc;
  uint8_t nested_level;

  const struct identifier_entry *hold_id;

  clock_t start = clock(), diff;

  while (1) {  // parse variable declarations
    if (e->token != TOKEN_DEF)
      break;

    if ((next = LIST_NEXT(e, entries)) == NULL || next->token != TOKEN_IDENTIFIER)
      reterr(-2, "expected identifier after def");

    if (initialised_vars >= var_num)
      reterr(-3, "already initialized all expected vars");

    if (get_variable(variables, initialised_vars, next->id) != NULL)
      reterr(-3, "redeclaration of variable %s", next->id->name);

    variables[initialised_vars].id = next->id;
    variables[initialised_vars].value = 0;
    initialised_vars++;
    trace_asm("declared `%s` (var #%zu/%zu)", next->id->name, initialised_vars, var_num);

    // go two forward
    e = LIST_NEXT(LIST_NEXT(e, entries), entries);
  }

  while (1) {  // program loop
    rpc = 0;
    next = LIST_NEXT(e, entries);

    switch (e->token) {
    break;
      case TOKEN_IDENTIFIER:
        hold_id = e->id;
        rpc = 1;
        break;
      case TOKEN_ASSIGN:
        if (hold_id == NULL)
          reterr(-2, "unexpected =");
        if (next == NULL || !IS_UNARY(next->token))
          reterr(-2, "expected identifier or digit after assign");
        

        struct Variable *var = get_variable(variables, initialised_vars, hold_id);
        if (next->token == TOKEN_DIGIT) {
          var->value = next->fvalue;
          trace_asm("set %s = %f", var->id->name, var->value);
          rpc = 2;
        } else if (next->token == TOKEN_IDENTIFIER) {
          var->value = get_variable(variables, initialised_vars, next->id)->value;
          trace_asm("set %s = %s (%f)", var->id->name, next->id->name, var->value);
          rpc = 2;
        } else if (next->token == TOKEN_EXPR_BEGIN) {
          bool is_bool;
          if (parse_expr(&next, variables, initialised_vars, &is_bool, &var->value))
            return -1;

          if (next == NULL || next->token != TOKEN_EXPR_END)
            reterr(-2, "missing )");

          e = next;
          trace_asm("set %s = %f [expr]", var->id->name, var->value);
          rpc = 1;
        } else
          reterr(-1, "Unknown token");

        break;
      case TOKEN_LABEL:  // we already have a list of labels, so ignore
        if (nested_level > 0)
          reterr(-2, "unexpected label in nested function");
        rpc = 2;
        break;
      case TOKEN_GOTO:
        if (next == NULL || next->token != TOKEN_IDENTIFIER)
          reterr(-2, "expected identifier after goto");
        
        const struct label_entry *label = find_label_by_id(labels, next->id);
        if (label == NULL)
          reterr(-1, "unknown label");

        trace_asm("goto %p", label->position);
        e = label->position;
        nested_level = 0;
        continue;
      case TOKEN_IF:
        if (next == NULL || !IS_UNARY(next->token))
          reterr(-2, "expected unary after IF");

        bool is_bool;
        float value;

        if (parse_expr(&next, variables, initialised_vars, &is_bool, &value))
          return -1;

        if (next->token != TOKEN_IF_THEN)
          reterr(-2, "expected THEN after IF expr");
        if (!is_bool)
          reterr(-2, "expected boolean expression");
        e = next;

        if (value == 0) { // skip IF block
          // find FI
          for (uint8_t nest = 1; nest > 0;) {
            e = LIST_NEXT(e, entries);
            if (e->token == TOKEN_IF)
              nest++;
            else if (e->token == TOKEN_IF_END)
              nest--;
          }
        } else {
          nested_level++;
        }

        rpc = 1;
        trace_asm("if %d", (int)value);

        break;
      case TOKEN_IF_END:
        if (nested_level > 0) {
          nested_level--;
          trace_asm("fi [nested: %u]", nested_level);
          rpc = 1;
          break;
        }
      case TOKEN_RETURN:
      case TOKEN_CALL:
      case TOKEN_WITH:
      case TOKEN_WITH_END:
      default:
        reterr(-2, "unknown or unexpected token %d", e->token);
    }

    for (; rpc > 0; rpc--) {
      if ((e = LIST_NEXT(e, entries)) == NULL)
        goto end;
    }
  }

end:
  diff = clock() - start;
  printf("Interpreting took %lds %ldms %ldns", diff / 10000000, (diff / 1000) % 1000, diff % 1000);
  return 0;
}

int parse(char* program) {
  printf("Starting lexer\n");
  
  struct tokenhead tokens;
  struct identifierhead ids;
  struct labelhead labels;
  size_t var_num;

  // lex code
  if (lexer(program, &tokens, &ids))
    return -1;

  print_tokens(&tokens);

  // analyse tokens (e.g. find labels)
  preprocess_tokens(&tokens, &labels, &var_num);

  // interpreter
  interpreter(&tokens, &labels, var_num);

  return 0;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Requires file name to parse\n");
    return -1;
  }
  
  int fd = open(argv[1], O_RDONLY);
  size_t len = lseek(fd, 0, SEEK_END);
  char *data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);

  if (data[len - 1] == '\0') {
    printf("Data not null terminated\n");
    return -1;
  }

  return parse(data);
}
