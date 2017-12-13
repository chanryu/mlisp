(set 'l (lambda  () (print 'lambda:  a)))
(set 'c (closure () (print 'closure: a)))

(set 'a 1)
(l)
(c)

(set 'a 2)
(l)
(c)

(do
  (set 'a 1)
  (l)
  (c))
