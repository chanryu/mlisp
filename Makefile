.PHONY: clean

CXXFLAGS += -Wall -pedantic -O2 -g -std=c++11

all: mlisp

mlisp: mlisp.cpp mlisp.hpp main.cpp
	$(CXX) $(CXXFLAGS) -o $@ mlisp.cpp main.cpp

clean:
	@rm -fv mlisp
	@rm -rfv mlisp.dSYM
