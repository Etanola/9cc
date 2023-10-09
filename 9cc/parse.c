#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"9cc.h"

Node *code[100];

IdLink *idents;

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

// 識別子リストから識別子を検索する
IdLink *find_ident(Token *tok, IdLink *idents) {
    for (IdLink *ident = idents; ident; ident = ident->next)
        if (ident->len == tok->len && !memcmp(tok->str, ident->name, ident->len))
            return ident;
    return NULL;
}

// 関数のノードを作成する
Node *new_node_ident(Token *tok, Nodekind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->str = tok->str;
    
    IdLink *ident = find_ident(tok, idents);
    if (ident) {
        if (node->kind == ND_LVAR && (ident->kind == ND_FUNC_CALL || ident->kind == ND_FUNC_DEF)) {
            error_at(token->str, "ローカル変数名が関数名と競合しています");
        }
        if ((node->kind == ND_FUNC_CALL || node->kind == ND_FUNC_DEF) && ident->kind == ND_LVAR) {
            error_at(token->str, "関数名がローカル変数名と競合しています");
        }
        node->offset = ident->offset;
    } else {
        ident = calloc(1, sizeof(IdLink));
        ident->next = idents;
        ident->kind = node->kind;
        ident->name = tok->str;
        ident->len = tok->len;
        ident->offset = idents->offset + 8;
        node->offset = ident->offset;
        idents = ident;
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
    idents = calloc(1, sizeof(IdLink));
    program();
}

// program = function*
void program() {
    int i = 0;
    while (!at_kind(TK_EOF)) {
        code[i++] = function();
    }
    code[i] = NULL;
}

// function = ident ("(" (ident ("," ident)*)? ")") "{" stmt* "}"
Node *function() {
    Node *node;
    expect_kind(TK_IDENT);
    node = new_node_ident(consume(), ND_FUNC_DEF);
    expect_op("(");
    consume();
    if (at_kind(TK_IDENT)) {
        node->args[0] = new_node_ident(consume(), ND_LVAR);
        int i = 1;
        while(!at_op(")")) {
            expect_op(",");
            consume();
            node->args[i] = new_node_ident(consume(), ND_LVAR);
            i++;
        }
        node->num_args = i;
    } else {
        node->num_args = 0;
    }
    expect_op(")");
    consume();

    expect_op("{");
    consume();
    for (int i = 0;!at_op("}");i++) {
        node = append_stmt(node,  stmt(), i);
    }
    consume();
    return node;
}

/*
stmt = "return" expr ";"
     | "if" "(" expr ")" stmt ("else" stmt) ?
     | "while" "(" expr ")" stmt
     | "for" "(" expr? ";" expr? ";" expr? ")" stmt
     | "{" stmt* "}"
     | expr? ";"
*/

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
            node = new_node(ND_BLOCK, NULL, NULL); // ";" = "{};"としてcodegenで処理する。
        } else {
            node = expr();
            expect_op(";");
            consume();
        }
    }
    return node;
}

// expr = assign
Node *expr() {
    return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
    Node *node = equality();
    
    if (at_kind(TK_RESERVED) && at_op("=")) {
        consume();
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
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

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
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

// add = mul ("+" mul | "-" mul)*
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

// mul = unary ("*" unary | "/" unary)*
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

// unary = ("+" | "-")* primary
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

/*
 primary = "(" expr ")"
         | ident ("(" (expr ("," expr)*)? ")")?
         | num
*/
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
            Node *node = new_node_ident(tok, ND_FUNC_CALL);
            if (!at_op(")")) {
                node->args[0] = expr();
                int i = 1;
                while(!at_op(")")) {
                    expect_op(",");
                    consume();
                    node->args[i] = expr();
                    i++;
                }
                node->num_args = i;
            } else {
                node->num_args = 0;
            }
            expect_op(")");
            consume();
            return node;
        }
        return new_node_ident(tok, ND_LVAR);
    }

    if (at_kind(TK_NUM)) {
        return new_node_num(consume()->val);
    }
}