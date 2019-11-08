(load "primitives.lisp")

(defmacro if (pred *arms)
    `(cond (,pred ,(car  arms))
           ('t    ,(cadr arms))))

(if 't (print "success - 1"))

(if '() (print "failure - 1"))

(if 't (print "success - 2")
       (print "failure - 2"))

(if '() (print "failure - 3")
        (print "success - 3"))