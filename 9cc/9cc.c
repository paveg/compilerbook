#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Kind of token
typedef enum {
  TK_RESERVED, // Token
  TK_NUM, // Integer token
  TK_EOF, // End of file token
} TokenKind;

typedef struct Token Token;

// Token types
struct Token {
  TokenKind kind; // Kind of token
  Token *next; // Next input token
  int val; // If kind is TK_NUM, that number
  char *str; // Token string
};

// Currently focused tokens
Token *token;

// Functions for reporting errors
// Takes the same arguments as printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// If the next token is the expected one, read one token and return true.
// Otherwise it returns false.
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
   token = token->next;
   return true;
}

// If the next token is the expected one, read one token.
// Otherwise it reports error.
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error("[ERROR] Not '%c'", op);
   token = token->next;
}

// If the next token is the expected one, read one token and return this number.
// Otherwise it reports error.
int expect_number() {
  if (token->kind != TK_NUM)
    error("[ERROR] Not a number");
  int val = token->val;
  token = token->next;
  return val;
}

// Returns true if end of input, false otherwise
bool at_eof() {
  return token->kind == TK_EOF;
}

// Create a new token and connect it to cur
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// Tokenize the input string p and return it
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip whitespace
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("Tokenize is not possible");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}


int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "[ERROR] incorrect number of arguments\n");
    return 1;
  }

  token = tokenize(argv[1]);

  //  Output the first part of the assembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // The first of the expression must be a number,
  // so check it and output the first mov instruction
  printf("  mov rax, %d\n", expect_number());

  // Output assembly while consuming token sequence `+ <number>` or `- <number>`
  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
