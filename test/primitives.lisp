; defun

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

; null.
(defun null. (x)
  (eq x '()))

; and.
(defun and. (x y)
  (cond (x (cond (y 't) ('t '())))
  		('t '())))

; not.
(defun not. (x)
  (cond (x '())
		('t 't)))

; append.
(defun append. (x y)
  (cond ((null. x) y)
		('t (cons (car x) (append. (cdr x) y)))))

; pair.
(defun pair. (x y)
  (cond ((and. (null. x) (null. y)) '())
		((and. (not. (atom x)) (not. (atom y)))
		 (cons (cons (car x) (cons (car y) '()))
			   (pair. (cdr x) (cdr y))))))

; assoc.
(defun assoc. (x y)
  (cond ((eq (caar y) x) (cadar y))
		('t (assoc. x (cdr y)))))

