(define equal? (lambda (a b)
  (cond ((number? a) (cond ((number? b) (number-equal? a b))
						   ('t nil)))
		((string? a) (cond ((string? b) (string-equal? a b))
						   ('t nil)))
		((symbol? a) (cond ((symbol? b) (eq a b))
						   ('t nil)))
		('t nil)))
