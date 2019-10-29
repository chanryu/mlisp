SRC := src
BUILD := build
MLISP := $(BUILD)/mlisp/mlisp

SRCS := $(shell find $(SRC) -type f -name '*.cpp')
OBJS := $(patsubst $(SRC)/%, $(BUILD)/%.o, $(SRCS))

CXXFLAGS += -Wall -DNDEBUG -pedantic -O2 -g -std=c++17

.PHONY: clean test

all: $(MLISP)

$(MLISP): $(OBJS)
	$(CXX) -o $@ $(OBJS) -L/usr/local/lib -lreadline

$(BUILD)/%.o: $(SRC)/%
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Isrc -I/usr/local/include -DMLISP_READLINE -MD -MF $@.d -c -o $@ $<
	@cp $@.d $@.P
	@sed -e 's/#.*//' \
	     -e 's/^[^:]*: *//' \
	     -e 's/ *\\$$//' \
	     -e '/^$$/ d' -e 's/$$/ :/' \
	     < $@.d >> $@.P
	@rm -f $@.d

include $(shell find $(BUILD) -type f -name '*.P' 2> /dev/null)

test: $(MLISP)
	@cd test && ./mlisp-test.sh

clean:
	rm -rf $(BUILD)
