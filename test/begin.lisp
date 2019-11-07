(define begin
    (lambda (*args)
        (cond ((atom (cdr args)) (car args))
              ('t (begin (car (cdr args))))
        )
    )
)