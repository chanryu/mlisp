BUILD := build

MLL_SRCS := $(wildcard src/mll/*.cpp)
MLL_OBJS := $(patsubst src/mll/%.cpp, $(BUILD)/mll/%.cpp.o, $(MLL_SRCS))
MLISP_SRCS := $(wildcard src/mlisp/*.cpp)
MLISP_OBJS := $(patsubst src/mlisp/%.cpp, $(BUILD)/mlisp/%.cpp.o, $(MLISP_SRCS))

CXXFLAGS += -Wall -DNDEBUG -pedantic -O2 -g -std=c++11

.PHONY: clean mlisp-test

all: mlisp-test

mlisp: $(MLL_OBJS) $(MLISP_OBJS)
	$(CXX) -o $@ $(MLL_OBJS) $(MLISP_OBJS) -L/usr/local/lib -I/usr/local/include -lreadline

$(BUILD)/%.cpp.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MD -MF $@.d -c -o $@ $<
	@cp $@.d $@.P
	@sed -e 's/#.*//' \
	     -e 's/^[^:]*: *//' \
	     -e 's/ *\\$$//' \
	     -e '/^$$/ d' -e 's/$$/ :/' \
	     < $@.d >> $@.P
	@rm -f $@.d

include $(shell find $(BUILD) -type f -name '*.P' 2> /dev/null)

mlisp-test: mlisp
	@./mlisp-test.sh

clean:
	rm -f  mlisp
	rm -rf $(BUILD)
