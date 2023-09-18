#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"9cc.h"

char *user_input;

Token *token;

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

Token *new_token(Tokenkind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // whitespace
        if (isspace(*p)) {
            p++;
            continue;
        }

        // new line
        if (startswith(p, "\\n")) {
            p+=2;
            continue;
        }

        // identifier
        if (islower(*p)) {
            char *start = p;
            int len = 0;
            while (islower(*p)) {
                len++;
                p++;
            }
            char *buffer = (char *) malloc(len + 1);
            strncpy(buffer, start, len);
            if (!strcmp(buffer, "return")) {
                cur = new_token(TK_RETURN, cur, buffer, len);
            } else {
                cur = new_token(TK_IDENT, cur, buffer, len);
            }
            continue;
        }

        // two character operand
        if (startswith(p, "==") || 
            startswith(p, "!=") || 
            startswith(p, "<=") || 
            startswith(p, ">=")) {
                cur = new_token(TK_RESERVED, cur, p, 2);
                p+=2;
                continue;
        }

        // one character operand
        if (strchr("+-*/()<>;=", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        // number
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}