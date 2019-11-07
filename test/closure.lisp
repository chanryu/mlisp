(define begin
    (lambda (*args)
        (cond ((atom (cdr args)) (car args))
              ('t (begin (car (cdr args))))
        )
    )
)
(define make_counter (lambda () (begin (define c 0) (lambda () (set! c (+ c 1))))))
(define counter (make_counter))
