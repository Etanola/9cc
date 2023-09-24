#!/bin/bash
assert() {
    input="$1"

    ./9cc "$input" > tmp.s
    cc -c -o func.o func.c
    cc -o tmp tmp.s func.o
    ./tmp
    actual="$?"

    echo "$actual!!!!!"
}

assert "return foo();"

echo ok