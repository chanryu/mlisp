BUILD := build

#SRCS := $(shell find . -type f -name '*.cpp')
SRCS := $(wildcard ./*.cpp)
OBJS := $(patsubst ./%.cpp, $(BUILD)/%.o, $(SRCS))

EXEC := mlisp

CXXFLAGS += -Wall -DNDEBUG -pedantic -O2 -g -std=c++11

.PHONY: clean

$(EXEC): $(OBJS)
	$(CXX) -o $@ $(OBJS)

$(BUILD)/%.o: ./%.cpp
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

clean:
	rm -f  $(EXEC)
	rm -rf $(BUILD)
