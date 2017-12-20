#!/usr/bin/env bash

LISP="./mlisp"
TEST_COUNT=0
PASS_COUNT=0

test() {
  echo "$1" | $LISP | grep -q "^$2$"
  if [ $? -eq 0 ]; then
    RESULT=" PASS"
    PASS_COUNT=$((PASS_COUNT + 1))
  else
    RESULT="!FAIL"
  fi
  echo "$RESULT: $1 -> $2"
  TEST_COUNT=$((TEST_COUNT + 1))
}

# quote
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

echo "Passed $PASS_COUNT of $TEST_COUNT"

