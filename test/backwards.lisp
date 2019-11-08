(load "primitives.lisp")

(defun reverse (lst)
    (cond ((null? lst) lst)
          ('t (append (reverse (cdr lst)) (list (car lst))))))

(defmacro backwards (*args)
    `(begin ,@(reverse args)))

(backwards (print 1) (print 2) (print 3))