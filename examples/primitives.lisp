; defmacro
(define defmacro
    (macro (name args body)
        `(define ,name (macro ,args ,body))))

; defun
(define defun
    (macro (name args *body)
        `(define ,name (lambda ,args ,@body))))

; nil
(define nil '())

; list
(defun list (*args) args)

; caar
(defun caar (x)
  (car (car x)))

; cadr
(defun cadr (x)
  (car (cdr x)))

; cadar
(defun cadar (x)
  (car (cdr (car x))))

; caddr
(defun caddr (x)
  (car (cdr (cdr x))))

; cdar
(defun cdar (x)
  (cdr (car x)))

; null?
(defun null? (x) (eq x nil))

; and
(defun and (x y)
  (cond (x (cond (y 't)
                 ('t nil)))
  		  ('t nil)))

; not?
(defun not? (x)
  (cond (x nil)
		    ('t 't)))

; append
(defun append (x y)
  (cond ((null? x) y)
	    	('t (cons (car x) (append (cdr x) y)))))

; pair
(defun pair (x y)
  (cond ((and (null? x) (null? y)) nil)
		((and (not? (atom x)) (not? (atom y)))
		 (cons (cons (car x) (cons (car y) nil))
			   (pair (cdr x) (cdr y))))))

; assoc
(defun assoc (x y)
  (cond ((eq (caar y) x) (cadar y))
		    ('t (assoc x (cdr y)))))

; subst
(defun subst (x y z)
  (cond ((atom z) (cond ((eq z y) x)
                        ('t       z)))
		    ('t (cons (subst x y (car z)) (subst x y (cdr z))))))

; reverse
(defun reverse (lst)
    (cond ((null? lst) lst)
          ('t (append (reverse (cdr lst)) (list (car lst))))))

; begin
(defun begin (*args)
  (defun seq (ls)
    (cond ((atom (cdr ls)) (car ls))
          ('t (seq (cdr ls))))
  )
  (seq args))
