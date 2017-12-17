(setq l (lambda () a))

(print "set a to 1")
(setq a 1)
(print "expected: 1, actual:" (l))

(print "set a to 2")
(setq a 2)
(print "expected: 2, actual:" (l))

(print "set a to 3")
(setq a 3)

(do
  (print "    new scope begin")
  (print "    set a to 1")
  (setq a 1)
  (print "    expected: 3, actual:" (l)))

(print "back to old scope")
(print "expected: 3, actual:" (l))
