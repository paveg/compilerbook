#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tokenizer implementation
//

// Kind of token.
typedef enum {
  TK_RESERVED, // Token.
  TK_NUM, // Integer token.
  TK_EOF, // End of file token.
} TokenKind;

typedef struct Token Token;

// Token types.
struct Token {
  TokenKind kind; // Kind of token.
  Token *next; // Next input token.
  int val; // If kind is TK_NUM, that number.
  char *str; // Token string.
};

// Currently focused tokens.
Token *token;

// Input program.
char *user_input;

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // pos number of output a blank.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Functions for reporting errors.
// Takes the same arguments as printf.
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
    error_at(token->str, "[ERROR] expected '%c'", op);
   token = token->next;
}

// If the next token is the expected one, read one token and return this number.
// Otherwise it reports error.
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "[ERROR] expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

// Returns true if end of input, false otherwise.
bool at_eof() {
  return token->kind == TK_EOF;
}

// Create a new token and connect it to cur.
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// Tokenize the input string `user_input` and return it.
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip whitespace.
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

//
// Parser
//

// Kind of AST (Abstract syntax tree) node.
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // Integer
} NodeKind;

// AST (Abstract syntax tree) node type.
typedef struct Node Node;

// AST (Abstract syntax tree) node types.
struct Node {
  NodeKind kind; // Node type.
  Node *lhs; // left-hand side.
  Node *rhs; // right-hand side.
  int val; // Use only if kind is ND_NUM.
};

// Create new node.
Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// Create new node from number.
Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *expr();
Node *mul();
Node *primary();

Node *expr() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_binary(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = primary();

  for (;;) {
    if (consume('*'))
      node = new_binary(ND_MUL, node, primary());
    else if (consume('/'))
      node = new_binary(ND_DIV, node, primary());
    else
      return node;
  }
}

Node *primary() {
  // If the next token is "(", it should be "(" expr ")" .
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  // Otherwise it should be a number.
  return new_num(expect_number());
}

//
// Code generator
//

void generate(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  generate(node->lhs);
  generate(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s [ERROR] incorrect number of arguments", argv[0]);

  // Tokenize and parse.
  user_input = argv[1];
  token = tokenize();
  Node *node = expr();

  // Output the first part of the assembly.
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Traverse the AST to emit assembly.
  generate(node);

  // A result must be at the top of the stack, so pop it to RAX to make it a program exit code.
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
