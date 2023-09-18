#include<stdbool.h>

//トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
} Tokenkind;

typedef struct Token Token;

struct Token {
    Tokenkind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

typedef enum {
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_NUM,    // integer
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_ASSIGN, // =
    ND_LVAR    // local variable
} Nodekind;

typedef struct Node Node;

struct Node {
    Nodekind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset;
};

// global variable
extern char *user_input;

extern Token *token;

extern Node *code[100];

// user_input
void error_at(char *loc, char *fmt, ...);

void error(char *fmt, ...);

bool consume(char *op);

void expect(char *op);

int expect_number();

char expect_ident();

bool at_ident();

bool at_num();

bool at_eof();


// token
Token *new_token(Tokenkind kind, Token *cur, char *str, int len);

bool startswith(char *p, char *q);

Token *tokenize();

Node *new_node(Nodekind kind, Node *lhs, Node *rhs);

Node *new_node_num(int val);

// node
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen(Node *node);