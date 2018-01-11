(define make_counter (lambda () (begin (define c 0) (lambda () (set c (+ c 1))))))
(define counter (make_counter))
