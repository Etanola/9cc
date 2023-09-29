#!/bin/bash
assert() {
    expected="$1"
    input="$2"


    ./9cc "$input" > tmp.s
    cc -c -o func.o func.c
    cc -o tmp tmp.s func.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 15 "foo = 3;\n return foo2(3, 4, 8);"
assert 120  "foo = 10;\n return foo3(1, 2, 3, 4, 5);"

echo ok