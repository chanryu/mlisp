(set 'l (lambda () a))

(print 'set-a-to-1)
(set 'a 1)
(print 'lambda-expected: 1 'actual: (l))

(print 'set-a-to-2)
(set 'a 2)
(print 'lambda-expected: 2 'actual: (l))

(do
  (print 'new-scope-begin)
  (set 'a 1)
  (print 'set-a-to-1)
  (print 'lambda-expected: 2 'actual: (l)))
