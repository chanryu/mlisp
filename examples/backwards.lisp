(load "primitives.lisp")

(define (reverse lst)
    (cond ((null? lst) lst)
          ('t (append (reverse (cdr lst)) (list (car lst))))))

(define-macro (backwards *args)
    `(begin ,@(reverse args)))

(backwards (print 1) (print 2) (print 3))