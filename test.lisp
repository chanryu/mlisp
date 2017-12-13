(set 'l (lambda () a))
(set 'c (closure () a))

(set 'a 1)
(print 'set-a-to-1)
(print 'lambda-expected: 1 'actual: (l))
(print 'closure-expected: 1 'actual: (c))

(set 'a 2)
(print 'set-a-to-2)
(print 'lambda-expected: 2 'actual: (l))
(print 'closure-expected: 2 'actual: (c))

(do
	(print 'new-scope-begin)
  (set 'a 1)
	(print 'set-a-to-1)
  (print 'lambda-expected: 1 'actual: (l))
  (print 'closure-expected: 2 'actual: (c)))
