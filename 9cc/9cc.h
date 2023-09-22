#include<stdbool.h>

// token struct
typedef enum {
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_RETURN,   // returnトークン
    TK_IF,       // ifトークン
    TK_ELSE,     // elseトークン
    TK_WHILE,    // whileトークン
    TK_FOR,      // forトークン
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


// node struct
typedef enum {
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_ASSIGN, // =
    ND_NUM,    // integer
    ND_LVAR,   // local variable
    ND_RETURN, // return
    ND_IF,     // if
    ND_WHILE,  // while
    ND_FOR,    // for
} Nodekind;

typedef struct Node Node;

struct Node {
    Nodekind kind;
    Node *lhs;
    Node *rhs;

    Node *cond; //制御構文の条件式
    Node *then; //制御構文の実行式
    Node *els;  //if文のelse時の実行式
    Node *init; //for文の初期化式
    Node *inc;  //for文のループ式

    int val;
    int offset;
};


// local variable struct
typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};


// global variable extern
extern char *user_input;

extern Token *token;

extern Node *code[100];

extern LVar *locals;


// user_input
void error_at(char *loc, char *fmt, ...);

void error(char *fmt, ...);

Token *consume();

bool at_kind(Tokenkind kind);

bool at_op(char *op);

void expect_kind(Tokenkind kind);

void expect_op(char *op);


// tokenize
Token *new_token(Tokenkind kind, Token *cur, char *str, int len);

bool startswith(char *p, char *q);

Token *tokenize();


// create node
Node *new_node(Nodekind kind, Node *lhs, Node *rhs);

Node *new_node_num(int val);

Node *new_node_ident(Token *tok);

LVar *find_lvar(Token *tok);

// parse tree
void parse();
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

// generate assembly
void gen_lval(Node *node);
static int count(void);
void gen(Node *node);