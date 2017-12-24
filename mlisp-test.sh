#!/usr/bin/env bash

LISP="./mlisp"
FAIL_COUNT=0

red() {
  printf "\033[31m$1\033[0m"
}

green() {
  printf "\033[32m$1\033[0m"
}

test() {
  if [ -z "$3" ]; then
    EVAL=$(echo "$1" | $LISP)
  else
    EVAL=$(echo "$1" | $LISP $3.lisp)
  fi
  echo "$EVAL" | grep -q "^$2$"
  if [ $? -eq 0 ]; then
    printf "%s $1 -> $2\n" $(green "PASS")
  else
    FAIL_COUNT=$((FAIL_COUNT + 1))
    printf "%s $1 -> $2 (actual: $EVAL)\n" $(red "FAIL")
  fi
}

test-op() {
  test "$1" "$2" "primitives"
}

test-subst() {
  test "$1" "$2" "subst"
}

# quote
test "'a" "a"
test "'(a b c)" "(a b c)"

# atom
test "(atom 'a)" "t"
test "(atom '(a b c))" "nil"
test "(atom '())" "t"
test "(atom (atom 'a))" "t"
test "(atom '(atom 'a))" "nil"

# eq
test "(eq 'a 'a)" "t"
test "(eq 'a 'b)" "nil"
test "(eq '() '())" "t"

# car
test "(car '(a b c))" "a"

# cdr
test "(cdr '(a b c))" "(b c)"

# cons
test "(cons 'a '(b c))" "(a b c)"
test "(cons 'a (cons 'b (cons 'c ())))" "(a b c)"
test "(car (cons 'a '(b c)))" "a"
test "(cdr (cons 'a '(b c)))" "(b c)"

# cond
test "(cond ((eq 'a 'b) 'first) ((atom 'a) 'second))" "second"

# lambda
test "((lambda (x) (cons x '(b))) 'a)" "(a b)"
test "((lambda (x y) (cons x (cdr y))) 'z '(a b c))" "(z b c)"

# cadr
test-op "(cadr '((a b) (c d) e))" "(c d)"

# caddr
test-op "(caddr '((a b) (c d) e))" "e"

# cdar
test-op "(cdar '((a b) (c d) e))" "(b)"

# null.
test-op "(null. 'a)" "nil"
test-op "(null. '())" "t"

# and.
test-op "(and. (atom 'a) (eq 'a 'a))" "t"

# not.
test-op "(not. (eq 'a 'a))" "nil"
test-op "(not. (eq 'a 'b))" "t"

# append.
test-op "(append. '(a b) '(c d))" "(a b c d)"
test-op "(append. '() '(c d))" "(c d)"

# pair.
test-op "(pair. '(x y z) '(a b c))" "((x a) (y b) (z c))"

# assoc.
test-op "(assoc. 'x '((x a) (y b)))" "a"
test-op "(assoc. 'x '((x new) (x a) (y b)))" "new"

# subst
test-subst "(subst 'm 'b '(a b (a b c) d))" "(a m (a m c) d)" "subst"


if [ $FAIL_COUNT -eq 0 ]; then
  echo "All tests passed."
else
  echo $(red "$FAIL_COUNT test(s) failed.")
fi
