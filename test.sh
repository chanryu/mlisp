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
  EVAL=$(echo "$1" | $LISP)
  echo "$EVAL" | grep -q "^$2$"
  if [ $? -eq 0 ]; then
    printf "%s $1 -> $2\n" $(green "PASS")
  else
    FAIL_COUNT=$((FAIL_COUNT + 1))
    printf "%s $1 -> $2 (actual: $EVAL)\n" $(red "FAIL")
  fi
}

# quote
test "(quote a)" "1"
test "(quote a)" "a"
test "(quote (a b c))" "(a b c)"
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

# defun
test "(defun subst (x y z)
        (cond ((atom z)
               (cond ((eq z y) x)
                     ('t z)))
              ('t (cons (subst x y (car z))
                        (subst x y (cdr z))))))
      (subst 'm 'b '(a b (a b c) d))" "(a m (a m c) d)"

if [ $FAIL_COUNT -eq 0 ]; then
  echo $(green "Passed all tests.")
else
  echo $(red "Failed $FAIL_COUNT test(s).")
fi
