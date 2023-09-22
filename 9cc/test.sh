#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 1 "1;"
assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 100 "13+29+90-32;"
assert 41 "12 + 34 - 5;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 4 "(3+6)/2;"
assert 10 "-10+20;"
assert 10 "- (-10);"
assert 10 "- (- (+10));"

assert 0 "0==1;"
assert 1 "42==42;"
assert 1 "0!=1;"
assert 0 "42!=42;"

assert 1 "0<1;"
assert 0 "1<1;"
assert 0 "2<1;"
assert 1 "0<=1;"
assert 1 "1<=1;"
assert 0 "2<=1;"

assert 1 "1>0;"
assert 0 "1>1;"
assert 0 "1>2;"
assert 1 "1>=0;"
assert 1 "1>=1;"
assert 0 "1>=2;"

assert 3 "1; 2; 3;"
assert 14 "a = 3; b = 5 * 6 - 8; a + b / 2;"
assert 3 "a = 3;"
assert 5 "j = 5;"
assert 2 "a = b = 2;"
assert 1 "0<1;"
assert 12 "k = 10;\n k = m = 2 * 2 * 2;\n k + m / 2;"
assert 3 "foo = 3;"
assert 6 "foo = 1;\n bar = 2 + 3;\n foo + bar;"

assert 0 "return 0;"
assert 14 "a = 3;\n b = 5 * 6 - 8;\n return a + b / 2;"
assert 5 "return 5;\n return 8;\n"

assert 8 "a = 1;\n b = 3;\n if (a == b)\n return 5;\n else \n return 8;"
assert 5 "a = 3;\n b = 3;\n if (a == b)\n return 5;\n else \n return 3;"
assert 5 "a = 3;\n b = 3;\n if (a == b)\n a = a + 5;\n else \n b = b + 3;\n if (a != b)\n return 5;\n else return 10;"
assert 10 "i = 0;\n while (i < 10)\n i = i + 1; \n return i;"
assert 3 "x = 0;\n for (i = 0;i < 3;i = i + 1)\n x = x + 1;\n return x;"

echo ok