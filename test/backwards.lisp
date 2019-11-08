(load "primitives.lisp")

(defun reverse (lst)
    (if (null? lst)
        lst
        (append (reverse (cdr lst)) (list (car lst)))))

(defmacro backwards (*args)
    ;`(begin ,@(reverse args)))
    (cons 'begin (reverse args)))

(backwards (print 1) (print 2) (print 3))