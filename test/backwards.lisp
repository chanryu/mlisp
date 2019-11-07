(defun null? (x) (eq x '()))
(defun list (*args) args)
(defun append (x y)
    (if (null? x)
        y
		(cons (car x) (append (cdr x) y))))
(defun reverse (lst)
    (if (null? lst)
        lst
        (append (reverse (cdr lst)) (list (car lst)))))
(defun begin (*args)
    (cond ((atom (cdr args)) (car args))
          ('t (begin (car (cdr args))))))

(defmacro backwards (*args)
    (cons 'begin (reverse args)))

(backwards (print 1) (print 2) (print 3))