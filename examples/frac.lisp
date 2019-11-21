;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; frac.scm
;;;
;;; draw fractal curves
;;; by T.Shido
;;; on August 20, 2005
;;;
;;; modified by Chan Ryu for mlisp
;;; on November 19, 2019
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(load "primitives.lisp")

(define _x car)
(define _y cadr)
(define point list)
(define first car)
(define second cadr)
(define = number-equal?)

;;; (rappend '(1 2 3) '(4 5 6)) -> (3 2 1 4 5 6)
(defun rappend (ls0 ls1)
  (append (reverse ls0) ls1))
  
;;; 
(defun divide (p1 p2 r)
  (point (+ (* r (_x p1)) (* (- 1.0 r) (_x p2)))
	       (+ (* r (_y p1)) (* (- 1.0 r) (_y p2)))))

(defun print-point (p)
  (print (_x p) (_y p)))

(defun for-each (proc lst)
  (cond ((null? lst) '())
         ('t (begin (proc (car lst))
                    (for-each proc (cdr lst))))))

(defmacro reverse! (ls)
  `(begin
    (define r (reverse ,ls))
    (set! ,ls (list (car ,ls)))
    r
  )
)

(defmacro if (pred *arms)
    `(cond (,pred ,(car  arms))
           ('t    ,(cadr arms))))

; the main function to create fractal curves
(defun fractal (proc n points)
  (defun loop (i points)
    (if (= n i)
      (for-each print-point points)
      (loop
        (+ i 1)
        (begin
          (defun iter (points acc)
            (if (null? (cdr points))
              (reverse (cons (car points) acc))
              (iter (cdr points)
                    (rappend (proc (first points) (second points)) acc))
            )
          )
          (iter points '())
        )
      )
    )
  )
  (loop 0 points)
  'done
)

;;; c curve
(defun c-curve (p1 p2)
  (define p3 (divide p1 p2 0.5))
  (list
    p1
    (point (+ (_x p3) (- (_y p3) (_y p2)))
           (+ (_y p3) (- (_x p2) (_x p3))))
  )
)

;; ;;; dragon curve
;; (define dragon-curve 
;;   (let ((n 0))
;;     (lambda (p1 p2)
;;       (let ((op (if (even? n) + -))
;;             (p3 (divide  p1 p2 0.5)))
;;         (set! n (1+ n))
;;         (list
;;           p1
;;           (point (op (_x p3) (- (_y p3) (_y p2))) 
;;                  (op (_y p3) (- (_x p2) (_x p3))))
;;         )
;;       )
;;     )
;;   )
;; )

;; ;;; koch curve
;; (defun koch (p1 p2)
;;   (let ((p3 (divide p1 p2 2/3))
;;         (p4 (divide p1 p2 1/3))
;;         (p5 (divide p1 p2 0.5))
;;         (c  (/ (sqrt 3) 2)))
;;     (list
;;       p1
;;       p3
;;       (point (- (_x p5) (* c (- (_y p4) (_y p3))))
;;              (+ (_y p5) (* c (- (_x p4) (_x p3)))))
;;       p4
;;     )
;;   )
;; )


;; C-Curve
;(fractal c-curve 14 '((0 0) (2 3)))

;; Dragon-Curve
;(fractal dragon-curve 14 '((0 0) (1 0)))

;; Koch-Curve
;(fractal koch 5 '((0 0) (1 0)))