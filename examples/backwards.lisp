(load "primitives.lisp")

(defmacro backwards (*args)
    `(begin ,@(reverse args)))

(backwards (print 1) (print 2) (print 3))