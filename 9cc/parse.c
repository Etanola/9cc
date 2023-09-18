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

// parse tree
void parse() {
    LVar *start = calloc(1, sizeof(LVar));
    locals = start;
    program();
}

void program() {
    int i = 0;
    while (!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
}

Node *stmt() {
    Node *node = expr();
    expect_op(";");
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    
    if (consume_op("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume_op("=="))
            node = new_node(ND_EQ, node, relational());
        else if(consume_op("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume_op("<"))
            node = new_node(ND_LT, node, add());
        else if(consume_op("<="))
            node = new_node(ND_LE, node, add());
        else if(consume_op(">"))
            node = new_node(ND_LT, add(), node);
        else if(consume_op(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume_op("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume_op("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;    
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume_op("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume_op("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if (consume_op("+"))
        return primary();
    else if (consume_op("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *primary() {
    if (consume_op("(")) {
        Node *node = expr();
        expect_op(")");
        return node;
    }

    if (at_ident()) {
        return new_node_ident(consume());
    }

    if (at_num()) {
        return new_node_num(consume()->val);
    }
}