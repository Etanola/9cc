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

Node *new_node_ret(Node *ret) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->ret = ret;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// local variable
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

Node *new_node_ident(Token *tok, bool func) {
    Node *node = calloc(1, sizeof(Node));
    if (func) {
        node->kind = ND_FUNC;
    } else {
        node->kind = ND_LVAR;
    }
    node->str = tok->str;

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

Node *new_node_if(Node *cond, Node *then, Node *els) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->cond = cond;
    node->then = then;
    node->els = els;
    return node;
}

Node *new_node_while(Node *cond, Node *then) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->cond = cond;
    node->then = then;
    return node;
}

Node *new_node_for(Node *init, Node *cond, Node *inc, Node *then) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    node->init = init;
    node->cond = cond;
    node->inc = inc;
    node->then = then;
    return node;
}

Node *append_stmt(Node *node, Node *stmt, int i) {
    node->stmt[i] = stmt;
    return node;
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
    while (!at_kind(TK_EOF)) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *stmt() {
    Node *node; 

    if (at_kind(TK_RETURN)) {
        consume();
        node = new_node_ret(expr());
        expect_op(";");
        consume();
    } else if (at_kind(TK_IF)) {
        consume();
        expect_op("(");
        consume();
        Node *cond = expr();
        expect_op(")");
        consume();
        Node *then = stmt();
        Node *els = NULL;
        if (at_kind(TK_ELSE)) {
            consume();
            els = stmt();
        }
        node = new_node_if(cond, then, els);
    } else if (at_kind(TK_WHILE)) {
        consume();
        expect_op("(");
        consume();
        Node *cond = expr();
        expect_op(")");
        consume();
        Node *then = stmt();
        node = new_node_while(cond, then);
    } else if (at_kind(TK_FOR)) {
        consume();
        expect_op("(");
        consume();
        Node *init = NULL;
        Node *cond = NULL;
        Node *inc = NULL;
        if (!at_op(";")) {
            init = expr();
        }
        expect_op(";");
        consume();
        if (!at_op(";")) {
            cond = expr();
        }
        expect_op(";");
        consume();
        if (!at_op(")")) {
            inc = expr();
        }
        expect_op(")");
        consume();
        Node *then = stmt();
        node = new_node_for(init, cond, inc, then);
    } else if (at_op("{")) {
        consume();
        node = new_node(ND_BLOCK, NULL, NULL);
        for (int i = 0;!at_op("}");i++) {
            node = append_stmt(node,  stmt(), i);
        }
        consume();
    } else {
        if (at_op(";")) {
            consume();
            node = new_node(ND_BLOCK, NULL, NULL);
        } else {
            node = expr();
            expect_op(";");
            consume();
        }
    }
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
    int sign  = 1; // 1:+, 0:-
    while(at_op("+") || at_op("-")) {
        if (at_op("+")) {
            consume();
        } else if (at_op("-")) {
            consume();
            sign = 1 - sign;
        }
    }
    if (sign == 1) {
        return primary();
    } else if (sign == 0) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
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
        Token *tok = consume();
        if (at_op("(")) {
            consume();
            expect_op(")");
            consume();
            return new_node_ident(tok, true);
        }
        return new_node_ident(tok, false);
    }

    if (at_kind(TK_NUM)) {
        return new_node_num(consume()->val);
    }
}