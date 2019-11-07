# mlisp

![Build Status](https://travis-ci.com/chanryu/mlisp.svg?branch=master)

### Why?
Because every programmer should write (unless already have written) their own Lisp interpreter!

### Goals

- Teach myself how to Lisp
- Teach myself how to write Lisp interpreter
- Create a C++ library for embeddable Scheme-like scripting language

### Non-goals

- Fast Lisp interpreter
- Feature complete Lisp

### TODOs

- `mll`
  - make primitive data types extensible
  - non-list cons cell
- `mlisp`
  - [`defmacro`](http://www.gigamonkeys.com/book/macros-defining-your-own.html)
  - [backquote](http://www.lispworks.com/documentation/HyperSpec/Body/02_df.htm)
  - ...
- etc.
  - bootstraping script for `pre-commit` git hook for auto `clang-format`ing
  - cmake?

### Links

- [The Roots of Lisp](http://www.paulgraham.com/rootsoflisp.html)  Paul Graham
- [(How to Write a (Lisp) Interpreter (in Python))](http://norvig.com/lispy.html) Peter Norvig
- [Lisp: Common Lisp, Racket, Clojure, Emacs Lisp](http://hyperpolyglot.org/lisp)
- [Lisp in fewer than 200 lines of C](https://news.ycombinator.com/item?id=15781883)
   - [Related threads](https://carld.github.io/2017/06/20/lisp-in-less-than-200-lines-of-c.html) in Heacker News
