#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"9cc.h"

Node *code[100];

LVar *locals;

// create node
Node *new_node(Nodekind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_left(Nodekind kind, Node *lhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
        node->offset = lvar->offset;
    } else {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->offset = locals->offset + 8;
        node->offset = lvar->offset;
        locals = lvar;
    }
    return node;
}

// local variable
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

// token control
Token *consume() {
    Token *tok = token;
    token = token->next;
    return tok;
}

bool at_kind(Tokenkind kind) {
    if (token->kind != kind)
        return false;
    return true;
}

bool at_op(char *op) {
    if (strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    return true;
}

void expect_kind(Tokenkind kind) {
    if (token->kind != kind) {
        error_at(token->str, "トークンの種類が異なります。expected:%d,but now:%d\n", kind, token->kind);
    }
}

void expect_op(char *op) {
    if (strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "'%c'ではありません", op);
}


// parse tree
void parse() {
    LVar *start = calloc(1, sizeof(LVar));
    locals = start;
    program();
}

void program() {
    int i = 0;
    while (!at_kind(TK_EOF))
        code[i++] = stmt();
    code[i] = NULL;
}

Node *stmt() {
    Node *node;
    if (at_kind(TK_RETURN)) {
        consume();
        node = new_node_left(ND_RETURN, expr());
    } else {
        node = expr();
    }
    expect_kind(TK_RESERVED);
    expect_op(";");
    consume();
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    
    if (at_kind(TK_RESERVED) && at_op("=")) {
        consume();
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();

    for (;;) {
        if (at_kind(TK_RESERVED) && at_op("==")){
            consume();
            node = new_node(ND_EQ, node, relational());
        } else if(at_kind(TK_RESERVED) && at_op("!=")) {
            consume();
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();

    for (;;) {
        if (at_kind(TK_RESERVED) && at_op("<")) {
            consume();
            node = new_node(ND_LT, node, add());
        } else if (at_kind(TK_RESERVED) && at_op("<=")) {
            consume();
            node = new_node(ND_LE, node, add());
        } else if (at_kind(TK_RESERVED) && at_op(">")) {
            consume();
            node = new_node(ND_LT, add(), node);
        } else if (at_kind(TK_RESERVED) && at_op(">=")) {
            consume();
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (at_kind(TK_RESERVED) && at_op("+")){
            consume();
            node = new_node(ND_ADD, node, mul());
        } else if (at_kind(TK_RESERVED) && at_op("-")) {
            consume();
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (at_kind(TK_RESERVED) && at_op("*")) {
            consume();
            node = new_node(ND_MUL, node, unary());
        } else if (at_kind(TK_RESERVED) && at_op("/")) {
            consume();
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    if (at_kind(TK_RESERVED) && at_op("+")) {
        consume();
        return primary();
    } else if (at_kind(TK_RESERVED) && at_op("-")) {
        consume();
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

Node *primary() {
    if (at_kind(TK_RESERVED) && at_op("(")) {
        consume();
        Node *node = expr();
        expect_kind(TK_RESERVED);
        expect_op(")");
        consume();
        return node;
    }

    if (at_kind(TK_IDENT)) {
        return new_node_ident(consume());
    }

    if (at_kind(TK_NUM)) {
        return new_node_num(consume()->val);
    }
}