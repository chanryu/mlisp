BUILD := build

SRCS := $(wildcard ./*.cpp)
OBJS := $(patsubst ./%.cpp, $(BUILD)/%.cpp.o, $(SRCS))

CXXFLAGS += -Wall -DNDEBUG -pedantic -O2 -g -std=c++11

.PHONY: clean mlisp-test

all: mlisp-test

mlisp: $(OBJS)
	$(CXX) -o $@ $(OBJS) -L/usr/local/lib -I/usr/local/include -lreadline

$(BUILD)/%.cpp.o: ./%.cpp
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
