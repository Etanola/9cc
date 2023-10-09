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
    ND_BLOCK,  // ブロック
    ND_FUNC_CALL,   // 関数呼び出し
    ND_FUNC_DEF,    // 関数定義
} Nodekind;

typedef struct Node Node;

struct Node {
    Nodekind kind;
    Node *lhs;
    Node *rhs;

    Node *ret;  //return文

    Node *cond; //制御構文の条件式
    Node *then; //制御構文の実行式
    Node *els;  //if文のelse時の実行式
    Node *init; //for文の初期化式
    Node *inc;  //for文のループ式

    Node *stmt[100]; // 複文を入れる

    Node *args[6]; // ND_FUNCの引数
    int num_args;  // 引数の個数

    int val;
    char *str;
    int offset;
};


// identifier link list struct
typedef struct IdLink IdLink;

struct IdLink {
    IdLink *next;
    Nodekind kind;
    char *name;
    int len;
    int offset;
};


// global variable extern
extern char *user_input;

extern Token *token;

extern Node *code[100];

extern IdLink *idents;


// user_input
void error_at(char *loc, char *fmt, ...);

void error(char *fmt, ...);


// tokenize
Token *tokenize();

// parse tree
void parse();
void program();
Node *function();
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
void gen_expr(Node *node);
void gen_stmt(Node *node);
void codegen();