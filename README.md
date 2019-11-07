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

- mll
  - make primitive data types extensible
  - quasiquote (`` ` ``), unquote (`,`) - [link](http://www.lispworks.com/documentation/HyperSpec/Body/02_df.htm)
  - non-list cons cell?
- mlisp
  - show error messages when interpreting files
  - `exit` procedure
- etc.
  - bootstraping script for `pre-commit` git hook for auto `clang-format`ing
  - cmake?

### Links

- [The Roots of Lisp](http://www.paulgraham.com/rootsoflisp.html)  Paul Graham
- [(How to Write a (Lisp) Interpreter (in Python))](http://norvig.com/lispy.html) Peter Norvig
- [Lisp in fewer than 200 lines of C](https://carld.github.io/2017/06/20/lisp-in-less-than-200-lines-of-c.html)
   - [Related threads](https://news.ycombinator.com/item?id=15781883) in Heacker News
