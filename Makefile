BUILD := build
MLISP := mlisp

MLL_SRCS := $(wildcard src/mll/*.cpp)
MLL_OBJS := $(patsubst src/mll/%.cpp, $(BUILD)/mll/%.cpp.o, $(MLL_SRCS))
MLISP_SRCS := $(wildcard src/$(MLISP)/*.cpp)
MLISP_OBJS := $(patsubst src/$(MLISP)/%.cpp, $(BUILD)/$(MLISP)/%.cpp.o, $(MLISP_SRCS))

CXXFLAGS += -Wall -DNDEBUG -pedantic -O2 -g -std=c++11

.PHONY: clean test

all: $(MLISP)

$(MLISP): $(MLL_OBJS) $(MLISP_OBJS)
	$(CXX) -o $@ $(MLL_OBJS) $(MLISP_OBJS) -L/usr/local/lib -lreadline

$(BUILD)/$(MLISP)/%.cpp.o: src/$(MLISP)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Isrc/mll -I/usr/local/include -DMLISP_READLINE -MD -MF $@.d -c -o $@ $<
	@cp $@.d $@.P
	@sed -e 's/#.*//' \
	     -e 's/^[^:]*: *//' \
	     -e 's/ *\\$$//' \
	     -e '/^$$/ d' -e 's/$$/ :/' \
	     < $@.d >> $@.P
	@rm -f $@.d

$(BUILD)/mll/mll.cpp.o: src/mll/mll.cpp src/mll/mll.hpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

include $(shell find $(BUILD) -type f -name '*.P' 2> /dev/null)

test: $(MLISP)
	@./mlisp-test.sh

clean:
	rm -f  mlisp
	rm -rf $(BUILD)
